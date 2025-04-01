#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "employeemodel.h"
#include "employeedelegate.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef USE_API
    apiClient = new ApiClient(this);
#endif

    initializeModels();
    setupTreeView();
    setupTableView();
    setupConnections();

#ifdef USE_API
    setupApiConnections();
#endif
    updateActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupTreeView()
{
    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->header()->setStretchLastSection(false);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Title, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Description, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_DueDate, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Status, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Employee, QHeaderView::Stretch);

    ui->treeView->header()->resizeSection(TreeItem::Column_DueDate, 100);
    ui->treeView->header()->resizeSection(TreeItem::Column_Status, 80);
    ui->treeView->header()->resizeSection(TreeItem::Column_Employee, 200);

    ui->treeView->header()->setMinimumSectionSize(50);
    ui->treeView->setMinimumHeight(300);
    ui->treeView->expandAll();

    for (int i = 0; i < ui->treeView->model()->columnCount(); i++)
    {
        ui->treeView->resizeColumnToContents(i);
    }
}

void MainWindow::setupTableView()
{
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->verticalHeader()->hide();

    ui->tableView->horizontalHeader()->setStretchLastSection(false);
    ui->tableView->horizontalHeader()->setSectionResizeMode(EmployeeModel::Column_Id, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(EmployeeModel::Column_FullName, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(EmployeeModel::Column_Position, QHeaderView::Stretch);

    ui->tableView->horizontalHeader()->resizeSection(EmployeeModel::Column_Id, 50);
    ui->tableView->horizontalHeader()->setMinimumSectionSize(50);
    ui->tableView->setMinimumHeight(200);

    ui->tableView->verticalHeader()->setMinimumSectionSize(25);

    for (int i = 0; i < ui->tableView->model()->columnCount(); i++)
    {
        ui->tableView->resizeColumnToContents(i);
    }
}

void MainWindow::initializeModels()
{
    auto *employeesModel = new EmployeeModel(this);

#ifdef USE_API
    employeesModel->setApiClient(apiClient);
#endif

    ui->tableView->setModel(employeesModel);

    auto *tasksModel = new TreeItemModel(employeesModel, this);
    ui->treeView->setModel(tasksModel);

#ifdef USE_API
    tasksModel->setApiClient(apiClient);
#endif

    ui->treeView->setItemDelegateForColumn(TreeItem::Column_Employee, new EmployeeDelegate(employeesModel, this));
    ui->treeView->setItemDelegateForColumn(TreeItem::Column_DueDate, new DateDelegate(this));
}

void MainWindow::setupConnections()
{
    connect(ui->exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);

    connect(ui->insertTaskAction, &QAction::triggered, this, &MainWindow::insertTask);
    connect(ui->insertSubtaskAction, &QAction::triggered, this, &MainWindow::insertSubtask);
    connect(ui->removeTaskAction, &QAction::triggered, this, &MainWindow::removeTask);

    connect(ui->actionAddEmployee, &QAction::triggered, this, &MainWindow::insertEmployee);
    connect(ui->actionRemoveEmployee, &QAction::triggered, this, &MainWindow::removeEmployee);

    connect(ui->taskCompletedAction, &QAction::triggered, [this]() { handleTaskStatusChange(true); });
    connect(ui->taskIsNotCompletedAction, &QAction::triggered, [this]() { handleTaskStatusChange(false); });

    auto *employeeModel = qobject_cast<EmployeeModel*>(ui->tableView->model());
    if (employeeModel)
    {
        connect(employeeModel, &EmployeeModel::employeeRemoved, [this](int employeeId)
        {
            auto *treeModel = qobject_cast<TreeItemModel*>(ui->treeView->model());
            if (treeModel)
            {
                treeModel->onEmployeeRemoved(employeeId);
            }
        });

        connect(employeeModel, &EmployeeModel::employeeNameChanged, [this](int employeeId)
        {
            auto *treeModel = qobject_cast<TreeItemModel*>(ui->treeView->model());
            if (treeModel)
            {
                treeModel->onEmployeeNameChanged(employeeId);
            }
        });
    }
}

void MainWindow::handleTaskStatusChange(bool completed)
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid())
    {
        qCritical() << Q_FUNC_INFO << ": invalid index";
        return;
    }

    auto *model = qobject_cast<TreeItemModel*>(ui->treeView->model());
    if (completed)
    {
        model->onTaskCompleted(index);
    }
    else
    {
        model->onTaskNotCompleted(index);
    }
    ui->treeView->viewport()->update();
}

