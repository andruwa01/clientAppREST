#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "treeitem.h"

#include <QAbstractItemModel>

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeModel(const QString &data, QObject *parent = nullptr);
    ~TreeModel() override;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &childIndex) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
    static void setupModelData(const QList<QStringView> &lines, TreeItem* parent);

    std::unique_ptr<TreeItem> rootItem;
    };

#endif // TREEMODEL_H
