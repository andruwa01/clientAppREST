#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "employeemodel.h"
#include "employeedelegate.h"

#include <QJsonArray>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto *employeeModel = new EmployeeModel(this);
    
    ui->treeView->setItemDelegateForColumn(TreeItem::Column_Employee, new EmployeeDelegate(employeeModel, this));
    auto *model = new TreeItemModel(employeeModel, this);
    ui->treeView->setModel(model);
    ui->treeView->setItemDelegateForColumn(TreeItem::Column_DueDate, new DateDelegate(this));
    
    // tree view settings
    ui->treeView->header()->setStretchLastSection(false);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Title, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Description, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_DueDate, QHeaderView::Fixed);
    ui->treeView->header()->setSectionResizeMode(TreeItem::Column_Status, QHeaderView::Fixed);
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
    for (int i = 0; i < ui->treeView->model()->columnCount(); ++i) {
        ui->treeView->resizeColumnToContents(i);
    }
    
    for (int i = 0; i < ui->tableView->model()->columnCount(); ++i) {
        ui->tableView->resizeColumnToContents(i);
    }

    connect(ui->exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);

    connect(ui->insertTaskAction, &QAction::triggered, this, &MainWindow::insertTask);
    connect(ui->insertSubtaskAction, &QAction::triggered, this, &MainWindow::insertSubtask);
    connect(ui->removeTaskAction, &QAction::triggered, this, &MainWindow::removeTask);

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

    // insert new task
    if (!model->insertRow(index.row() + 1, index.parent()))
    {
        return;
    }

    updateActions();

    // set data to new task
    for (int column = 0; column < model->columnCount(index.parent()); column++)
    {
        const QModelIndex child = model->index(index.row() + 1, column, index.parent());
        model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);
    }
}

void MainWindow::insertSubtask()
{
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->treeView->model();

    // to correct insert row when we choose element with index.column() != 0
    if (index.column() != 0)
    {
        index = index.siblingAtColumn(0);
    }

    // insert new task
    if (!model->insertRow(0, index))
    {
        qCritical() << Q_FUNC_INFO << "model->insertRow() failed";
        return;
    }

    // set data to a new task
    for (int column = 0; column < model->columnCount(index); column++)
    {
        const QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant(tr("[No data]")), Qt::EditRole);

        // set header if there is not
        if (!model->headerData(column, Qt::Horizontal).isValid())
        {
            model->setHeaderData(column, Qt::Horizontal, QVariant(tr("[No header]")), Qt::EditRole);
        }
    }

    // move user selection to inserted task
    ui->treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
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

void MainWindow::updateActions()
{
    // check if we have task to select
    const bool hasSelection = !ui->treeView->selectionModel()->selection().isEmpty();
    ui->removeTaskAction->setEnabled(hasSelection);

    const bool hasCurrent = ui->treeView->selectionModel()->currentIndex().isValid();
//    ui->insertTaskAction->setEnabled(hasCurrent);
    ui->taskCompletedAction->setEnabled(hasCurrent);
    ui->taskIsNotCompletedAction->setEnabled(hasCurrent);

    if (hasCurrent)
    {
        ui->treeView->closePersistentEditor(ui->treeView->selectionModel()->currentIndex());

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
