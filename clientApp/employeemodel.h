#ifndef EMPLOYEEMODEL_H
#define EMPLOYEEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "helpdefines.h"

#ifdef USE_API
#include "apiclient.h"
#endif

class Employee
{
public:
    int id = 0;
    QString fullName;
    QString position;
};

class EmployeeModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    // model columns
    enum EmployeeColumns
    {
        Column_Id        = 0,
        Column_FullName  = 1,
        Column_Position  = 2,
        Column_Count     = 3
    };

    explicit EmployeeModel(QObject *parent = nullptr);

    // model interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // custom methods
    void addEmployee(const Employee &employee);
    void removeEmployee(int row);
    const QVector<Employee>& getEmployees() const { return m_employees; }
    QString getEmployeeNameById(int id) const;
    int getEmployeeIdByRow(int row) const;

#ifdef USE_API
    // API integration methods
    void setApiClient(ApiClient* client);
    void syncWithServer();
#endif

signals:
    void employeeRemoved(int employeeId);
    void employeeNameChanged(int employeeId);

#ifdef USE_API
private slots:
    void handleEmployeesReceived(const QList<ApiEmployee>& employees);
    void handleEmployeeCreated(const ApiEmployee& employee);
    void handleEmployeeUpdated(const ApiEmployee& employee);
    void handleEmployeeDeleted(int id);
#endif

private:
    QVector<Employee> m_employees;
    int m_nextId = 1;

#ifdef USE_API
    ApiClient* m_apiClient = nullptr;
#endif

    bool isIdUnique(int id) const;
    int generateNextId();

#ifdef USE_API
    // Conversion helpers
    static Employee fromApiEmployee(const ApiEmployee& apiEmployee);
    static ApiEmployee toApiEmployee(const Employee& employee);
#endif
};

#endif // EMPLOYEEMODEL_H
