#ifndef REMINDEREDIT_H
#define REMINDEREDIT_H

#include <QDialog>
#include "reminder.h"
#include <QStringList>
#include <QDateTime>

namespace Ui {
class ReminderEdit;
}

class ReminderEdit : public QDialog
{
    Q_OBJECT

public:
    explicit ReminderEdit(QWidget *parent = nullptr);
    ~ReminderEdit();

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

#endif // REMINDEREDIT_H 