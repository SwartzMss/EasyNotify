#ifndef REMINDERTABLEMODEL_H
#define REMINDERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "reminder.h"

class ReminderTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ReminderTableModel(QObject *parent = nullptr);

    // 重写QAbstractTableModel的虚函数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // 提醒管理函数
    void addReminder(const Reminder &reminder);
    void updateReminder(int row, const Reminder &reminder);
    void removeReminder(int row);
    Reminder getReminder(int row) const;
    QVector<Reminder> getAllReminders() const;

    // JSON序列化
    void loadFromJson(const QList<Reminder> &reminders);
    QJsonArray saveToJson() const;

    // 提醒状态管理
    void toggleReminder(int row);

    // 搜索功能
    void search(const QString &text);

private:
    void updateFilteredList();

    QVector<Reminder> m_reminders;
    QVector<Reminder> m_filteredReminders;
    QString m_searchText;
    bool m_isFiltered;
};

#endif // REMINDERTABLEMODEL_H