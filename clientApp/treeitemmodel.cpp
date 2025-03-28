#include "treeitemmodel.h"

TreeItemModel::TreeItemModel(QObject *parent)
    : QAbstractItemModel{parent},
      rootItem(new TreeItem(QJsonObject()))
{

}

TreeItemModel::~TreeItemModel()
{

}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const
{

}

QModelIndex TreeItemModel::parent(const QModelIndex &child) const
{

}

int TreeItemModel::rowCount(const QModelIndex &parent) const
{

}

int TreeItemModel::columnCount(const QModelIndex &parent) const
{

}

QVariant TreeItemModel::data(const QModelIndex &index, int role) const
{

}

bool TreeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

}

QVariant TreeItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{

}

bool TreeItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{

}

bool TreeItemModel::insertRows(int row, int count, const QModelIndex &parent)
{

}

bool TreeItemModel::removeRows(int row, int count, const QModelIndex &parent)
{

}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const
{

}
