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

    explicit TreeItem(const QJsonObject &taskData, TreeItem *p_parentTask = nullptr);

    // for item model

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

    // help functions

    void setTaskDataFromJson(const QJsonObject &taskData);
    QJsonObject taskDataToJson() const;
    void printTaskData();

private:
    // helper functions
    TaskStatus stringToStatus(const QString &status) const;
    QString statusToString(TaskStatus status) const;

    // ptrs to child tasks
    std::vector<std::unique_ptr<TreeItem>> childTasks;
    // ptr to parent task
    TreeItem *p_parentTask = nullptr;

private: // data
    int m_id = -1;
    int m_parentTaskId = -1;
    int m_assigneeId = -1;

    QString m_title = "";
    QString m_description = "";
    QDate m_dueDate = QDate();
    TaskStatus m_status = TaskStatus::New;
};

#endif // TREEITEM_H
