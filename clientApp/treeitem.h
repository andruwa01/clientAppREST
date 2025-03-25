#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVariantList>

class TreeItem
{
public:
    explicit TreeItem(QVariantList data, TreeItem *parentItem = nullptr);

    void appendChild(std::unique_ptr<TreeItem> &&child); // add data to model (when model first created)
    // allow model to obtain info about childs
    TreeItem *child(int row);
    int childCount() const;

    int columnCount() const; // columns int the item
    QVariant data(int column) const; // data by column

    // obtain row number and parent item
    int row() const; // get current item's location in parent's list of items
    TreeItem *parentItem();

private:
    std::vector<std::unique_ptr<TreeItem>> m_childItems;
    QVariantList m_itemData;
    TreeItem *m_parentItem;
};

#endif // TREEITEM_H
