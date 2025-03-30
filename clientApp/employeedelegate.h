#ifndef EMPLOYEEDELEGATE_H
#define EMPLOYEEDELEGATE_H

#include <QStyledItemDelegate>
#include <QApplication>

class EmployeeModel;

class EmployeeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit EmployeeDelegate(EmployeeModel *employeeModel, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    EmployeeModel *m_employeeModel;
};

#endif // EMPLOYEEDELEGATE_H
