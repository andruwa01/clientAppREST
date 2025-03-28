#include "treeitemmodel.h"

TreeItemModel::TreeItemModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    p_rootItem = std::make_unique<TreeItem>(QJsonObject());

    #ifdef TEST_JSON_INPUT

    // load json file (test)
    // read json from the file
    QFile file(":/testTasks.json");
    const bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    Q_ASSERT_X(res, Q_FUNC_INFO, "failed to open json file");
    if (!res) return;

    // read json to model
    QString jsonLines = QString::fromUtf8(file.readAll());

    qDebug() << "json contents:\n" << jsonLines;

    // form json to valid format
    if (jsonLines.startsWith("\"") && jsonLines.endsWith("\""))
    {
        jsonLines = jsonLines.mid(1, jsonLines.length() - 2);  // remove outer ""
    }
    jsonLines.replace("\\\"", "\"");  // replace \" with " in str
    jsonLines = jsonLines.trimmed();  // remove spaces and \n

    qDebug() << "json contents after:\n" << jsonLines;

    // try to parse
    QByteArray jsonData = jsonLines.toUtf8();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    doc.isNull() ? qDebug() << "fail to parse json" : qDebug() << "success parse json, size: " << doc.array().size();

    // set data in tree
    QJsonArray taskArray = doc.array();
    for (const QJsonValue &value : taskArray)
    {
        // parse
    }

    #endif

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
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch(section)
        {
            case 0: return "Title";
            case 1: return "Description";
            case 2: return "Due date";
            case 3: return "Status";
            default: return QVariant();
        }
    }

    return QVariant();
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
