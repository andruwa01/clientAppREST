#include "treeitemmodel.h"

TreeItemModel::TreeItemModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    p_rootItem = std::make_unique<TreeItem>(QJsonObject());


    #ifdef TEST_JSON_INPUT

    QFile file(":/testTasks.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << Q_FUNC_INFO << "failed to open json file:" << file.fileName();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isArray())
    {
        qCritical() << Q_FUNC_INFO << "failed to parse JSON or it is not an array";
        return;
    }

    QJsonArray taskArray = doc.array();
    QHash<int, TreeItem*> itemMap;

    for (const QJsonValue &value : taskArray)
    {
        QJsonObject taskJsonObj = value.toObject();

        int id = taskJsonObj["id"].toInt(-1);
        int parentId = taskJsonObj["parent_task_id"].toInt(-1);

        if (id == -1)
        {
            qCritical() << Q_FUNC_INFO << "missing 'id' field in task";
        continue;
        }

        TreeItem *p_parentItem = p_rootItem.get();
        if (parentId != -1 && itemMap.contains(parentId))
        {
        p_parentItem = itemMap.value(parentId);
        }

        int newRow = p_parentItem->childCount();
        p_parentItem->insertChildren(newRow, 1);
        TreeItem *insertedItem = p_parentItem->child(newRow);

        PRINT_DEBUG_JSON(taskJsonObj);

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
        qCritical() << Q_FUNC_INFO << ": parent.isValid() or parent.column() == 0";
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
        returnIndex = QModelIndex();
    }

    return returnIndex;
}

int TreeItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 0)
    {
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
        qCritical() << Q_FUNC_INFO << "invalid index";
        return {};
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        TreeItem *item = getItem(index);
        return item->data(index.column());
    }

    return {};
}

bool TreeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
    {
        qCritical() << Q_FUNC_INFO << "invalid index";
        return false;
    }

    TreeItem *item = nullptr;
    bool result = false;

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        item = getItem(index);
        result = item->setData(index.column(), value);
        if (result)
        {
            emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        }
        else
        {
            qCritical() << Q_FUNC_INFO << "item->setData() returns false";
        }

        return result;
    }

    qCritical() << Q_FUNC_INFO << "no role handler for role" << role;

    return result;
}

QVariant TreeItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch(section)
        {
            case COLUMN_TITLE: 		 return "Title";
            case COLUMN_DESCRIPTION: return "Description";
            case COLUMN_DUE_DATE: 	 return "Due date";
            case COLUMN_STATUS: 	 return "Status";
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
        qCritical() << Q_FUNC_INFO << "parentItem == nullptr";
        return false;
    }

    beginInsertRows(parent, row, row + count - 1);
    const bool success = parentItem->insertChildren(row, count);
    endInsertRows();
    qDebug() << Q_FUNC_INFO << ": parentItem->insertChildren(row, count) success:" << success;

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
        qCritical() << Q_FUNC_INFO << "invalid index";
        return Qt::NoItemFlags;
    }

    if (index.column() == COLUMN_STATUS)
    {
        return QAbstractItemModel::flags(index);
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
