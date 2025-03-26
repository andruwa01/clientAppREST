#include "treemodel.h"

TreeModel::TreeModel(const QStringList &headers, const QByteArray &jsonData, QObject *parent) : QAbstractItemModel(parent)
{
    QVariantList rootTaskData;
    for (const QString &header : headers)
    {
        rootTaskData << header;
    }
    p_rootTask = std::make_unique<TreeItem>(rootTaskData);
    setupModelDataFromJson(jsonData);
}

TreeModel::~TreeModel() {}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{

}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{

}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{

}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{

}

int TreeModel::rowCount(const QModelIndex &parent) const
{

}

int TreeModel::columnCount(const QModelIndex &parent) const
{

}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{

}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{

}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{

}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{

}

void TreeModel::setupModelDataFromJson(const QByteArray &jsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
}
