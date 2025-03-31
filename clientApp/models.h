#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QDate>

struct Employee
{
    int id;
    QString fullName;
    QString position;
};

struct Task
{
    int id;
    QString title;
    QString description;
    int parentTaskId;
    int assigneeId;
    QDate dueDate;
    QString status;
};

#endif // MODELS_H
