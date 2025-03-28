#ifndef TREEITEMMODEL_H
#define TREEITEMMODEL_H

#include <QAbstractItemModel>

class TreeItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeItemModel(QObject *parent = nullptr);
};

#endif // TREEITEMMODEL_H
