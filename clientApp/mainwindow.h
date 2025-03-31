#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "treeitemmodel.h"
#include "datedelegate.h"
#include "helpdefines.h"

#ifdef USE_API
#include "apiclient.h"
#endif

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
#ifdef USE_API
    void refreshData();
    void handleApiError(const QString& error);
#endif

private slots:
    void insertTask();
    void insertSubtask();
    void removeTask();
    void insertEmployee();
    void removeEmployee();

private:
    Ui::MainWindow *ui;
#ifdef USE_API
    ApiClient* apiClient;
    void setupApiConnections();
#endif
};
#endif // MAINWINDOW_H
