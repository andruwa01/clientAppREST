#include "treeitem.h"

TreeItem::TreeItem(const QJsonObject &taskData, TreeItem *p_parentTask)
    : p_parentTask(p_parentTask)
{
    setTaskDataFromJson(taskData);
}

void TreeItem::setTaskDataFromJson(const QJsonObject &taskData)
{
    if (taskData.empty())
    {
        return;
    }

    m_id = taskData.value("id").toInt();
    m_parentTaskId = taskData.value("parent_task_id").toInt(0);
    m_assigneeId = taskData.value("assignee_id").toInt(0);

    m_title = taskData.value("title").toString();
    m_description = taskData.value("description").toString();
    QString dueDateStr = taskData.value("due_date").toString();

    // Parse date using UI format
    m_dueDate = QDate::fromString(dueDateStr, DATE_FORMAT);
    if (!m_dueDate.isValid())
    {
        qWarning() << "Invalid date format:" << dueDateStr;
        m_dueDate = QDate::currentDate();  // Use current date if parsing failed
    }

    QString statusStr = taskData.value("status").toString();
    m_status = stringToStatus(statusStr);

    printTaskData();
}

QJsonObject TreeItem::taskDataToJson() const
{
    QJsonObject jsonObj;
    jsonObj["id"] = m_id;
    jsonObj["parent_task_id"] = (m_parentTaskId != 0) ? m_parentTaskId : QJsonValue::Null;
    jsonObj["assignee_id"] = (m_assigneeId != 0) ? m_assigneeId : QJsonValue::Null;
    jsonObj["title"] = m_title;
    jsonObj["description"] = m_description;
    jsonObj["due_date"] = m_dueDate.toString(DATE_FORMAT); // UI format for date
    jsonObj["status"] = statusToString(m_status);
    return jsonObj;
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
    return Column_Employee + 1; // last enum + 1
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
        case Column_Title:        return m_title;
        case Column_Description:  return m_description;
        case Column_DueDate:      
            if (!m_dueDate.isValid())
            {
                qWarning() << "Invalid date in TreeItem, using current";
                return QDate::currentDate().toString(DATE_FORMAT);
            }
            return m_dueDate.toString(DATE_FORMAT);
        case Column_Status:       return statusToString(m_status);
        case Column_Employee:     return m_assigneeId;
        default: qWarning() << Q_FUNC_INFO << ": unknown column:" << column;
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
    case Column_Title:
        m_title = value.toString();
        break;
    case Column_Description:
        m_description = value.toString();
        break;
    case Column_DueDate:
    {
        QString dateStr = value.toString();
        qDebug() << "Setting date in TreeItem, input:" << dateStr;
        
        QDate date = QDate::fromString(dateStr, DATE_FORMAT);
        if (!date.isValid())
        {
            date = QDate::fromString(dateStr, Qt::ISODate);
        }
        
        if (date.isValid())
        {
            m_dueDate = date;
            qDebug() << "Date set successfully:" << m_dueDate.toString(Qt::ISODate);
            return true;
        }
        
        qWarning() << "Failed to parse date:" << dateStr;
        return false;
    }
    case Column_Status:
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
    case Column_Employee:
        m_assigneeId = value.toInt();
        break;
    default:
        return false;
    }

    return true;
}


TreeItem::TaskStatus TreeItem::stringToStatus(const QString &status) const
{
    if (status == "new")          return TaskStatus::New;
    if (status == "in_progress")  return TaskStatus::InProgress;
    if (status == "completed")    return TaskStatus::Completed;

    qDebug() << Q_FUNC_INFO << ": unknown format of status: " << status;

    return TaskStatus::New;
}

QString TreeItem::statusToString(TaskStatus status) const
{
    switch (status)
    {
        case TaskStatus::New:        return "new";
        case TaskStatus::InProgress: return "in_progress";
        case TaskStatus::Completed:  return "completed";
        default:                     return "unknown";
    }
}

void TreeItem::printTaskData()
{
    qDebug() << "\nTreeItem private members:";
    qDebug() << "m_id:" << m_id;
    qDebug() << "m_parentTaskId:" << m_parentTaskId;
    qDebug() << "m_assigneeId:" << m_assigneeId;
    qDebug() << "m_title:" << m_title;
    qDebug() << "m_description:" << m_description;
    qDebug() << "m_dueDate:" << m_dueDate;
    qDebug() << "m_status:" << statusToString(m_status);
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
    if (!m_dueDate.isValid()) 
    {
        qWarning() << "Invalid date requested from TreeItem, returning current date";
        return QDate::currentDate();
    }
    return m_dueDate;
}

void TreeItem::setDueDate(const QDate &newDueDate)
{
    m_dueDate = newDueDate;
}

TreeItem::TaskStatus TreeItem::status() const
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