void MainWindow::insertTask()
{
    const QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();

    if (!model->insertRow(index.row() + 1, index.parent()))
    {
        return;
    }
    updateActions();
}

void MainWindow::insertSubtask()
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();

    if (index.column() != 0)
    {
        index = index.siblingAtColumn(0);
    }

    if (!model->insertRow(0, index))
    {
        qCritical() << Q_FUNC_INFO << "model->insertRow() failed";
        return;
    }

    ui->treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                                    QItemSelectionModel::ClearAndSelect);
    updateActions();
}

void MainWindow::removeTask()
{
    const QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();
    if (model->removeRow(index.row(), index.parent()))
    {
        updateActions();
    }
}

void MainWindow::insertEmployee()
{
    auto *employeeModel = qobject_cast<EmployeeModel*>(ui->tableView->model());
    if (!employeeModel)
    {
        return;
    }

    Employee newEmployee;
    newEmployee.id = 0;
    newEmployee.fullName = tr("New Employee");
    newEmployee.position = tr("Position");

    employeeModel->addEmployee(newEmployee);

    const QModelIndex newIndex = employeeModel->index(employeeModel->rowCount() - 1, 0);
    ui->tableView->setCurrentIndex(newIndex);
    ui->tableView->edit(newIndex);
}

void MainWindow::removeEmployee()
{
    const QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid())
    {
        return;
    }

    auto *employeeModel = qobject_cast<EmployeeModel*>(ui->tableView->model());
    if (!employeeModel)
    {
        return;
    }

    employeeModel->removeEmployee(index.row());
    updateActions();
}

void MainWindow::updateActions()
{
    const bool hasTaskSelection = !ui->treeView->selectionModel()->selection().isEmpty();
    ui->removeTaskAction->setEnabled(hasTaskSelection);

    const bool hasCurrentTask = ui->treeView->selectionModel()->currentIndex().isValid();
    ui->taskCompletedAction->setEnabled(hasCurrentTask);
    ui->taskIsNotCompletedAction->setEnabled(hasCurrentTask);

    const bool hasEmployeeSelection = ui->tableView->selectionModel() &&
                                      !ui->tableView->selectionModel()->selection().isEmpty();
    ui->actionRemoveEmployee->setEnabled(hasEmployeeSelection);

    if (hasCurrentTask)
    {
        const int row = ui->treeView->selectionModel()->currentIndex().row();
        const int column = ui->treeView->selectionModel()->currentIndex().column();

        if (ui->treeView->selectionModel()->currentIndex().parent().isValid())
        {
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        }
        else
        {
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
        }
    }
}

#ifdef USE_API
void MainWindow::refreshData()
{
    if (!apiClient)
    {
        return;
    }

    auto *employeeModel = qobject_cast<EmployeeModel*>(ui->tableView->model());
    if (employeeModel)
    {
        employeeModel->syncWithServer();
    }

    auto *treeModel = qobject_cast<TreeItemModel*>(ui->treeView->model());
    if (treeModel)
    {
        treeModel->syncWithServer();
    }
}

void MainWindow::setupApiConnections()
{
    if (!apiClient)
    {
        return;
    }

    connect(apiClient, &ApiClient::errorOccurred, this, &MainWindow::handleApiError);

    refreshData();
}

void MainWindow::handleApiError(const QString &error)
{
    QMessageBox::critical(this, tr("Error"), error);
}
#endif
