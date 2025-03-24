#include "mainwindow.h"

#include <QApplication>

#include <QTreeView>
#include <QStandardItemModel>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    QStandardItemModel model;
    model.setHorizontalHeaderLabels({"Name", "Desctiption"});

    QStandardItem* rootItem = model.invisibleRootItem();

    // Добавление первого сотрудника
    QStandardItem* employee1 = new QStandardItem("Сотрудник 1");
    QStandardItem* task1 = new QStandardItem("Задача 1");
    QStandardItem* task2 = new QStandardItem("Задача 2");
    employee1->appendRow({task1, new QStandardItem("Описание 1")});
    employee1->appendRow({task2, new QStandardItem("Описание 2")});
    employee1->appendRow({task2, new QStandardItem("Описание 2")});

    rootItem->appendRow(employee1);

    // Добавление второго сотрудника
    QStandardItem* employee2 = new QStandardItem("Сотрудник 2");
    rootItem->appendRow(employee2);

    // Создание и отображение QTreeView
    QTreeView treeView;
    treeView.setModel(&model);
    treeView.setWindowTitle("Пример QStandardItemModel");
    treeView.show();

    return a.exec();
}
