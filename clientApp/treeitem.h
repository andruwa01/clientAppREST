#ifndef TREEITEM_H
#define TREEITEM_H

#include "helpdefines.h"

#include <QVariant>
#include <QDate>
#include <QJsonObject>

class TreeItem
{
public:
    enum class TaskStatus
    {
        New,
        InProgress,
        Completed
    };

    enum TreeItemColumns
    {
        Column_Title        = 0,
        Column_Description  = 1, 
        Column_DueDate      = 2,
        Column_Status       = 3,
        Column_Employee     = 4
    };

public:
    explicit TreeItem(const QJsonObject &taskData, TreeItem *p_parentTask = nullptr);

    // model interface methods
    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    int rowInParentChilds() const;
    bool setData(int column, const QVariant &value);

    // getters and setters
    int id() const;
    void setId(int newId);

    int parentTaskId() const;
    void setParentTaskId(int newParentTaskId);

    const QString &title() const;
    void setTitle(const QString &newTitle);

    const QString &description() const;
    void setDescription(const QString &newDescription);

    const QDate &dueDate() const;
    void setDueDate(const QDate &newDueDate);

    TaskStatus status() const;
    void setStatus(TaskStatus newStatus);

    int assigneeId() const;
    void setAssigneeId(int newAssigneeId);

    // helper methods
    void setTaskDataFromJson(const QJsonObject &taskData);
    QJsonObject taskDataToJson() const;
    void printTaskData();

private:
    // helper functions
    TaskStatus stringToStatus(const QString &status) const;
    QString statusToString(TaskStatus status) const;

    // tree structure
    std::vector<std::unique_ptr<TreeItem>> childTasks;
    TreeItem *p_parentTask = nullptr;

private: // task data
    int m_id = 0;
    int m_parentTaskId = 0;
    int m_assigneeId = 0;

    QString m_title = "";
    QString m_description = "";
    QDate m_dueDate = QDate();
    TaskStatus m_status = TaskStatus::New;
};

#endif // TREEITEM_H
