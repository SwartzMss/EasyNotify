#ifndef REMINDERMANAGER_H
#define REMINDERMANAGER_H

#include <QObject>
#include <QTimer>
#include <QJsonArray>
#include "notificationPopup.h"
#include <QVector>
#include "reminder.h"
#include "configmanager.h"
#include <QThread>
#include <QRecursiveMutex>

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
    QThread *workerThread;
    mutable QRecursiveMutex mutex;
    QVector<Reminder> m_reminders;
};

#endif // REMINDERMANAGER_H 