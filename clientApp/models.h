#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QDate>

struct ApiEmployee
{
    int id;
    QString fullName;
    QString position;
};

struct Task
{
    int id = 0;
    QString title;
    QString description;
    int parentTaskId = 0;
    int assigneeId = 0;
    QDate dueDate = QDate::currentDate();
    QString status = "new";
};

#endif // MODELS_H
