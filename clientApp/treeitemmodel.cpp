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
    std::function<void(TreeItem*)> updateItems = [this, employeeId, &updateItems](TreeItem* item) {
        if (!item) return;
        
        if (item->data(TreeItem::Column_Employee).toInt() == employeeId) {
            item->setData(TreeItem::Column_Employee, 0); // 0 means Not Assigned
            QModelIndex idx = createIndex(item->rowInParentChilds(), TreeItem::Column_Employee, item);
            emit dataChanged(idx, idx, {Qt::DisplayRole});
        }
        
        for (int i = 0; i < item->childCount(); i++)
        {
            updateItems(item->child(i));
        }
    };
    
    updateItems(p_rootItem.get());
}

void TreeItemModel::onEmployeeNameChanged(int employeeId)
{
    std::function<void(TreeItem*)> updateItems = [this, employeeId, &updateItems](TreeItem* item) {
        if (!item) return;
        
        if (item->data(TreeItem::Column_Employee).toInt() == employeeId) {
            QModelIndex idx = createIndex(item->rowInParentChilds(), TreeItem::Column_Employee, item);
            emit dataChanged(idx, idx, {Qt::DisplayRole});
        }
        
        for (int i = 0; i < item->childCount(); i++)
        {
            updateItems(item->child(i));
        }
    };
    
    updateItems(p_rootItem.get());
}

#ifdef USE_API
void TreeItemModel::setApiClient(ApiClient* client)
{
    m_apiClient = client;
    
    if (m_apiClient)
    {
        connect(m_apiClient, &ApiClient::tasksReceived, this, &TreeItemModel::handleTasksReceived);
        connect(m_apiClient, &ApiClient::taskCreated, this, &TreeItemModel::handleTaskCreated);
        connect(m_apiClient, &ApiClient::taskUpdated, this, &TreeItemModel::handleTaskUpdated);
        connect(m_apiClient, &ApiClient::taskDeleted, this, &TreeItemModel::handleTaskDeleted);
    }
}

void TreeItemModel::syncWithServer()
{
    if (m_apiClient)
    {
        m_apiClient->getTasks();
    }
}

void TreeItemModel::handleTasksReceived(const QList<Task>& tasks)
{
    beginResetModel();
    
    // Clear existing data
    p_rootItem = std::make_unique<TreeItem>(QJsonObject());
    m_itemMap.clear();
    
    // First pass: create all items
    for (const Task& task : tasks)
    {
        TreeItem* parentItem = p_rootItem.get();
        if (task.parentTaskId != 0 && m_itemMap.contains(task.parentTaskId))
        {
            parentItem = m_itemMap[task.parentTaskId];
        }
        
        int newRow = parentItem->childCount();
        parentItem->insertChildren(newRow, 1);
        TreeItem* newItem = parentItem->child(newRow);
        
        // Set task data
        QJsonObject taskJson = m_apiClient->taskToJson(task);
        newItem->setTaskDataFromJson(taskJson);
        
        // Cache the item
        m_itemMap.insert(task.id, newItem);
    }
    
    endResetModel();
}

void TreeItemModel::handleTaskCreated(const Task& task)
{
    TreeItem* parentItem = p_rootItem.get();
    if (task.parentTaskId != 0 && m_itemMap.contains(task.parentTaskId))
    {
        parentItem = m_itemMap[task.parentTaskId];
    }
    
    beginInsertRows(createIndex(parentItem->rowInParentChilds(), 0, parentItem), 
                    parentItem->childCount(), parentItem->childCount());
    
    parentItem->insertChildren(parentItem->childCount(), 1);
    TreeItem* newItem = parentItem->child(parentItem->childCount() - 1);
    
    QJsonObject taskJson = m_apiClient->taskToJson(task);
    newItem->setTaskDataFromJson(taskJson);
    m_itemMap.insert(task.id, newItem);
    
    endInsertRows();
}

void TreeItemModel::handleTaskUpdated(const Task& task)
{
    if (TreeItem* item = m_itemMap.value(task.id))
    {
        QJsonObject taskJson = m_apiClient->taskToJson(task);
        item->setTaskDataFromJson(taskJson);
        
        QModelIndex topLeft = createIndex(item->rowInParentChilds(), 0, item);
        QModelIndex bottomRight = createIndex(item->rowInParentChilds(), columnCount() - 1, item);
        emit dataChanged(topLeft, bottomRight);
    }
}

void TreeItemModel::handleTaskDeleted(int id)
{
    if (TreeItem* item = m_itemMap.value(id))
    {
        TreeItem* parentItem = item->parent();
        if (!parentItem)
        {
            return;
        }
        
        int row = item->rowInParentChilds();
        beginRemoveRows(createIndex(parentItem->rowInParentChilds(), 0, parentItem), row, row);
        parentItem->removeChildren(row, 1);
        m_itemMap.remove(id);
        endRemoveRows();
    }
}

TreeItem* TreeItemModel::createTreeItem(const Task& task, TreeItem* parent)
{
    QJsonObject taskJson;
    taskJson["id"] = task.id;
    taskJson["parent_task_id"] = task.parentTaskId;
    taskJson["title"] = task.title;
    taskJson["description"] = task.description;
    taskJson["due_date"] = task.dueDate.toString(DATE_FORMAT);
    taskJson["status"] = task.status;
    taskJson["assignee_id"] = task.assigneeId;
    
    return new TreeItem(taskJson, parent);
}

Task TreeItemModel::toTask(TreeItem* item)
{
    Task task;
    task.id = item->id();
    task.parentTaskId = item->parentTaskId();
    task.title = item->title();
    task.description = item->description();
    task.dueDate = item->dueDate();
    task.status = item->statusToString(item->status());
    task.assigneeId = item->assigneeId();
    return task;
}
#endif

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
#ifdef USE_API
            if (m_apiClient)
            {
                Task task = toTask(item);
                m_apiClient->updateTask(task.id, task);
            }
#endif
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

#ifdef USE_API
    if (m_apiClient)
    {
        // Create new task via API
        Task newTask;
        newTask.parentTaskId = parentItem == p_rootItem.get() ? 0 : parentItem->id();
        newTask.title = tr("[No data]");
        newTask.status = "new";
        m_apiClient->createTask(newTask);
        return true; // Actual insertion will happen async in handleTaskCreated
    }
#endif

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

    TreeItem* itemToRemove = parentItem->child(row);
    if (!itemToRemove)
    {
        return false;
    }

#ifdef USE_API
    if (m_apiClient)
    {
        m_apiClient->deleteTask(itemToRemove->id());
        return true; // Actual removal will happen in handleTaskDeleted
    }
#endif

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
