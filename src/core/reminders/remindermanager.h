#ifndef REMINDERMANAGER_H
#define REMINDERMANAGER_H

#include <QObject>
#include <QTimer>
#include <QJsonArray>
#include "ui/notifications/notificationPopup.h"
#include <QVector>
#include "core/reminders/reminder.h"
#include "core/config/configmanager.h"
#include <QRecursiveMutex>
#include <QMutex>
#include <QMutexLocker>

class ReminderManager : public QObject
{
    Q_OBJECT

public:
    explicit ReminderManager(QObject *parent = nullptr);
    ~ReminderManager();

    void addReminder(const Reminder &reminder);
    void updateReminder(const Reminder &reminder);
    void deleteReminder(const Reminder &reminder);
    QVector<Reminder> getReminders() const;

    void pauseAll();
    void resumeAll();
    void saveReminders();

signals:
    void reminderTriggered(const Reminder &reminder);

private slots:
    void checkReminders();

private:
    void setupTimer();
    void calculateNextTrigger(Reminder &reminder);
    bool shouldTrigger(const Reminder &reminder) const;
    QJsonArray getRemindersJson() const;
    void loadReminders();
    QTimer *checkTimer;
    bool isPaused;
    mutable QRecursiveMutex mutex;
    QVector<Reminder> m_reminders;
};

#endif // REMINDERMANAGER_H 
