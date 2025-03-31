#include "employeemodel.h"

EmployeeModel::EmployeeModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    #ifdef TEST_JSON_INPUT

    // Test data initialization
    Employee e1{1, "John Doe", "Developer"};
    Employee e2{2, "Jane Smith", "Manager"};
    Employee e3{3, "Bob Johnson", "Designer"};
    Employee e5{5, "Andrew Cheprasov", "Programmer"};

    addEmployee(e1);
    addEmployee(e2);
    addEmployee(e3);
    addEmployee(e5);

    #endif
}

int EmployeeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_employees.size();
}

int EmployeeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return Column_Position + 1; // last enum + 1
}

QVariant EmployeeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) 
    {
        const Employee &employee = m_employees.at(index.row());
        switch (index.column()) 
        {
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
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) 
    {
        switch (section) 
        {
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
    {
        return Qt::NoItemFlags;
    }

    // Make id column non-editable
    if (index.column() == Column_Id)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool EmployeeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
    {
        return false;
    }

    Employee &employee = m_employees[index.row()];
    bool changed = false;

    switch (index.column()) 
    {
        case Column_FullName:
            if (employee.fullName != value.toString())
            {
                employee.fullName = value.toString();
                changed = true;
            }
            break;
        case Column_Position:
            if (employee.position != value.toString())
            {
                employee.position = value.toString();
                changed = true;
            }
            break;
        default:
            return false;
    }

    if (changed)
    {
#ifdef USE_API
        if (m_apiClient)
        {
            m_apiClient->updateEmployee(employee.id, toApiEmployee(employee));
        }
#endif
        emit dataChanged(index, index, {role});
        if (index.column() == Column_FullName)
        {
            emit employeeNameChanged(employee.id);
        }
    }

    return changed;
}

void EmployeeModel::addEmployee(const Employee &employee)
{
    Employee newEmployee = employee;

    if (employee.id == 0)
    {
        newEmployee.id = generateNextId();
    }
    else if (!isIdUnique(employee.id))
    {
        qWarning() << "Employee with id" << employee.id << "already exists. Generating new id.";
        newEmployee.id = generateNextId();
    }

#ifdef USE_API
    if (m_apiClient && employee.id == 0)
    {
        m_apiClient->createEmployee(toApiEmployee(newEmployee));
        return; // Will be added asynchronously after server response
    }
#endif

    beginInsertRows(QModelIndex(), m_employees.size(), m_employees.size());
    m_employees.append(newEmployee);
    endInsertRows();
}

bool EmployeeModel::isIdUnique(int id) const
{
    return std::none_of(m_employees.begin(), m_employees.end(),
                       [id](const Employee &emp) { return emp.id == id; });
}

int EmployeeModel::generateNextId()
{
    int maxId = 0;
    for (const auto &emp : m_employees)
    {
        if (emp.id > maxId)
        {
            maxId = emp.id;
        }
    }
    return maxId + 1;
}

void EmployeeModel::removeEmployee(int row)
{
    if (row < 0 || row >= m_employees.size())
    {
        return;
    }

    int employeeId = m_employees[row].id;
    
#ifdef USE_API
    if (m_apiClient)
    {
        m_apiClient->deleteEmployee(employeeId);
        return; // Actual removal will happen in handleEmployeeDeleted
    }
#endif

    beginRemoveRows(QModelIndex(), row, row);
    m_employees.removeAt(row);
    endRemoveRows();
}

QString EmployeeModel::getEmployeeNameById(int id) const
{
    for (const Employee &emp : m_employees) 
    {
        if (emp.id == id)
        {
            return emp.fullName;
        }
    }
    return QString();
}

int EmployeeModel::getEmployeeIdByRow(int row) const
{
    if (row >= 0 && row < m_employees.size())
    {
        return m_employees[row].id;
    }
    return -1;
}

#ifdef USE_API
void EmployeeModel::setApiClient(ApiClient* client)
{
    m_apiClient = client;
    
    if (m_apiClient)
    {
        connect(m_apiClient, &ApiClient::employeesReceived, this, &EmployeeModel::handleEmployeesReceived);
        connect(m_apiClient, &ApiClient::employeeCreated, this, &EmployeeModel::handleEmployeeCreated);
        connect(m_apiClient, &ApiClient::employeeUpdated, this, &EmployeeModel::handleEmployeeUpdated);
        connect(m_apiClient, &ApiClient::employeeDeleted, this, &EmployeeModel::handleEmployeeDeleted);
    }
}

void EmployeeModel::syncWithServer()
{
    if (m_apiClient)
    {
        m_apiClient->getEmployees();
    }
}

void EmployeeModel::handleEmployeesReceived(const QList<ApiEmployee>& apiEmployees)
{
    beginResetModel();
    m_employees.clear();
    
    for (const auto& apiEmployee : apiEmployees)
    {
        m_employees.append(fromApiEmployee(apiEmployee));
    }
    endResetModel();
}

void EmployeeModel::handleEmployeeCreated(const ApiEmployee& employee)
{
    addEmployee(fromApiEmployee(employee));
}

void EmployeeModel::handleEmployeeUpdated(const ApiEmployee& employee)
{
    for (int i = 0; i < m_employees.size(); ++i) {
        if (m_employees[i].id == employee.id) {
            m_employees[i] = fromApiEmployee(employee);
            emit dataChanged(index(i, 0), index(i, Column_Count - 1));
            emit employeeNameChanged(employee.id);
            break;
        }
    }
}

void EmployeeModel::handleEmployeeDeleted(int id)
{
    for (int i = 0; i < m_employees.size(); i++)
    {
        if (m_employees[i].id == id)
        {
            beginRemoveRows(QModelIndex(), i, i);
            m_employees.removeAt(i);
            endRemoveRows();
            emit employeeRemoved(id);
            break;
        }
    }
}

Employee EmployeeModel::fromApiEmployee(const ApiEmployee& apiEmployee)
{
    Employee employee;
    employee.id = apiEmployee.id;
    employee.fullName = apiEmployee.fullName;
    employee.position = apiEmployee.position;
    return employee;
}

ApiEmployee EmployeeModel::toApiEmployee(const Employee& employee)
{
    ApiEmployee apiEmployee;
    apiEmployee.id = employee.id;
    apiEmployee.fullName = employee.fullName;
    apiEmployee.position = employee.position;
    return apiEmployee;
}
#endif
