#include "apiclient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>

ApiClient::ApiClient(QObject *parent) : QObject(parent) 
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &ApiClient::handleNetworkReply);
}

// Tasks methods
void ApiClient::getTasks() 
{
    QNetworkRequest request(QUrl(apiUrl + "/tasks"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->get(request);
}

void ApiClient::getTask(int id) 
{
    QNetworkRequest request(QUrl(apiUrl + "/tasks/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->get(request);
}

void ApiClient::createTask(const Task& task) 
{
    QNetworkRequest request(QUrl(apiUrl + "/tasks"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(taskToJson(task));
    qDebug() << "\nSending POST request to /tasks:";
    qDebug() << "URL:" << request.url().toString();
    qDebug() << "Content-Type:" << request.header(QNetworkRequest::ContentTypeHeader).toString();
    qDebug() << "Payload:" << doc.toJson(QJsonDocument::Compact);
    
    manager->post(request, doc.toJson());
}

void ApiClient::updateTask(int id, const Task& task) 
{
    QNetworkRequest request(QUrl(apiUrl + "/tasks/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonDocument doc(taskToJson(task));

    qDebug() << "\nupdate task with id: " << id;
    PRINT_DEBUG_JSON(doc);

    manager->put(request, doc.toJson());
}

void ApiClient::deleteTask(int id) 
{
    QNetworkRequest request(QUrl(apiUrl + "/tasks/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->deleteResource(request);
}

// Employees methods
void ApiClient::getEmployees() 
{
    QNetworkRequest request(QUrl(apiUrl + "/employees"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->get(request);
}

void ApiClient::getEmployee(int id) 
{
    QNetworkRequest request(QUrl(apiUrl + "/employees/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->get(request);
}

void ApiClient::createEmployee(const ApiEmployee& employee) 
{
    QNetworkRequest request(QUrl(apiUrl + "/employees"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(employeeToJson(employee));
    manager->post(request, doc.toJson());
}

void ApiClient::updateEmployee(int id, const ApiEmployee& employee) 
{
    QNetworkRequest request(QUrl(apiUrl + "/employees/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(employeeToJson(employee));
    manager->put(request, doc.toJson());
}

void ApiClient::deleteEmployee(int id) 
{
    QNetworkRequest request(QUrl(apiUrl + "/employees/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->deleteResource(request);
}

// Parsing methods
QString ApiClient::getRequestMethod(QNetworkReply* reply) 
{
    switch (reply->operation()) 
    {
        case QNetworkAccessManager::GetOperation:
            return "GET";
        case QNetworkAccessManager::DeleteOperation:
            return "DELETE";
        case QNetworkAccessManager::PostOperation:
            return "POST";
        case QNetworkAccessManager::PutOperation:
            return "PUT";
        default:
            qWarning() << "Unknown request operation:" << reply->operation();
            return "UNKNOWN";
    }
}

void ApiClient::handleNetworkReply(QNetworkReply* reply) 
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString endpoint = reply->url().path();
    QString method = getRequestMethod(reply);
    
    if (reply->error() == QNetworkReply::NoError) 
    {
        if (method == "DELETE") 
        {
            // Handle DELETE operations
            if (endpoint.startsWith("/api/employees/")) 
            {
                int id = endpoint.section('/', -1).toInt();
                emit employeeDeleted(id);
            } 
            else if (endpoint.startsWith("/api/tasks/")) 
            {
                int id = endpoint.section('/', -1).toInt();
                emit taskDeleted(id);
            }
        } 
        else 
        {
            QJsonDocument doc = parseReply(reply);
            
            if (endpoint == "/api/tasks") 
            {
                if (method == "GET")
                {
                    emit tasksReceived(parseTasksArray(doc.array()));
                }
                else if (method == "POST")
                {
                    emit taskCreated(parseTask(doc.object()));
                }
            } 
            else if (endpoint.startsWith("/api/tasks/")) 
            {
                if (method == "GET")
                {
                    emit taskReceived(parseTask(doc.object()));
                }
                else if (method == "PUT")
                {
                    emit taskUpdated(parseTask(doc.object()));
                }
            } 
            else if (endpoint == "/api/employees") 
            {
                if (method == "GET")
                {
                    emit employeesReceived(parseEmployeesArray(doc.array()));
                }
                else if (method == "POST")
                {
                    emit employeeCreated(parseEmployee(doc.object()));
                }
            } 
            else if (endpoint.startsWith("/api/employees/")) 
            {
                if (method == "GET")
                {
                    emit employeeReceived(parseEmployee(doc.object()));
                }
                else if (method == "PUT")
                {
                    emit employeeUpdated(parseEmployee(doc.object()));
                }
            }
        }
    } 
    else if (statusCode == 404) 
    {
        emit errorOccurred(tr("Resource not found"));
    } 
    else 
    {
        emit errorOccurred(reply->errorString());
    }
    
    reply->deleteLater();
}

QJsonDocument ApiClient::parseReply(QNetworkReply* reply) 
{
    return QJsonDocument::fromJson(reply->readAll());
}

Task ApiClient::parseTask(const QJsonObject& obj) 
{
    Task task;
    task.id = obj["id"].toInt();
    
    // Debug output
    qDebug() << "Parsing task from JSON:";
    qDebug() << "Raw id value:" << obj["id"];
    qDebug() << "Parsed id:" << task.id;
    
    task.title = obj["title"].toString();
    task.description = obj["description"].toString();
    task.parentTaskId = obj["parent_task_id"].isNull() ? 0 : obj["parent_task_id"].toInt();
    task.assigneeId = obj["assignee_id"].isNull() ? 0 : obj["assignee_id"].toInt();
    
    // Parse date from API
    QString dateStr = obj["due_date"].toString();
    task.dueDate = QDate::fromString(dateStr, Qt::ISODate);
    qDebug() << "Parsed date from API:" << dateStr 
             << "as" << task.dueDate;
    
    task.status = obj["status"].toString();
    return task;
}

ApiEmployee ApiClient::parseEmployee(const QJsonObject& obj) 
{
    ApiEmployee employee;
    employee.id = obj["id"].toInt();
    employee.fullName = obj["full_name"].toString();
    employee.position = obj["position"].toString();
    return employee;
}

QList<Task> ApiClient::parseTasksArray(const QJsonArray& array) 
{
    QList<Task> tasks;
    for (const QJsonValue& value : array) 
    {
        tasks.append(parseTask(value.toObject()));
    }
    return tasks;
}

QList<ApiEmployee> ApiClient::parseEmployeesArray(const QJsonArray& array) 
{
    QList<ApiEmployee> employees;
    for (const QJsonValue& value : array) 
    {
        employees.append(parseEmployee(value.toObject()));
    }
    return employees;
}

QJsonObject ApiClient::taskToJson(const Task& task) 
{
    // Debug task object
    qDebug() << "\nConverting Task to JSON:";
    qDebug() << "ID:" << task.id;
    qDebug() << "Title:" << task.title;
    qDebug() << "Description:" << task.description;
    qDebug() << "Parent ID:" << task.parentTaskId;
    qDebug() << "Assignee ID:" << task.assigneeId;
    qDebug() << "Due Date:" << task.dueDate.toString(Qt::ISODate);
    qDebug() << "Status:" << task.status;

    QJsonObject obj;
    obj["id"] = task.id;
    obj["title"] = task.title;
    obj["description"] = task.description;
    
    obj.insert("parent_task_id", task.parentTaskId == 0 ? QJsonValue(QJsonValue::Null) : QJsonValue(task.parentTaskId));
    obj.insert("assignee_id", task.assigneeId == 0 ? QJsonValue(QJsonValue::Null) : QJsonValue(task.assigneeId));
    
    // Convert date to ISO format for API
    if (task.dueDate.isValid()) 
    {
        QString isoDate = task.dueDate.toString(Qt::ISODate);
        qDebug() << "Converting date to ISO for API:" << task.dueDate 
                 << "as" << isoDate;
        obj["due_date"] = isoDate;
    } 
    else 
    {
        qWarning() << "Invalid date in task, using current date";
        obj["due_date"] = QDate::currentDate().toString(Qt::ISODate);
    }
    obj["status"] = task.status;
    
    // Debug JSON output
    qDebug() << "\nJSON object details:";
    qDebug() << "parent_task_id type:" << obj["parent_task_id"].type();
    qDebug() << "parent_task_id is null?" << obj["parent_task_id"].isNull();
    qDebug() << "assignee_id type:" << obj["assignee_id"].type();
    qDebug() << "assignee_id is null?" << obj["assignee_id"].isNull();
    qDebug() << "Raw JSON:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);
    
    return obj;
}

QJsonObject ApiClient::employeeToJson(const ApiEmployee& employee) 
{
    QJsonObject obj;
    obj["full_name"] = employee.fullName;
    obj["position"] = employee.position;
    return obj;
}