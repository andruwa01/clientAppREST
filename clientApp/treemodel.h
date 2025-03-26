#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "treeitem.h"

#include <QAbstractItemModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(TreeModel)

    explicit TreeModel(const QStringList &headers, const QByteArray &jsonData, QObject *parent = nullptr);
    ~TreeModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &parent = {}) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = {}) override;
private:
    void setupModelDataFromJson(const QByteArray &jsonData);
    TreeItem *getItem(const QModelIndex &index) const;

    std::unique_ptr<TreeItem> p_rootItem;
};

#endif // TREEMODEL_H
