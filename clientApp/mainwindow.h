#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "treeitemmodel.h"
#include "datedelegate.h"
#include "apiclient.h"

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
    void refreshData();
    void handleApiError(const QString& error);

private slots:
    void insertTask();
    void insertSubtask();
    void removeTask();
    void insertEmployee();
    void removeEmployee();

private:
    Ui::MainWindow *ui;
    ApiClient* apiClient;
    void setupApiConnections();
};
#endif // MAINWINDOW_H
