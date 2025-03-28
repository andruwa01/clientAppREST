#include "treeitem.h"

TreeItem::TreeItem(QVariantList taskData, TreeItem *p_parentTask)
    : taskData(std::move(taskData)), p_parentTask(p_parentTask)
{}

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

int TreeItem::row() const
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

int TreeItem::columnCount() const
{
    return int(taskData.count());
}

QVariant TreeItem::data(int column) const
{
    return taskData.value(column);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > qsizetype(childTasks.size()))
    {
        return false;
    }

    for (int row = 0; row < count; ++row)
    {
        QVariantList data(columns);
        childTasks.insert(childTasks.cbegin() + position, std::make_unique<TreeItem>(data, this));
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
    if (column < 0 || column >= taskData.size())
    {
        return false;
    }

    taskData[column] = value;
    return true;
}
