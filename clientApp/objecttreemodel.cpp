#include "objecttreemodel.h"

ObjectTreeModel::ObjectTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    _rootItem = new QObject(this);
}

void ObjectTreeModel::setColumns(QStringList cols)
{
    _columns = cols;
}

QObject *ObjectTreeModel::objByIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return _rootItem;
    }
    return static_cast<QObject*>(index.internalPointer());
}

QModelIndex ObjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

QModelIndex ObjectTreeModel::parent(const QModelIndex &child) const
{

}

int ObjectTreeModel::rowCount(const QModelIndex &parent) const
{
    return objByIndex(parent)->children().count();
}

int ObjectTreeModel::columnCount(const QModelIndex &parent) const
{
    return _columns.count();
}

QVariant ObjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole)
    {
        return objByIndex(index)->property(_columns.at(index.column()).toUtf8());
    }
}
