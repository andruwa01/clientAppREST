#ifndef OBJECTTREEMODEL_H
#define OBJECTTREEMODEL_H

#include <QObject>
#include <QAbstractItemModel>

class ObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ObjectTreeModel(QObject* parent = nullptr);
    void setColumns(QStringList cols);

protected:
    QObject* _rootItem;
    QStringList _columns;
    QObject* objByIndex(const QModelIndex &index) const;

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
};

#endif // OBJECTTREEMODEL_H
