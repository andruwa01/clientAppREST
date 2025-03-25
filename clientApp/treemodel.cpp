#include "treemodel.h"

TreeModel::TreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel{parent}
    , rootItem(std::make_unique<TreeItem>(QVariantList{tr("title"), tr("Summary")}))
{
    setupModelData(QStringView{data}.split(u'\n'), rootItem.get());
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};
    TreeItem *parentItem = parent.isValid()
            ? static_cast<TreeItem*>(parent.internalPointer())
            : rootItem.get();
    if (auto *childItem = parentItem->child(row))
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex TreeModel::parent(const QModelIndex &childIndex) const
{
    if (!childIndex.isValid())
        return {};
    auto *childItem = static_cast<TreeItem*>(childIndex.internalPointer());
    TreeItem *parentItem = childItem->parentItem();
    return parentItem != rootItem.get()
            ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}

TreeModel::~TreeModel() = default;


