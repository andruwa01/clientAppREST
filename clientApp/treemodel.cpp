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

//    if (!doc.isArray())
//           return;

//       QJsonArray tasksArray = doc.array();

//       // Для связи id задачи с соответствующим TreeItem
//       QHash<int, TreeItem*> itemMap;

//       // Проходим по всем задачам
//       for (const QJsonValue &value : tasksArray)
//       {
//           QJsonObject taskObj = value.toObject();
//           int id = taskObj["id"].toInt();

//           // Подготовка данных для столбцов (пример: title, description, due_date, status)
//           QVariantList columnData;
//           columnData << taskObj["title"].toString()
//                      << taskObj["description"].toString()
//                      << taskObj["due_date"].toString()
//                      << taskObj["status"].toString();

//           // Создаем новый элемент. Можно использовать insertChildren(), если нужно,
//           // или создать элемент напрямую.
//           TreeItem *newItem = new TreeItem(columnData, p_rootTask.get());

//           // Определяем родительский элемент
//           if (taskObj["parent_task_id"].isNull()) {
//               // Элемент без родителя – добавляем к корневому элементу.
//               p_rootTask->insertChildren(p_rootTask->childCount(), 1, p_rootTask->columnCount());
//               // Обычно после вставки получаем указатель на созданный элемент:
//               TreeItem *insertedItem = p_rootTask->child(p_rootTask->childCount() - 1);
//               *insertedItem = *newItem; // Пример копирования данных или можно передать newItem напрямую.
//               itemMap.insert(id, insertedItem);
//           } else {
//               int parentId = taskObj["parent_task_id"].toInt();
//               TreeItem *parentItem = itemMap.value(parentId, nullptr);
//               if (parentItem) {
//                   parentItem->insertChildren(parentItem->childCount(), 1, p_rootTask->columnCount());
//                   TreeItem *insertedItem = parentItem->child(parentItem->childCount() - 1);
//                   *insertedItem = *newItem;
//                   itemMap.insert(id, insertedItem);
//               } else {
//                   // Если родитель еще не найден, можно либо откладывать добавление,
//                   // либо добавить элемент к корню как fallback.
//                   p_rootTask->insertChildren(p_rootTask->childCount(), 1, p_rootTask->columnCount());
//                   TreeItem *insertedItem = p_rootTask->child(p_rootTask->childCount() - 1);
//                   *insertedItem = *newItem;
//                   itemMap.insert(id, insertedItem);
//               }
//           }
//       }
}
