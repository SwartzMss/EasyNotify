#ifndef COMPLETED_REMINDERLIST_H
#define COMPLETED_REMINDERLIST_H

#include <QWidget>
#include <QSortFilterProxyModel>
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
    QPushButton *deleteButton() const;
    QTableView *tableView() const;

public slots:
    void onDeleteClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupConnections();
    void setupModel();
    void deleteReminder(const QModelIndex &index);
    void refreshList();
    void searchReminders(const QString &text);

    Ui::CompletedReminderList *ui;
    ReminderManager *reminderManager;
    CompletedReminderTableModel *model;
    QSortFilterProxyModel *proxyModel;
    QString m_searchText;
};

#endif // COMPLETED_REMINDERLIST_H
