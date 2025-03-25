#include "mainwindow.h"
#include "stringlistmodel.h"

#include <QApplication>
#include <QAbstractItemView>

#include <QListView>
#include <QTableView>
#include <QFile>
#include <QTreeView>
#include <QScreen>

#include "treeitem.h"
#include "treemodel.h"

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);

////    MainWindow w;
////    w.show();

//    QStringList numbers;
//    numbers << "one" << "two" << "three";

//    StringListModel *model = new StringListModel(nullptr, numbers);
//    QAbstractItemView *view = new QTableView;

//    view->setModel(model);
//    view->show();

//    return a.exec();

    QApplication app(argc, argv);

    QFile file("default.txt");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    TreeModel model(QString::fromUtf8(file.readAll()));
    file.close();

    QTreeView view;
    view.setModel(&model);
    view.setWindowTitle(TreeModel::tr("Simple Tree Model"));
    for (int c = 0; c < model.columnCount(); ++c)
        view.resizeColumnToContents(c);
    view.expandAll();
    const auto screenSize = view.screen()->availableSize();
    view.resize({screenSize.width() / 2, screenSize.height() * 2 / 3});
    view.show();

    return app.exec();
}
