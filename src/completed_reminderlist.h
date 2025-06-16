#ifndef COMPLETED_REMINDERLIST_H
#define COMPLETED_REMINDERLIST_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QJsonObject>
#include <QJsonArray>
#include "completed_reminderedit.h"
#include "remindermanager.h"
#include "completed_remindertablemodel.h"
#include <QPushButton>
#include <QTableView>

QT_BEGIN_NAMESPACE
namespace Ui { class CompletedReminderList; }
QT_END_NAMESPACE

class CompletedReminderList : public QWidget
{
    Q_OBJECT

public:
    explicit CompletedReminderList(QWidget *parent = nullptr);
     ~CompletedReminderList();

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

    Ui::CompletedReminderList *ui;
    ReminderManager *reminderManager;
    CompletedReminderTableModel *model;
    QSortFilterProxyModel *proxyModel;
    CompletedReminderEdit *editDialog;
    QString m_searchText;
};

#endif // COMPLETED_REMINDERLIST_H
