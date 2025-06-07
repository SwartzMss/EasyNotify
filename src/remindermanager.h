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

class ReminderManager : public QObject
{
    Q_OBJECT

public:
    explicit ReminderManager(QObject *parent = nullptr);
    ~ReminderManager();

    void addReminder(const QJsonObject &reminder);
    void updateReminder(const QString &id, const QJsonObject &reminder);
    void deleteReminder(const QString &id);
    void pauseAll();
    void resumeAll();
    QJsonArray getReminders() const;
    void saveReminders();
    void loadReminders();

signals:
    void reminderTriggered(const QJsonObject &reminder);
    void remindersChanged();

private slots:
    void checkReminders();
    void onReminderTriggered(const QJsonObject &reminder);

private:
    void setupTimer();
    void calculateNextTrigger(QJsonObject &reminder);
    bool shouldTrigger(const QJsonObject &reminder) const;
    void showNotification(const QJsonObject &reminder);
    void updateReminderNextTrigger(const QString &id, const QDateTime &nextTrigger);
    void initializeReminders();

    QTimer *checkTimer;
    QMap<QString, QJsonObject> reminders;
    bool isPaused;
};

#endif // REMINDERMANAGER_H 