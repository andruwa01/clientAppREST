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

    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "Failed to parse JSON or not an array";
        return;
    }

    // set data in tree
    QJsonArray taskArray = doc.array();
    QHash<int, TreeItem*> itemMap;
    for (const QJsonValue &value : taskArray)
    {
        QJsonObject taskJsonObj = value.toObject();
        int id = taskJsonObj["id"].toInt();

        TreeItem *p_parentItem = nullptr;
        if (taskJsonObj["parent_task_id"].isNull())
        {
            p_parentItem = p_rootItem.get();
        }
        else
        {
            int parentId = taskJsonObj["parent_task_id"].toInt();
            p_parentItem = itemMap.value(parentId, p_rootItem.get());
        }

        p_parentItem->insertChildren(p_parentItem->childCount(), 1);
        TreeItem *insertedItem = p_parentItem->child(p_parentItem->childCount() - 1);

        // insert json in insertedItem
        insertedItem->setTaskDataFromJson(taskJsonObj);

        itemMap.insert(id, insertedItem);
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
