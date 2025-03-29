#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonArray>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto *model = new TreeItemModel();
    ui->view->setModel(model);

    for (int column = 0; column < model->columnCount(); column++)
    {
        ui->view->resizeColumnToContents(column);
    }
    ui->view->expandAll();


    connect(ui->exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(ui->view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);

    connect(ui->insertTaskAction, &QAction::triggered, this, &MainWindow::insertTask);
    connect(ui->insertSubtaskAction, &QAction::triggered, this, &MainWindow::insertSubtask);
    connect(ui->removeTaskAction, &QAction::triggered, this, &MainWindow::removeTask);

    connect(ui->taskCompletedAction, &QAction::triggered, [this, model]()
    {
        QModelIndex index = ui->view->currentIndex();
        if (index.isValid())
        {
            model->onTaskCompleted(index);
            ui->view->viewport()->update();
        }
        else
        {
            qCritical() << Q_FUNC_INFO << ": invalid index";
        }
    });

    connect(ui->taskIsNotCompletedAction, &QAction::triggered, [this, model]
    {
        QModelIndex index = ui->view->currentIndex();
        if (index.isValid())
        {
            model->onTaskNotCompleted(index);
            ui->view->viewport()->update();
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
    const QModelIndex index = ui->view->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->view->model();

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
    QModelIndex index = ui->view->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->view->model();

    // to correct insert row when we chose element with index.column() != 0
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
    ui->view->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    updateActions();
}

void MainWindow::removeTask()
{
    const QModelIndex index = ui->view->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->view->model();
    if (model->removeRow(index.row(), index.parent()))
    {
        updateActions();
    }
}

void MainWindow::updateActions()
{
    // check if we have task to select
    const bool hasSelection = !ui->view->selectionModel()->selection().isEmpty();
    ui->removeTaskAction->setEnabled(hasSelection);

    const bool hasCurrent = ui->view->selectionModel()->currentIndex().isValid();
//    ui->insertTaskAction->setEnabled(hasCurrent);
    ui->taskCompletedAction->setEnabled(hasCurrent);
    ui->taskIsNotCompletedAction->setEnabled(hasCurrent);

    if (hasCurrent)
    {
        ui->view->closePersistentEditor(ui->view->selectionModel()->currentIndex());

        const int row = ui->view->selectionModel()->currentIndex().row();
        const int column = ui->view->selectionModel()->currentIndex().column();

        if (ui->view->selectionModel()->currentIndex().parent().isValid())
        {
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        }
        else
        {
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
        }
    }
}
