#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVariant>

class TreeItem
{
public:
    explicit TreeItem(QVariantList taskData, TreeItem *p_parentTask = nullptr);

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    int row() const;
    bool setData(int column, const QVariant &value);
private:
    std::vector<std::unique_ptr<TreeItem>> childTasks;
    QVariantList taskData;
    TreeItem *p_parentTask;
};

#endif // TREEITEM_H
