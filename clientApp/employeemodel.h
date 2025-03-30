#ifndef EMPLOYEEMODEL_H
#define EMPLOYEEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class Employee
{
public:
    int id = 0; // Changed from -1 to 0
    QString fullName;
    QString position;
};

class EmployeeModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum EmployeeColumns
    {
        Column_Id 		= 0,
        Column_FullName = 1,
        Column_Position = 2,
        Column_Count 	= 3
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

signals:
    void employeeRemoved(int employeeId);
    void employeeNameChanged(int employeeId);  // Добавляем новый сигнал

private:
    QVector<Employee> m_employees;
    int m_nextId = 1;  // For auto-incrementing IDs

    bool isIdUnique(int id) const;
    int generateNextId();
};

#endif // EMPLOYEEMODEL_H
