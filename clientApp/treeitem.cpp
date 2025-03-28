#include "treeitem.h"

TreeItem::TreeItem(const QJsonObject &taskData, TreeItem *p_parentTask)
    : p_parentTask(p_parentTask)
{
    setTaskDataFromJson(taskData);
}

void TreeItem::setTaskDataFromJson(const QJsonObject &taskData)
{
    m_id = taskData.contains("id") ? taskData["id"].toInt() : 0;
    m_parentTaskId = taskData.contains("parent_task_id") ? taskData["parent_task_id"].toInt() : -1;
    m_assigneeId = taskData.contains("assignee_id") ? taskData["assignee_id"].toInt() : -1;

    // data to display
    m_title = taskData.contains("title") ? taskData["title"].toString() : QString();
    m_description = taskData.contains("description") ? taskData["description"].toString() : QString();
    QString dueDateStr = taskData.contains("due_date") ? taskData["due_date"].toString() : QString();
    m_dueDate = QDate::fromString(dueDateStr, DATE_FORMAT);
    if (!m_dueDate.isValid()) {
        qWarning() << "invalid date format:" << dueDateStr;
        m_dueDate = QDate();
    }

    QString statusStr = taskData.contains("status") ? taskData["status"].toString() : QString();
    m_status = stringToStatus(statusStr);
}


TreeItem *TreeItem::child(int number)
{
    TreeItem *item = nullptr;
    if (number >= 0 && number < childCount())
    {
        item = childTasks.at(number).get();
    }
    return item;
}

int TreeItem::childCount() const
{
    return int(childTasks.size());
}

int TreeItem::columnCount() const
{
    return DISPLAY_COLUMNS;
}

int TreeItem::rowInParentChilds() const
{
    if (!p_parentTask)
    {
        return 0;
    }

    const auto it = std::find_if(p_parentTask->childTasks.cbegin(), p_parentTask->childTasks.cend(),
                                 [this](const std::unique_ptr<TreeItem> &treeItem)
    {
        return treeItem.get() == this;
    });

    if (it != p_parentTask->childTasks.cend())
    {
        return std::distance(p_parentTask->childTasks.cbegin(), it);
    }
    Q_ASSERT(false); // should not happen
    return -1;
}

QVariant TreeItem::data(int column) const
{
    switch(column)
    {
    case 0: return m_title;
    case 1: return m_description;
    case 2: return m_dueDate.toString(DATE_FORMAT);
    case 3: return statusToString(m_status);
    default:
        qWarning() << "unknown column:" << column;
        return QVariant();
    }
}

bool TreeItem::insertChildren(int position, int count)
{
    if (position < 0 || position > qsizetype(childTasks.size()))
    {
        return false;
    }

    for (int row = 0; row < count; row++)
    {
        childTasks.insert(childTasks.cbegin() + position, std::make_unique<TreeItem>(QJsonObject(), this));
    }

    return true;
}

TreeItem *TreeItem::parent()
{
    return p_parentTask;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > qsizetype(childTasks.size()))
    {
        return false;
    }

    for (int row = 0; row < count; row++)
    {
        childTasks.erase(childTasks.cbegin() + position);
    }

    return true;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    switch (column)
    {
    case 0:
        m_title = value.toString();
        break;
    case 1:
        m_description = value.toString();
        break;
    case 2:
    {
        QString dateStr = value.toString();
        QDate date = QDate::fromString(dateStr, DATE_FORMAT);
        if (date.isValid())
        {
            m_dueDate = date;
        } else
        {
            qWarning() << "Invalid date format:" << dateStr;
            return false;
        }
        break;
    }
    case 3:
    {
        QString statusStr = value.toString();
        m_status = stringToStatus(statusStr);
        if (m_status == TaskStatus::New && statusStr != "new")
        {
            qWarning() << "Invalid status format:" << statusStr;
            return false;
        }
        break;
    }
    default:
        return false;
    }

    return true;
}


TaskStatus TreeItem::stringToStatus(const QString &status) const
{
    if (status == "new")
    {
        return TaskStatus::New;
    }
    if (status == "in_progress")
    {
        return TaskStatus::InProgress;
    }
    if (status == "completed")
    {
        return TaskStatus::Completed;
    }

    qDebug() << "stringToStatus(): unkonown format of status: " << status;
    return TaskStatus::New;
}

QString TreeItem::statusToString(TaskStatus status) const
{
    switch (status)
    {
        case TaskStatus::New: return "New";
        case TaskStatus::InProgress: return "In Progress";
        case TaskStatus::Completed: return "Completed";
        default: return "unknown";
    }
}


int TreeItem::id() const
{
    return m_id;
}

void TreeItem::setId(int newId)
{
    m_id = newId;
}

int TreeItem::parentTaskId() const
{
    return m_parentTaskId;
}

void TreeItem::setParentTaskId(int newParentTaskId)
{
    m_parentTaskId = newParentTaskId;
}

const QString &TreeItem::title() const
{
    return m_title;
}

void TreeItem::setTitle(const QString &newTitle)
{
    m_title = newTitle;
}

const QString &TreeItem::description() const
{
    return m_description;
}

void TreeItem::setDescription(const QString &newDescription)
{
    m_description = newDescription;
}

const QDate &TreeItem::dueDate() const
{
    return m_dueDate;
}

void TreeItem::setDueDate(const QDate &newDueDate)
{
    m_dueDate = newDueDate;
}

TaskStatus TreeItem::status() const
{
    return m_status;
}

void TreeItem::setStatus(TaskStatus newStatus)
{
    m_status = newStatus;
}

int TreeItem::assigneeId() const
{
    return m_assigneeId;
}

void TreeItem::setAssigneeId(int newAssigneeId)
{
    m_assigneeId = newAssigneeId;
}
