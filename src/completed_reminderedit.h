#ifndef COMPLETED_REMINDEREDIT_H
#define COMPLETED_REMINDEREDIT_H

#include <QDialog>
#include "reminder.h"
#include <QStringList>
#include <QDateTime>

namespace Ui {
class ReminderEdit;
}

class CompletedReminderEdit : public QDialog
{
    Q_OBJECT

public:
    explicit CompletedReminderEdit(QWidget *parent = nullptr);
    ~CompletedReminderEdit();

    void prepareEditReminder(const Reminder &reminder);
    void prepareNewReminder();
    Reminder getReminder() const;

private slots:
    void onTypeChanged(int index);
    void onOkClicked();
    void onCancelClicked();
    void onDateTimeChanged(const QDateTime &dateTime);
    void onDaysChanged();

private:
    void setupConnections();
    QDateTime calculateNextTrigger() const;
    bool validateInput() const;
    void updateNextTriggerTime();

    Ui::ReminderEdit *ui;
    Reminder m_reminder;

};

#endif // COMPLETED_REMINDEREDIT_H
