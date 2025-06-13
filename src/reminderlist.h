#ifndef REMINDERLIST_H
#define REMINDERLIST_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QJsonObject>
#include <QJsonArray>
#include "reminderedit.h"
#include "remindermanager.h"
#include "remindertablemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ReminderList; }
QT_END_NAMESPACE

class ReminderList : public QWidget
{
    Q_OBJECT

public:
    explicit ReminderList(QWidget *parent = nullptr);
    ~ReminderList();

    void setReminderManager(ReminderManager *manager);
    void loadReminders(const QList<Reminder> &reminders);
    QJsonArray getReminders() const;
    void addNewReminder();
    void editReminder(const QModelIndex &index);
    void deleteReminder(const QModelIndex &index);
    void toggleReminder(const QModelIndex &index);
    void refreshList();
    void searchReminders(const QString &text);
    QJsonObject getReminderData(const QString &name) const;

public slots:
    void onReminderTriggered(const Reminder &reminder);
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onImportClicked();
    void onExportClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupConnections();
    void setupModel();
    void addReminderToModel(const QJsonObject &reminder);
    void updateReminderInModel(const QJsonObject &reminder);
    void loadReminders();
    void saveReminders();

    Ui::ReminderList *ui;
    ReminderManager *reminderManager;
    ReminderTableModel *model;
    QSortFilterProxyModel *proxyModel;
    QString m_searchText;
};

#endif // REMINDERLIST_H 