#ifndef TREEITEMMODEL_H
#define TREEITEMMODEL_H

#include "treeitem.h"
#include "helpdefines.h"
#include "employeemodel.h"

#include <QAbstractItemModel>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

class TreeItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(TreeItemModel)

    explicit TreeItemModel(EmployeeModel *employeeModel, QObject *parent = nullptr);
    ~TreeItemModel() override;

public slots:
    void onTaskCompleted(const QModelIndex &index);
    void onTaskNotCompleted(const QModelIndex &index);

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
//    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
    std::unique_ptr<TreeItem> p_rootItem;
    TreeItem *getItem(const QModelIndex &index) const;
    EmployeeModel *m_employeeModel;
};

#endif // TREEITEMMODEL_H
