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
    // initialize API client
    apiClient = new ApiClient(this);
#endif

    auto *employeeModel = new EmployeeModel(this);
    
#ifdef USE_API
    // connect employee model with API
    employeeModel->setApiClient(apiClient);
#endif

    ui->treeView->setItemDelegateForColumn(TreeItem::Column_Employee, new EmployeeDelegate(employeeModel, this));
    auto *model = new TreeItemModel(employeeModel, this);
    ui->treeView->setModel(model);

#ifdef USE_API
    // connect tree model with API
    model->setApiClient(apiClient);
#endif

    ui->treeView->setItemDelegateForColumn(TreeItem::Column_DueDate, new DateDelegate(this));
    
    // tree view settings
    ui->treeView->header()->setStretchLastSection(false);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Title, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Description, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_DueDate, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Status, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Employee, QHeaderView::Stretch);

    ui->treeView->header()->resizeSection(TreeItem::Column_DueDate, 100);
    ui->treeView->header()->resizeSection(TreeItem::Column_Status, 80);
    ui->treeView->header()->resizeSection(TreeItem::Column_Employee, 200); // increase initial width
    
    ui->treeView->setAlternatingRowColors(true);
    ui->treeView->expandAll();

    // table view settings
    ui->tableView->setModel(employeeModel);
    ui->tableView->horizontalHeader()->setStretchLastSection(false);
    ui->tableView->horizontalHeader()->setSectionResizeMode(EmployeeModel::Column_Id, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(EmployeeModel::Column_FullName, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(EmployeeModel::Column_Position, QHeaderView::Stretch);
    
    // set fixed column sizes for table
    ui->tableView->horizontalHeader()->resizeSection(EmployeeModel::Column_Id, 50);
    
    // set minimum column sizes
    ui->tableView->horizontalHeader()->setMinimumSectionSize(50);
    ui->treeView->header()->setMinimumSectionSize(50);
    
    // set minimum row heights
    ui->tableView->verticalHeader()->setMinimumSectionSize(25);
    ui->treeView->header()->setMinimumSectionSize(25);
    
    // set minimum view sizes
    ui->treeView->setMinimumHeight(300);
    ui->tableView->setMinimumHeight(200);

    // general TableView settings
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->verticalHeader()->hide();
    
    // final size adjustments
    for (int i = 0; i < ui->treeView->model()->columnCount(); i++)
    {
        ui->treeView->resizeColumnToContents(i);
    }
    
    for (int i = 0; i < ui->tableView->model()->columnCount(); i++)
    {
        ui->tableView->resizeColumnToContents(i);
    }

    connect(ui->exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);

    connect(ui->insertTaskAction, &QAction::triggered, this, &MainWindow::insertTask);
    connect(ui->insertSubtaskAction, &QAction::triggered, this, &MainWindow::insertSubtask);
    connect(ui->removeTaskAction, &QAction::triggered, this, &MainWindow::removeTask);
    
    // Add new employee action connections
    connect(ui->actionAddEmployee, &QAction::triggered, this, &MainWindow::insertEmployee);
    connect(ui->actionRemoveEmployee, &QAction::triggered, this, &MainWindow::removeEmployee);

    // Connect employee removal handler
    connect(employeeModel, &EmployeeModel::employeeRemoved,
            [this](int employeeId) 
    {
        auto *treeModel = qobject_cast<TreeItemModel*>(ui->treeView->model());
        if (treeModel)
        {
            treeModel->onEmployeeRemoved(employeeId);
        }
    });

    // Connect employee name change handler
    connect(employeeModel, &EmployeeModel::employeeNameChanged, [this](int employeeId)
    {
                auto *treeModel = qobject_cast<TreeItemModel*>(ui->treeView->model());
                if(treeModel)
                {
                    treeModel->onEmployeeNameChanged(employeeId);
                }
    });

    connect(ui->taskCompletedAction, &QAction::triggered, [this, model]()
    {
        QModelIndex index = ui->treeView->currentIndex();
        if (index.isValid())
        {
            model->onTaskCompleted(index);
            ui->treeView->viewport()->update();
        }
        else
        {
            qCritical() << Q_FUNC_INFO << ": invalid index";
        }
    });

    connect(ui->taskIsNotCompletedAction, &QAction::triggered, [this, model]
    {
        QModelIndex index = ui->treeView->currentIndex();
        if (index.isValid())
        {
            model->onTaskNotCompleted(index);
            ui->treeView->viewport()->update();
        }
        else
        {
            qCritical() << Q_FUNC_INFO << ": invalid index";
        }
    });

#ifdef USE_API
    setupApiConnections();
#endif
    updateActions();
}

MainWindow::~MainWindow()
{
    delete ui;
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
    newEmployee.id = 0;  // ID will be assigned by model/database
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

    // Employee-related actions
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

    // Sync both models with server
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
    
    // initial data load
    refreshData();
}

void MainWindow::handleApiError(const QString& error)
{
    QMessageBox::critical(this, tr("Error"), error);
}
#endif
