#include "treeitemmodel.h"

// update new column

TreeItemModel::TreeItemModel(EmployeeModel *employeeModel, QObject *parent)
    : QAbstractItemModel{parent}, m_employeeModel(employeeModel)
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

void TreeItemModel::onTaskCompleted(const QModelIndex &index)
{
    TreeItem *item = getItem(index);
    if (item)
    {
        item->setData(TreeItem::Column_Status, "completed");
        emit dataChanged(index, index, {Qt::DisplayRole});
    }
}

void TreeItemModel::onTaskNotCompleted(const QModelIndex &index)
{
    TreeItem* item = getItem(index);
    if (item)
    {
        item->setData(TreeItem::Column_Status, "new");
        emit dataChanged(index, index, {Qt::DisplayRole});
    }
}

void TreeItemModel::onEmployeeRemoved(int employeeId)
{
    // Recursive function to update items
    std::function<void(TreeItem*)> updateItems = [this, employeeId, &updateItems](TreeItem* item) {
        if (!item) return;
        
        // Check if current item has the removed employee
        if (item->data(TreeItem::Column_Employee).toInt() == employeeId) {
            item->setData(TreeItem::Column_Employee, 0); // 0 means Not Assigned
        }
        
        // Process children
        for (int i = 0; i < item->childCount(); ++i) {
            updateItems(item->child(i));
        }
    };
    
    // Start from root
    updateItems(p_rootItem.get());
    
    // Notify views about the change
    emit dataChanged(QModelIndex(), QModelIndex(), {Qt::DisplayRole});
}

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
            case TreeItem::Column_Title:       return "Title";
            case TreeItem::Column_Description: return "Description";
            case TreeItem::Column_DueDate:     return "Due date"; 
            case TreeItem::Column_Status:      return "Status";
            case TreeItem::Column_Employee:    return "Employee";
            default: return QVariant();
        }
    }

    return QVariant();
}

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

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (index.column() == TreeItem::Column_Status)
    {
        return defaultFlags;
    }

    return defaultFlags | Qt::ItemIsEditable;
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
