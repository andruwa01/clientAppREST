#ifndef TREEITEM_H
#define TREEITEM_H

#include "helpdefines.h"

#include <QVariant>
#include <QDate>
#include <QJsonObject>

enum class TaskStatus
{
    New,
    InProgress,
    Completed
};

class TreeItem
{
public:
    explicit TreeItem(const QJsonObject &taskData, TreeItem *p_parentTask = nullptr);

    void setTaskDataFromJson(const QJsonObject &taskData);

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

    TaskStatus stringToStatus(const QString &status) const;
    QString statusToString(TaskStatus status) const;
    void printTaskData();

private:
    std::vector<std::unique_ptr<TreeItem>> childTasks;

    int m_id = 0;
    int m_parentTaskId = 0;
    int m_assigneeId = 0;

    // displayed data		// column
    QString m_title = "";		// 0
    QString m_description = "";  // 1
    QDate m_dueDate = QDate::fromString("0001-01-01", DATE_FORMAT);		// 2
    TaskStatus m_status = stringToStatus("new");	// 3

    // ptr to parent task
    TreeItem *p_parentTask;
};

#endif // TREEITEM_H
