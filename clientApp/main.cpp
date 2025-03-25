#include "mainwindow.h"
#include "stringlistmodel.h"

#include <QApplication>
#include <QAbstractItemView>

#include <QListView>
#include <QTableView>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    MainWindow w;
//    w.show();

    QStringList numbers;
    numbers << "one" << "two" << "three";

    StringListModel *model = new StringListModel(nullptr, numbers);
    QAbstractItemView *view = new QTableView;

    view->setModel(model);
    view->show();

    return a.exec();
}
