#include "treeitemmodel.h"

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

#ifdef USE_API
        if (m_apiClient)
        {
            Task task = toTask(item);
            m_apiClient->updateTask(task.id, task);
        }
#endif
    }
}

void TreeItemModel::onTaskNotCompleted(const QModelIndex &index)
{
    TreeItem* item = getItem(index);
    if (item)
    {
        item->setData(TreeItem::Column_Status, "new");
        emit dataChanged(index, index, {Qt::DisplayRole});

#ifdef USE_API
        if (m_apiClient)
        {
            Task task = toTask(item);
            m_apiClient->updateTask(task.id, task);
        }
#endif
    }
}

void TreeItemModel::onEmployeeRemoved(int employeeId)
{
    bool updated = false;
    std::function<void(TreeItem*)> updateItems = [this, employeeId, &updated, &updateItems](TreeItem* item) {
        if (!item) return;

        if (item->data(TreeItem::Column_Employee).toInt() == employeeId)
        {
            item->setData(TreeItem::Column_Employee, 0);
            QModelIndex idx = createIndex(item->rowInParentChilds(), TreeItem::Column_Employee, item);
            emit dataChanged(idx, idx, {Qt::DisplayRole});

#ifdef USE_API
            if (m_apiClient && item->id() > 0)
            {
                Task task = toTask(item);
                m_apiClient->updateTask(task.id, task);
            }
#endif
            updated = true;
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

    QModelIndex parentIndex;
    if (parentItem != p_rootItem.get())
    {
        parentIndex = createIndex(parentItem->rowInParentChilds(), 0, parentItem);
    }

    beginInsertRows(parentIndex, parentItem->childCount(), parentItem->childCount());

    parentItem->insertChildren(parentItem->childCount(), 1);
    TreeItem* newItem = parentItem->child(parentItem->childCount() - 1);

    QJsonObject taskJson = m_apiClient->taskToJson(task);
    newItem->setTaskDataFromJson(taskJson);

    if (task.id > 0)
    {
        m_itemMap.insert(task.id, newItem);
        qDebug() << "Added new task to itemMap, id:" << task.id;
    }
    else
    {
        qWarning() << "Invalid task id for new task:" << task.id;
    }

    endInsertRows();

    QModelIndex newIndex = createIndex(parentItem->childCount() - 1, 0, newItem);
    emit dataChanged(newIndex, newIndex, {Qt::DisplayRole, Qt::EditRole});
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
            qWarning() << "Cannot delete task with id" << id << ": no parent item";
            return;
        }

        int row = item->rowInParentChilds();
        if (row < 0)
        {
            qWarning() << "Cannot delete task with id" << id << ": invalid row";
            return;
        }

        QSet<int> idsToRemove;
        std::function<void(TreeItem*)> collectIds = [&idsToRemove, &collectIds](TreeItem* item)
        {
            if (!item) return;
            idsToRemove.insert(item->id());
            for (int i = 0; i < item->childCount(); ++i)
            {
                collectIds(item->child(i));
            }
        };

        collectIds(item);
        qDebug() << "Will remove task" << id << "with subtasks. Total tasks to remove:" << idsToRemove.size();

        QModelIndex parentIndex = parentItem == p_rootItem.get() ?
                                QModelIndex() :
                                createIndex(parentItem->rowInParentChilds(), 0, parentItem);

        beginRemoveRows(parentIndex, row, row);
        parentItem->removeChildren(row, 1);

        for (int idToRemove : idsToRemove)
        {
            m_itemMap.remove(idToRemove);
            qDebug() << "Removed from itemMap:" << idToRemove;
        }

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

    // Fix date conversion
    QDate itemDate = item->dueDate();
    qDebug() << "Converting date from TreeItem to Task:" << itemDate;

    if (itemDate.isValid()) {
        task.dueDate = itemDate;
    } else {
        qWarning() << "Invalid date in TreeItem, using current date";
        task.dueDate = QDate::currentDate();
    }

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
    if (!child.isValid() || !child.internalPointer())
    {
        return QModelIndex();
    }

    TreeItem *childItem = static_cast<TreeItem*>(child.internalPointer());
    if (!childItem)
    {
        return QModelIndex();
    }

    TreeItem *parentItem = childItem->parent();
    if (!parentItem || parentItem == p_rootItem.get())
    {
        return QModelIndex();
    }

    return createIndex(parentItem->rowInParentChilds(), 0, parentItem);
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
    Q_UNUSED(role)

    if (!index.isValid())
    {
        qWarning() << "Attempting to set data with invalid index";
        return false;
    }

    TreeItem *item = getItem(index);
    if (!item) {
        qWarning() << "Failed to get item for index";
        return false;
    }

    bool result = item->setData(index.column(), value);
    if (result)
    {
#ifdef USE_API
        if (m_apiClient)
        {
            if (item->id() <= 0) {
                qWarning() << "Attempting to update task with invalid id:" << item->id();
                return false;
            }
            Task task = toTask(item);
            m_apiClient->updateTask(task.id, task);
        }
#endif
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }
    else
    {
        qWarning() << "Failed to set data for item";
    }
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
        Task newTask;
        newTask.title = tr("New Task");
        newTask.description = tr("Description");
        newTask.parentTaskId = parentItem == p_rootItem.get() ? 0 : parentItem->id();
        newTask.status = "new";
        newTask.assigneeId = 0;
        newTask.dueDate = QDate::currentDate();

        m_apiClient->createTask(newTask);
        return true;
    }
#endif

    beginInsertRows(parent, row, row + count - 1);
    const bool success = parentItem->insertChildren(row, count);
    if (success)
    {
        TreeItem* newItem = parentItem->child(row);
        newItem->setData(TreeItem::Column_Title, tr("New Task"));
        newItem->setData(TreeItem::Column_Description, tr("Description"));
        newItem->setData(TreeItem::Column_Status, "new");
        newItem->setData(TreeItem::Column_DueDate, QDate::currentDate().toString(DATE_FORMAT));
    }
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

    TreeItem* itemToRemove = parentItem->child(row);
    if (!itemToRemove)
    {
        return false;
    }

#ifdef USE_API
    if (m_apiClient)
    {
        int taskId = itemToRemove->id();
        if (!m_itemMap.contains(taskId))
        {
            qWarning() << "Attempting to delete task that is not in itemMap:" << taskId;
            return false;
        }

        QSet<int> allTaskIds;
        std::function<void(TreeItem*)> collectIds = [&allTaskIds, &collectIds](TreeItem* item) {
            if (!item) return;
            allTaskIds.insert(item->id());
            for (int i = 0; i < item->childCount(); ++i) {
                collectIds(item->child(i));
            }
        };
        collectIds(itemToRemove);

        qDebug() << "About to delete task" << taskId << "with" << (allTaskIds.size() - 1) << "subtasks";

        m_apiClient->deleteTask(taskId);
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
    if (!index.isValid() || !index.internalPointer())
    {
        return Qt::NoItemFlags;
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (!item)
    {
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
