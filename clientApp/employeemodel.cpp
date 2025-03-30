#include "employeemodel.h"

EmployeeModel::EmployeeModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // Add some test data
    Employee e1{1, "John Doe", "Developer"};
    Employee e2{2, "Jane Smith", "Manager"};
    Employee e3{3, "Bob Johnson", "Designer"};
    
    addEmployee(e1);
    addEmployee(e2);
    addEmployee(e3);
}

int EmployeeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_employees.size();
}

int EmployeeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return Column_Position + 1; // last enum + 1
}

QVariant EmployeeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const Employee &employee = m_employees.at(index.row());
        switch (index.column()) {
            case Column_Id:
                return employee.id;
            case Column_FullName:
                return employee.fullName;
            case Column_Position:
                return employee.position;
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant EmployeeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case Column_Id:
                return tr("ID");
            case Column_FullName:
                return tr("Full Name");
            case Column_Position:
                return tr("Position");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags EmployeeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool EmployeeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    Employee &employee = m_employees[index.row()];
    switch (index.column()) {
        case Column_Id:
            employee.id = value.toInt();
            break;
        case Column_FullName:
            employee.fullName = value.toString();
            break;
        case Column_Position:
            employee.position = value.toString();
            break;
        default:
            return false;
    }

    emit dataChanged(index, index, {role});
    return true;
}

void EmployeeModel::addEmployee(const Employee &employee)
{
    beginInsertRows(QModelIndex(), m_employees.size(), m_employees.size());
    m_employees.append(employee);
    endInsertRows();
}

void EmployeeModel::removeEmployee(int row)
{
    if (row < 0 || row >= m_employees.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_employees.removeAt(row);
    endRemoveRows();
}

QString EmployeeModel::getEmployeeNameById(int id) const
{
    for (const Employee &emp : m_employees) {
        if (emp.id == id)
            return emp.fullName;
    }
    return QString();
}

int EmployeeModel::getEmployeeIdByRow(int row) const
{
    if (row >= 0 && row < m_employees.size())
        return m_employees[row].id;
    return -1;
}
