#include "apiclient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>

ApiClient::ApiClient(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &ApiClient::handleNetworkReply);
}

// Tasks methods
void ApiClient::getTasks() {
    QNetworkRequest request(QUrl(apiUrl + "/tasks"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->get(request);
}

void ApiClient::getTask(int id) {
    QNetworkRequest request(QUrl(apiUrl + "/tasks/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->get(request);
}

void ApiClient::createTask(const Task& task) {
    QNetworkRequest request(QUrl(apiUrl + "/tasks"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(taskToJson(task));
    manager->post(request, doc.toJson());
}

void ApiClient::updateTask(int id, const Task& task) {
    QNetworkRequest request(QUrl(apiUrl + "/tasks/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(taskToJson(task));
    manager->put(request, doc.toJson());
}

void ApiClient::deleteTask(int id) {
    QNetworkRequest request(QUrl(apiUrl + "/tasks/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->deleteResource(request);
}

// Employees methods
void ApiClient::getEmployees() {
    QNetworkRequest request(QUrl(apiUrl + "/employees"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->get(request);
}

void ApiClient::getEmployee(int id) {
    QNetworkRequest request(QUrl(apiUrl + "/employees/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->get(request);
}

void ApiClient::createEmployee(const ApiEmployee& employee) {
    QNetworkRequest request(QUrl(apiUrl + "/employees"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(employeeToJson(employee));
    manager->post(request, doc.toJson());
}

void ApiClient::updateEmployee(int id, const ApiEmployee& employee) {
    QNetworkRequest request(QUrl(apiUrl + "/employees/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(employeeToJson(employee));
    manager->put(request, doc.toJson());
}

void ApiClient::deleteEmployee(int id) {
    QNetworkRequest request(QUrl(apiUrl + "/employees/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->deleteResource(request);
}

// Parsing methods
void ApiClient::handleNetworkReply(QNetworkReply* reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = parseReply(reply);
        QString endpoint = reply->url().path();
        
        if (endpoint == "/api/tasks") {
            emit tasksReceived(parseTasksArray(doc.array()));
        } else if (endpoint.startsWith("/api/tasks/")) {
            emit taskReceived(parseTask(doc.object()));
        } else if (endpoint == "/api/employees") {
            emit employeesReceived(parseEmployeesArray(doc.array()));
        } else if (endpoint.startsWith("/api/employees/")) {
            emit employeeReceived(parseEmployee(doc.object()));
        }
    } else if (statusCode == 404) {
        emit errorOccurred(tr("Resource not found"));
    } else {
        emit errorOccurred(reply->errorString());
    }
    
    reply->deleteLater();
}

QJsonDocument ApiClient::parseReply(QNetworkReply* reply) {
    return QJsonDocument::fromJson(reply->readAll());
}

Task ApiClient::parseTask(const QJsonObject& obj) {
    Task task;
    task.id = obj["id"].toInt();
    task.title = obj["title"].toString();
    task.description = obj["description"].toString();
    task.parentTaskId = obj["parent_task_id"].isNull() ? 0 : obj["parent_task_id"].toInt();
    task.assigneeId = obj["assignee_id"].isNull() ? 0 : obj["assignee_id"].toInt();
    task.dueDate = QDate::fromString(obj["due_date"].toString(), Qt::ISODate);
    task.status = obj["status"].toString();
    return task;
}

ApiEmployee ApiClient::parseEmployee(const QJsonObject& obj) {
    ApiEmployee employee;
    employee.id = obj["id"].toInt();
    employee.fullName = obj["full_name"].toString();
    employee.position = obj["position"].toString();
    return employee;
}

QList<Task> ApiClient::parseTasksArray(const QJsonArray& array) {
    QList<Task> tasks;
    for (const QJsonValue& value : array) {
        tasks.append(parseTask(value.toObject()));
    }
    return tasks;
}

QList<ApiEmployee> ApiClient::parseEmployeesArray(const QJsonArray& array) {
    QList<ApiEmployee> employees;
    for (const QJsonValue& value : array) {
        employees.append(parseEmployee(value.toObject()));
    }
    return employees;
}

QJsonObject ApiClient::taskToJson(const Task& task) {
    QJsonObject obj;
    obj["title"] = task.title;
    obj["description"] = task.description;
    obj["parent_task_id"] = task.parentTaskId == 0 ? QJsonValue::Null : task.parentTaskId;
    obj["assignee_id"] = task.assigneeId == 0 ? QJsonValue::Null : task.assigneeId;
    obj["due_date"] = task.dueDate.toString(Qt::ISODate);
    obj["status"] = task.status;
    return obj;
}

QJsonObject ApiClient::employeeToJson(const ApiEmployee& employee) {
    QJsonObject obj;
    obj["full_name"] = employee.fullName;
    obj["position"] = employee.position;
    return obj;
}
