#ifndef ACTIVE_REMINDERLIST_H
#define ACTIVE_REMINDERLIST_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QJsonObject>
#include <QJsonArray>
#include "active_reminderedit.h"
#include "remindermanager.h"
#include "active_remindertablemodel.h"
#include <QPushButton>
#include <QTableView>

QT_BEGIN_NAMESPACE
namespace Ui { class ReminderList; }
QT_END_NAMESPACE

class ActiveReminderList : public QWidget
{
    Q_OBJECT

public:
    explicit ActiveReminderList(QWidget *parent = nullptr);
     ~ActiveReminderList();

    void setReminderManager(ReminderManager *manager);
    void loadReminders(const QList<Reminder> &reminders);
    QJsonArray getReminders() const;
    QPushButton *addButton() const;
    QPushButton *deleteButton() const;
    QPushButton *importButton() const;
    QPushButton *exportButton() const;
    QTableView *tableView() const;

public slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onImportClicked();
    void onExportClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupConnections();
    void setupModel();
    void addNewReminder();
    void editReminder(const QModelIndex &index);
    void deleteReminder(const QModelIndex &index);
    void refreshList();
    void searchReminders(const QString &text);
    QJsonObject getReminderData(const QString &name) const;
    void addReminderToModel(const Reminder &reminder);
    void updateReminderInModel(const Reminder &reminder);

    Ui::ReminderList *ui;
    ReminderManager *reminderManager;
    ActiveReminderTableModel *model;
    QSortFilterProxyModel *proxyModel;
    ActiveReminderEdit *editDialog;
    QString m_searchText;
};

#endif // ACTIVE_REMINDERLIST_H
