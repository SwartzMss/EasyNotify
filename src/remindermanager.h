#ifndef REMINDERMANAGER_H
#define REMINDERMANAGER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include "NotificationPopup.h"
#include <QVector>
#include "reminder.h"
#include "configmanager.h"

class ReminderManager : public QObject
{
    Q_OBJECT

public:
    explicit ReminderManager(QObject *parent = nullptr);
    ~ReminderManager();

    void addReminder(const Reminder &reminder);
    void updateReminder(int index, const Reminder &reminder);
    void deleteReminder(int index);
    QVector<Reminder> getReminders() const;
    void toggleReminder(int index);

    void pauseAll();
    void resumeAll();
    QJsonArray getRemindersJson() const;
    void saveReminders();
    void loadReminders();

signals:
    void remindersChanged();
    void reminderTriggered(const Reminder &reminder);

private slots:
    void checkReminders();
    void onReminderTriggered(const Reminder &reminder);

private:
    void setupTimer();
    void calculateNextTrigger(Reminder &reminder);
    bool shouldTrigger(const Reminder &reminder) const;
    void showNotification(const Reminder &reminder);
    void updateReminderNextTrigger(const QString &id, const QDateTime &nextTrigger);
    void initializeReminders();

    QTimer *checkTimer;
    QMap<QString, QJsonObject> reminders;
    bool isPaused;

    QVector<Reminder> m_reminders;
};

#endif // REMINDERMANAGER_H 