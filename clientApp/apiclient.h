#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "models.h"
#include "helpdefines.h"

class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);
    
    // Tasks API
    void getTasks();
    void getTask(int id);
    void createTask(const Task& task);
    void updateTask(int id, const Task& task);
    void deleteTask(int id);
    
    // Employees API
    void getEmployees();
    void getEmployee(int id);
    void createEmployee(const ApiEmployee& employee);
    void updateEmployee(int id, const ApiEmployee& employee);
    void deleteEmployee(int id);

    // JSON conversion methods
    QJsonObject taskToJson(const Task& task);
    QJsonObject employeeToJson(const ApiEmployee& employee);

signals:
    void tasksReceived(const QList<Task>& tasks);
    void taskReceived(const Task& task);
    void employeesReceived(const QList<ApiEmployee>& employees);
    void employeeReceived(const ApiEmployee& employee);
    void errorOccurred(const QString& error);
    void taskCreated(const Task& task);
    void taskUpdated(const Task& task);
    void taskDeleted(int id);
    void employeeCreated(const ApiEmployee& employee);
    void employeeUpdated(const ApiEmployee& employee);
    void employeeDeleted(int id);

private:
    QNetworkAccessManager* manager;
    QString apiUrl = "http://localhost:8080/api";

    void handleNetworkReply(QNetworkReply* reply);
    QJsonDocument parseReply(QNetworkReply* reply);
    QList<Task> parseTasksArray(const QJsonArray& array);
    QList<ApiEmployee> parseEmployeesArray(const QJsonArray& array);
    Task parseTask(const QJsonObject& obj);
    ApiEmployee parseEmployee(const QJsonObject& obj);
    QString getRequestMethod(QNetworkReply* reply);
};

#endif // APICLIENT_H
