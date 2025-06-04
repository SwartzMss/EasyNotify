#include "remindermanager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QIcon>
#include "logger.h"
#include <QJsonObject>
#include <QDir>
#include <QUuid>
#include <QDateTime>

ReminderManager::ReminderManager(QObject *parent)
    : QObject(parent)
    , isPaused(false)
{
    setupTimer();
    loadReminders();
}

ReminderManager::~ReminderManager()
{
    saveReminders();
}

void ReminderManager::setupTimer()
{
    checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, &ReminderManager::checkReminders);
    checkTimer->start(1000); // 每秒检查一次
}

void ReminderManager::addReminder(const QJsonObject &reminder)
{
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QJsonObject reminderWithId = reminder;
    reminderWithId["Id"] = id;
    reminders[id] = reminderWithId;
    saveReminders();
    LOG_INFO(QString("添加提醒: %1").arg(reminder["Name"].toString()));
}

void ReminderManager::updateReminder(const QString &id, const QJsonObject &reminder)
{
    if (!reminders.contains(id)) {
        LOG_WARNING(QString("尝试更新不存在的提醒: %1").arg(id));
        return;
    }
    QJsonObject reminderWithId = reminder;
    reminderWithId["Id"] = id;
    reminders[id] = reminderWithId;
    saveReminders();
    LOG_INFO(QString("更新提醒: %1").arg(reminder["Name"].toString()));
}

void ReminderManager::deleteReminder(const QString &id)
{
    if (!reminders.contains(id)) {
        LOG_WARNING(QString("尝试删除不存在的提醒: %1").arg(id));
        return;
    }
    QString name = reminders[id]["Name"].toString();
    reminders.remove(id);
    saveReminders();
    LOG_INFO(QString("删除提醒: %1").arg(name));
}

void ReminderManager::pauseAll()
{
    isPaused = true;
    checkTimer->stop();
    LOG_INFO("所有提醒已暂停");
}

void ReminderManager::resumeAll()
{
    isPaused = false;
    checkTimer->start();
    LOG_INFO("所有提醒已恢复");
}

void ReminderManager::checkReminders()
{
    if (isPaused) {
        return;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    QList<QString> triggeredIds;

    for (auto it = reminders.begin(); it != reminders.end(); ++it) {
        QJsonObject reminder = it.value();
        if (reminder["IsEnabled"].toBool() && shouldTrigger(reminder)) {
            triggeredIds.append(it.key());
            onReminderTriggered(reminder);
        }
    }

    // 更新已触发的提醒
    for (const QString &id : triggeredIds) {
        QJsonObject &reminder = reminders[id];
        calculateNextTrigger(reminder);
    }

    if (!triggeredIds.isEmpty()) {
        saveReminders();
    }
}

void ReminderManager::onReminderTriggered(const QJsonObject &reminder)
{
    showNotification(reminder);
    emit reminderTriggered(reminder);
}

void ReminderManager::calculateNextTrigger(QJsonObject &reminder)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime nextTime = QDateTime::fromString(reminder["NextTrigger"].toString(), "yyyy-MM-dd HH:mm:ss");
    QString type = reminder["Type"].toString();

    if (type == tr("一次性")) {
        // 一次性提醒触发后自动禁用
        reminder["IsEnabled"] = false;
    } else if (type == tr("每天")) {
        nextTime = nextTime.addDays(1);
    } else if (type == tr("每周")) {
        nextTime = nextTime.addDays(7);
    } else if (type == tr("每月")) {
        nextTime = nextTime.addMonths(1);
    }

    reminder["NextTrigger"] = nextTime.toString("yyyy-MM-dd HH:mm:ss");
}

bool ReminderManager::shouldTrigger(const QJsonObject &reminder) const
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime triggerTime = QDateTime::fromString(reminder["NextTrigger"].toString(), "yyyy-MM-dd HH:mm:ss");
    
    // 允许1秒的误差
    return qAbs(currentTime.secsTo(triggerTime)) <= 1;
}

void ReminderManager::showNotification(const QJsonObject &reminder)
{
    if (!trayIcon) {
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon(":/img/tray_icon.png"));
    }

    QString title = reminder["Name"].toString();
    QString message = tr("提醒时间到了！");
    
    trayIcon->show();
    trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 5000);
}

void ReminderManager::loadReminders()
{
    QFile file("reminders.json");
    if (!file.exists()) {
        LOG_DEBUG("提醒数据文件不存在，将创建新文件");
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("无法读取提醒数据");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        reminders.clear();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            reminders[it.key()] = it.value().toObject();
        }
        LOG_INFO(QString("已加载 %1 个提醒").arg(reminders.size()));
    } else {
        LOG_WARNING("提醒数据格式无效");
    }
}

void ReminderManager::saveReminders()
{
    QFile file("reminders.json");
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("无法保存提醒数据");
        return;
    }

    QJsonObject obj;
    for (auto it = reminders.begin(); it != reminders.end(); ++it) {
        obj[it.key()] = it.value();
    }
    QJsonDocument doc(obj);
    file.write(doc.toJson());
    LOG_DEBUG("提醒数据已保存");
}

QJsonArray ReminderManager::getReminders() const
{
    QJsonArray array;
    for (const auto& reminder : reminders) {
        array.append(reminder);
    }
    return array;
} 