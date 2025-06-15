#include "active_remindertablemodel.h"
#include <QJsonArray>
#include <QJsonObject>

ActiveReminderTableModel::ActiveReminderTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_isFiltered(false)
{
}

int ActiveReminderTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_isFiltered ? m_filteredReminders.size() : m_reminders.size();
}

int ActiveReminderTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 3;
}

QVariant ActiveReminderTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Reminder &reminder = m_isFiltered ? m_filteredReminders[index.row()] : m_reminders[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            return reminder.name();
        case 1:
            switch (reminder.type()) {
            case Reminder::Type::Once: return "一次性";
            case Reminder::Type::Daily: return "每天";
            default: return "未知";
            }
        case 2:
            return reminder.nextTrigger().toString("yyyy-MM-dd hh:mm:ss");
        }
    }
    return QVariant();
}

QVariant ActiveReminderTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "名称";
        case 1: return "类型";
        case 2: return "下次触发时间";
        }
    }
    return QVariant();
}

bool ActiveReminderTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    int row = index.row();
    if (m_isFiltered) {
        if (row >= m_filteredReminders.size())
            return false;
        row = m_reminders.indexOf(m_filteredReminders[row]);
    }

    if (row >= m_reminders.size())
        return false;

    Reminder &reminder = m_reminders[row];
    switch (index.column()) {
    case 0:
        reminder.setName(value.toString());
        break;
    default:
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags ActiveReminderTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == 0)
        flags |= Qt::ItemIsEditable;
    return flags;
}

void ActiveReminderTableModel::addReminder(const Reminder &reminder)
{
    beginInsertRows(QModelIndex(), m_reminders.size(), m_reminders.size());
    m_reminders.append(reminder);
    endInsertRows();
    updateFilteredList();
}

void ActiveReminderTableModel::updateReminder(int row, const Reminder &reminder)
{
    if (row < 0 || row >= m_reminders.size())
        return;

    m_reminders[row] = reminder;
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
    updateFilteredList();
}

void ActiveReminderTableModel::removeReminder(int row)
{
    if (row < 0 || row >= m_reminders.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_reminders.removeAt(row);
    endRemoveRows();
    updateFilteredList();
}

Reminder ActiveReminderTableModel::getReminder(int row) const
{
    if (row < 0 || row >= m_reminders.size())
        return Reminder();
    return m_reminders[row];
}

QVector<Reminder> ActiveReminderTableModel::getAllReminders() const
{
    return m_reminders;
}

void ActiveReminderTableModel::loadFromJson(const QList<Reminder> &reminders)
{
    beginResetModel();
    m_reminders = reminders;
    endResetModel();
}

QJsonArray ActiveReminderTableModel::saveToJson() const
{
    QJsonArray array;
    for (const Reminder &reminder : m_reminders) {
        array.append(reminder.toJson());
    }
    return array;
}


void ActiveReminderTableModel::search(const QString &text)
{
    m_searchText = text;
    updateFilteredList();
}

void ActiveReminderTableModel::updateFilteredList()
{
    if (m_searchText.isEmpty()) {
        m_isFiltered = false;
        emit layoutChanged();
        return;
    }

    m_filteredReminders.clear();
    for (const Reminder &reminder : m_reminders) {
        if (reminder.name().contains(m_searchText, Qt::CaseInsensitive)) {
            m_filteredReminders.append(reminder);
        }
    }
    m_isFiltered = true;
    emit layoutChanged();
}
