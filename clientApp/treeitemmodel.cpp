#include "treeitemmodel.h"

TreeItemModel::TreeItemModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    p_rootItem = std::make_unique<TreeItem>(QJsonObject());

    #ifdef TEST_JSON_INPUT

    // load json file (test)
    // read json from the file
    QFile file(":/testTasks.json");
    const bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    Q_ASSERT_X(res, Q_FUNC_INFO, "failed to open json file");
    if (!res) return;

    // read json to model
    QString jsonLines = QString::fromUtf8(file.readAll());

    qDebug() << "json contents:\n" << jsonLines;

    // form json to valid format
    if (jsonLines.startsWith("\"") && jsonLines.endsWith("\""))
    {
        jsonLines = jsonLines.mid(1, jsonLines.length() - 2);  // remove outer ""
    }
    jsonLines.replace("\\\"", "\"");  // replace \" with " in str
    jsonLines = jsonLines.trimmed();  // remove spaces and \n

    qDebug() << "json contents after:\n" << jsonLines;

    // try to parse
    QByteArray jsonData = jsonLines.toUtf8();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (doc.isNull() || !doc.isArray())
    {
        qCritical() << "TreeItemModel():" << "Failed to parse JSON or not an array";
        return;
    }

    // set data in tree
    QJsonArray taskArray = doc.array();
    QHash<int, TreeItem*> itemMap;
    for (const QJsonValue &value : taskArray)
    {
        QJsonObject taskJsonObj = value.toObject();
        int id = taskJsonObj["id"].toInt();

        TreeItem *p_parentItem = nullptr;
        if (taskJsonObj["parent_task_id"].isNull())
        {
            p_parentItem = p_rootItem.get();
        }
        else
        {
            int parentId = taskJsonObj["parent_task_id"].toInt();
            p_parentItem = itemMap.value(parentId, p_rootItem.get());
        }

        p_parentItem->insertChildren(p_parentItem->childCount(), 1);
        TreeItem *insertedItem = p_parentItem->child(p_parentItem->childCount() - 1);

        // insert json in insertedItem
        insertedItem->setTaskDataFromJson(taskJsonObj);

        itemMap.insert(id, insertedItem);
    }

    #endif

}

TreeItemModel::~TreeItemModel()
{}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        qCritical() << "index():" << "! parent.isValid() or parent.column() == 0";
        return {};
    }

    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
    {
        return {};
    }

    if (auto *childItem = parentItem->child(row))
    {
        return createIndex(row, column, childItem);
    }

    return {};
}

QModelIndex TreeItemModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
    {
        return {};
    }

    TreeItem *childItem = getItem(child);
    TreeItem *parentItem = childItem ? childItem->parent() : nullptr;

    QModelIndex returnIndex;
    if (parentItem != p_rootItem.get() && parentItem != nullptr)
    {
        returnIndex = createIndex(parentItem->rowInParentChilds(), 0, parentItem);
    }
    else
    {
        returnIndex = QModelIndex{};
    }

    return returnIndex;
}

int TreeItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 0)
    {
        qCritical() << "rowCount():" << "! parent.isValid() or parent.column() == 0";
        return 0;
    }

    const TreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

int TreeItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return  p_rootItem->columnCount();
}

QVariant TreeItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        qCritical() << "data():" << "! index.isValid()";
        return {};
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        qCritical() << "data():" << "role != Qt::DisplayRole and role != Qt::EditRole";
        return {};
    }

    TreeItem *item = getItem(index);
    return item->data(index.column());
}

bool TreeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
    {
        qCritical() << "setData():" << "role != Qt::EditRole";
        return false;
    }

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
    {
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }

    return result;

}

QVariant TreeItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch(section)
        {
            case 0: return "Title";
            case 1: return "Description";
            case 2: return "Due date";
            case 3: return "Status";
            default: return QVariant();
        }
    }

    return QVariant();
}

//bool TreeItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
//{
//    if (orientation == Qt::Horizontal && role == Qt::EditRole)
//    {
//        switch (section)
//        {
//            case 0
//                m_headerData[0] = value.toString();
//                break;
//            case 1:
//                m_headerData[1] = value.toString();
//                break;
//            case 2:
//                m_headerData[2] = value.toString();
//                break;
//            case 3:
//                m_headerData[3] = value.toString();
//                break;
//            default:
//                return false;
//        }

//        emit headerDataChanged(orientation, section, section);

//        return true;
//    }

//    return false;
//}


bool TreeItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
    {
        qCritical() << "insertRows()" << "!parentItem";
        return false;
    }

    beginInsertRows(parent, row, row + count - 1);
    const bool success = parentItem->insertChildren(row, count);
    endInsertRows();

    return success;
}

bool TreeItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
    {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    const bool success = parentItem->removeChildren(row, count);
    endRemoveRows();

    return success;
}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        qCritical() << "flags():" << "!index.isValid()";
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem *TreeItemModel::getItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (auto *item = static_cast<TreeItem*>(index.internalPointer()))
        {
            return item;
        }
    }

    return p_rootItem.get();
}
