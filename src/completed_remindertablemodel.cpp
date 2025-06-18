#include "completed_remindertablemodel.h"
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>

CompletedReminderTableModel::CompletedReminderTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_isFiltered(false)
{
}

int CompletedReminderTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_isFiltered ? m_filteredReminders.size() : m_reminders.size();
}

int CompletedReminderTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 4;
}

QVariant CompletedReminderTableModel::data(const QModelIndex &index, int role) const
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
            switch (reminder.priority()) {
            case Reminder::Priority::Low: return "低";
            case Reminder::Priority::High: return "高";
            case Reminder::Priority::Medium:
            default: return "中";
            }
        case 3:
            return reminder.nextTrigger().toString("yyyy-MM-dd hh:mm:ss");
        }
    }
    return QVariant();
}

QVariant CompletedReminderTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "名称";
        case 1: return "类型";
        case 2: return "优先级";
        case 3: return "完成时间";
        }
    }
    return QVariant();
}

bool CompletedReminderTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

Qt::ItemFlags CompletedReminderTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == 0)
        flags |= Qt::ItemIsEditable;
    return flags;
}

void CompletedReminderTableModel::addReminder(const Reminder &reminder)
{
    beginInsertRows(QModelIndex(), m_reminders.size(), m_reminders.size());
    m_reminders.append(reminder);
    endInsertRows();
    updateFilteredList();
}

void CompletedReminderTableModel::updateReminder(int row, const Reminder &reminder)
{
    if (row < 0 || row >= m_reminders.size())
        return;

    m_reminders[row] = reminder;
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
    updateFilteredList();
}

void CompletedReminderTableModel::removeReminder(int row)
{
    if (row < 0 || row >= m_reminders.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_reminders.removeAt(row);
    endRemoveRows();
    updateFilteredList();
}

void CompletedReminderTableModel::removeReminders(const QList<QPair<int, Reminder>> &reminders)
{
    if (reminders.isEmpty()) {
        return;
    }

    // 按行号从大到小排序，这样删除时不会影响其他行的索引
    QList<int> rowsToRemove;
    for (const auto &pair : reminders) {
        rowsToRemove.append(pair.first);
    }
    std::sort(rowsToRemove.begin(), rowsToRemove.end(), std::greater<int>());

    // 一次性通知视图将要删除多行
    beginRemoveRows(QModelIndex(), rowsToRemove.last(), rowsToRemove.first());
    
    // 从大到小删除，这样不会影响前面的索引
    for (int row : rowsToRemove) {
        if (row >= 0 && row < m_reminders.size()) {
            m_reminders.removeAt(row);
        }
    }
    
    endRemoveRows();
    updateFilteredList();
}

Reminder CompletedReminderTableModel::getReminder(int row) const
{
    if (row < 0 || row >= m_reminders.size())
        return Reminder();
    return m_reminders[row];
}

QVector<Reminder> CompletedReminderTableModel::getAllReminders() const
{
    return m_reminders;
}

void CompletedReminderTableModel::loadFromJson(const QList<Reminder> &reminders)
{
    beginResetModel();
    m_reminders = reminders;
    endResetModel();
}

QJsonArray CompletedReminderTableModel::saveToJson() const
{
    QJsonArray array;
    for (const Reminder &reminder : m_reminders) {
        array.append(reminder.toJson());
    }
    return array;
}


void CompletedReminderTableModel::search(const QString &text)
{
    m_searchText = text;
    updateFilteredList();
}

void CompletedReminderTableModel::updateFilteredList()
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
