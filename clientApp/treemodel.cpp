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

TreeModel::~TreeModel() = default;


