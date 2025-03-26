#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const QStringList headers({tr("Title"), tr("Description"), tr("Assignee to"), tr("Due date"), tr("Status")});

    // get JSON from REST API

    // read json from the file
    QFile file(":/testTasks.json");
    const bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    Q_ASSERT_X(res, Q_FUNC_INFO, "failed to open json file");
    if (!res) return;
    // read json to model
    QString jsonLines = QString::fromUtf8(file.readAll());

    // load json to model
//    auto *model = new TreeModel(headers);

//    qDebug() << "json contents:\n" << jsonLines;
    file.close();



    connect(ui->exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

MainWindow::~MainWindow()
{
    delete ui;
}
