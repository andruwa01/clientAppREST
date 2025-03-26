#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonArray>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const QStringList headers({tr("Название"), tr("Описание"), tr("Назначено на"), tr("Дата выполнения"), tr("Статус задачи")});

    // get JSON from REST API

    // read json from the file
    QFile file(":/testTasks.json");
    const bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    Q_ASSERT_X(res, Q_FUNC_INFO, "failed to open json file");
    if (!res) return;
    // read json to model
    QString jsonLines = QString::fromUtf8(file.readAll());

    qDebug() << "json contents:\n" << jsonLines;

    // form json to valid format
    if (jsonLines.startsWith("\"") && jsonLines.endsWith("\""))
    {
        jsonLines = jsonLines.mid(1, jsonLines.length() - 2);  // remove outer ""
    }
    jsonLines.replace("\\\"", "\"");  // remove \\\"
    jsonLines = jsonLines.trimmed();  // remove spaces and \n

    qDebug() << "json contents after:\n" << jsonLines;

    // try to parse
    QByteArray jsonData = jsonLines.toUtf8();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    doc.isNull() ? qDebug() << "fail to parse json" : qDebug() << "success parse json, size: " << doc.array().size();

    // load json to model
    auto *model = new TreeModel(headers, jsonData);

    file.close();



    connect(ui->exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

MainWindow::~MainWindow()
{
    delete ui;
}
