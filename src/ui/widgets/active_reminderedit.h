#ifndef ACTIVE_REMINDEREDIT_H
#define ACTIVE_REMINDEREDIT_H

#include <QDialog>
#include "core/reminders/reminder.h"
#include <QStringList>
#include <QDateTime>
#include <QTime>
#include "core/providers/priorityiconprovider.h"

namespace Ui {
class ReminderEdit;
}

class ActiveReminderEdit : public QDialog
{
    Q_OBJECT

public:
    explicit ActiveReminderEdit(QWidget *parent = nullptr);
    ~ActiveReminderEdit();

    void prepareEditReminder(const Reminder &reminder);
    void prepareNewReminder();
    Reminder getReminder() const;

private slots:
    void onTypeChanged(int index);
    void onOkClicked();
    void onCancelClicked();
    void onDateTimeChanged(const QDateTime &dateTime);
    void onTimeChanged(const QTime &time);
    void onDaysChanged();

private:
    void setupConnections();
    QDateTime calculateNextTrigger() const;
    bool validateInput() const;
    void updateNextTriggerTime();
    void applyDialogStyle();
    void setupPrioritySelector();

    Ui::ReminderEdit *ui;
    Reminder m_reminder;

};

#endif // ACTIVE_REMINDEREDIT_H 
