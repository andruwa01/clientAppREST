#include "employeedelegate.h"
#include "employeemodel.h"
#include <QComboBox>

EmployeeDelegate::EmployeeDelegate(EmployeeModel *employeeModel, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_employeeModel(employeeModel)
{
}

QWidget* EmployeeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    QComboBox *editor = new QComboBox(parent);
    
    // Fill employee names
    const auto& employees = m_employeeModel->getEmployees();
    editor->addItem("Not Assigned", 0);
    
    for (const Employee& emp : employees) 
    {
        editor->addItem(emp.fullName, emp.id);
    }
    
    return editor;
}

void EmployeeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
    {
        return;
    }

    int currentEmployeeId = index.data(Qt::EditRole).toInt();
    int idx = comboBox->findData(currentEmployeeId);
    if (idx != -1) 
    {
        comboBox->setCurrentIndex(idx);
    }
}

void EmployeeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
    {
        return;
    }
        
    int employeeId = comboBox->currentData().toInt();
    model->setData(index, employeeId, Qt::EditRole);
}

void EmployeeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

void EmployeeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return;
    }

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    int employeeId = index.data(Qt::DisplayRole).toInt();
    QString displayText = employeeId != 0 ?
                         m_employeeModel->getEmployeeNameById(employeeId) : 
                         "Not Assigned";
    
    opt.text = displayText;
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}
