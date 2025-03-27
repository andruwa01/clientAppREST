#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "treemodel.h"

#include <QMainWindow>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateActions();

private slots:
    void insertTask();
    void insertSubtask();
    void removeTask();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
