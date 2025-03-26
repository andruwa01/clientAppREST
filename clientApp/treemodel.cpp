#include "treemodel.h"

TreeModel::TreeModel(const QStringList &headers, const QByteArray &jsonData, QObject *parent) : QAbstractItemModel(parent)
{
    QVariantList rootTaskData;
    for (const QString &header : headers)
    {
        rootTaskData << header;
    }
    p_rootItem = std::make_unique<TreeItem>(rootTaskData);
    setupModelDataFromJson(jsonData);
}

TreeModel::~TreeModel() {}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    TreeItem *item = getItem(index);

    return item->data(index.column());
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        ? p_rootItem->data(section) : QVariant{};
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return {};

    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return {};

    if (auto *childItem = parentItem->child(row))
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem ? childItem->parent() : nullptr;

    return (parentItem != p_rootItem.get() && parentItem != nullptr)
        ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};

}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 0)
        return 0;

    const TreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return p_rootItem->columnCount();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = p_rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position,
                                                    rows,
                                                    p_rootItem->columnCount());
    endInsertRows();

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (auto *item = static_cast<TreeItem*>(index.internalPointer()))
            return item;
    }
    return p_rootItem.get();

}

void TreeModel::setupModelDataFromJson(const QByteArray &jsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (!doc.isArray())
           return;

       QJsonArray tasksArray = doc.array();

       // Для связи id задачи с соответствующим TreeItem
       QHash<int, TreeItem*> itemMap;

       // Проходим по всем задачам
       for (const QJsonValue &value : tasksArray)
       {
           QJsonObject taskObj = value.toObject();
           int id = taskObj["id"].toInt();

           // Подготовка данных для столбцов (пример: title, description, due_date, status)
           QVariantList columnData;
           columnData << taskObj["title"].toString()
                      << taskObj["description"].toString()
                      << taskObj["due_date"].toString()
                      << taskObj["status"].toString();

           // Создаем новый элемент. Можно использовать insertChildren(), если нужно,
           // или создать элемент напрямую.
           TreeItem *newItem = new TreeItem(columnData, p_rootItem.get());

           // Определяем родительский элемент
           if (taskObj["parent_task_id"].isNull()) {
               // Элемент без родителя – добавляем к корневому элементу.
               p_rootItem->insertChildren(p_rootItem->childCount(), 1, p_rootItem->columnCount());
               // Обычно после вставки получаем указатель на созданный элемент:
               TreeItem *insertedItem = p_rootItem->child(p_rootItem->childCount() - 1);
//               *insertedItem = *newItem; // Пример копирования данных или можно передать newItem напрямую.
               insertedItem = std::move(newItem);
               itemMap.insert(id, insertedItem);
           } else {
               int parentId = taskObj["parent_task_id"].toInt();
               TreeItem *parentItem = itemMap.value(parentId, nullptr);
               if (parentItem) {
                   parentItem->insertChildren(parentItem->childCount(), 1, p_rootItem->columnCount());
                   TreeItem *insertedItem = parentItem->child(parentItem->childCount() - 1);
//                   *insertedItem = *newItem;
                   insertedItem = std::move(newItem);
                   itemMap.insert(id, insertedItem);
               } else {
                   // Если родитель еще не найден, можно либо откладывать добавление,
                   // либо добавить элемент к корню как fallback.
                   p_rootItem->insertChildren(p_rootItem->childCount(), 1, p_rootItem->columnCount());
                   TreeItem *insertedItem = p_rootItem->child(p_rootItem->childCount() - 1);
//                   *insertedItem = *newItem;
                   insertedItem = std::move(newItem);
                   itemMap.insert(id, insertedItem);
               }
           }
       }
}


