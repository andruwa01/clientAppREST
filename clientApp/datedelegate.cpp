#include "datedelegate.h"

DateDelegate::DateDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *DateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    if (index.column() != COLUMN_DUE_DATE)
    {
        qCritical() << Q_FUNC_INFO << ": index is not DUE_DATE index";
        return nullptr;
    }

    auto *editor = new QDateEdit(parent);
    editor->setCalendarPopup(true);
    editor->setDisplayFormat(DATE_FORMAT);
    editor->setMinimumDate(QDate::currentDate().addYears(-10));
    editor->setMaximumDate(QDate::currentDate().addYears(10));
    return editor;
}

void DateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto *dateEditor = qobject_cast<QDateEdit *>(editor);
    if (dateEditor)
    {
        const QDate date = QDate::fromString(index.data(Qt::EditRole).toString(), DATE_FORMAT);
        if (date.isValid())
        {
            dateEditor->setDate(date);
        }
        else
        {
            dateEditor->setDate(QDate::currentDate());
        }
    }
}

void DateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto *dateEditor = qobject_cast<QDateEdit *>(editor);
    if (dateEditor)
    {
        model->setData(index, dateEditor->date().toString(DATE_FORMAT), Qt::EditRole);
    }
}

void DateDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
