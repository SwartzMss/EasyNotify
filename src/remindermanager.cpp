#include "ReminderManager.h"
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
#include <QApplication>
#include <QScreen>
#include <QCoreApplication>
#include "configmanager.h"

ReminderManager::ReminderManager(QObject *parent)
    : QObject(parent)
    , checkTimer(new QTimer(this))
    , isPaused(false)
{
    LOG_INFO("ReminderManager 初始化");
    setupTimer();
    loadReminders();
}

ReminderManager::~ReminderManager()
{
    LOG_INFO("ReminderManager 析构");
    saveReminders();
}

void ReminderManager::setupTimer()
{
    LOG_INFO("设置定时器");
    connect(checkTimer, &QTimer::timeout, this, &ReminderManager::checkReminders);
    checkTimer->start(1000); // 每秒检查一次
}

void ReminderManager::loadReminders()
{
    LOG_INFO("开始加载提醒");
    QJsonArray remindersArray = ConfigManager::instance().getReminders();
    reminders.clear();
    
    for (const QJsonValue &value : remindersArray) {
        if (value.isObject()) {
            QJsonObject reminder = value.toObject();
            QString id = reminder["id"].toString();
            if (!id.isEmpty()) {
                reminders[id] = reminder;
                LOG_INFO(QString("加载提醒: %1, 下次触发时间: %2")
                    .arg(id)
                    .arg(reminder["nextTrigger"].toString()));
            }
        }
    }
    
    // 同步暂停状态
    isPaused = ConfigManager::instance().isPaused();
    
    LOG_INFO(QString("共加载 %1 个提醒").arg(reminders.size()));
}

void ReminderManager::addReminder(const QJsonObject &reminder)
{
    QString id = reminder["id"].toString();
    if (!id.isEmpty()) {
        // 确保提醒对象包含所有必要字段
        QJsonObject newReminder = reminder;
        if (!newReminder.contains("nextTrigger")) {
            QDateTime currentTime = QDateTime::currentDateTime();
            newReminder["nextTrigger"] = currentTime.toString(Qt::ISODate);
        }
        if (!newReminder.contains("type")) {
            newReminder["type"] = QCoreApplication::translate("ReminderManager", "一次性");
        }
        if (!newReminder.contains("title")) {
            newReminder["title"] = id;
        }
        if (!newReminder.contains("message")) {
            newReminder["message"] = QCoreApplication::translate("ReminderManager", "提醒时间到了！");
        }

        reminders[id] = newReminder;
        LOG_INFO(QString("添加提醒: %1, 类型: %2, 下次触发时间: %3")
            .arg(id)
            .arg(newReminder["type"].toString())
            .arg(newReminder["nextTrigger"].toString()));
        saveReminders();
        emit remindersChanged();
    } else {
        LOG_ERROR("尝试添加没有ID的提醒");
    }
}

void ReminderManager::updateReminder(const QString &id, const QJsonObject &reminder)
{
    if (reminders.contains(id)) {
        reminders[id] = reminder;
        LOG_INFO(QString("更新提醒: %1, 类型: %2, 下次触发时间: %3")
            .arg(id)
            .arg(reminder["type"].toString())
            .arg(reminder["nextTrigger"].toString()));
        saveReminders();
        emit remindersChanged();
    }
}

void ReminderManager::deleteReminder(const QString &id)
{
    if (reminders.remove(id) > 0) {
        LOG_INFO(QString("删除提醒: %1").arg(id));
        saveReminders();
        emit remindersChanged();
    }
}

void ReminderManager::pauseAll()
{
    LOG_INFO("暂停所有提醒");
    isPaused = true;
    ConfigManager::instance().setPaused(true);
}

void ReminderManager::resumeAll()
{
    LOG_INFO("恢复所有提醒");
    isPaused = false;
    ConfigManager::instance().setPaused(false);
}

QJsonArray ReminderManager::getReminders() const
{
    QJsonArray array;
    for (const auto &reminder : reminders) {
        array.append(reminder);
    }
    return array;
}

void ReminderManager::saveReminders()
{
    LOG_INFO("保存提醒数据");
    QJsonArray array;
    for (const auto &reminder : reminders) {
        array.append(reminder);
    }
    ConfigManager::instance().setReminders(array);
}

void ReminderManager::checkReminders()
{
    if (isPaused) {
        LOG_DEBUG("提醒检查已暂停");
        return;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    LOG_DEBUG(QString("检查提醒，当前时间: %1").arg(currentTime.toString("yyyy-MM-dd HH:mm:ss")));
    
    for (auto it = reminders.begin(); it != reminders.end(); ++it) {
        QJsonObject reminder = it.value();
        QString id = reminder["id"].toString();
        QString nextTrigger = reminder["nextTrigger"].toString();
        
        LOG_DEBUG(QString("检查提醒 [%1]: 下次触发时间 = %2")
            .arg(id)
            .arg(nextTrigger));
            
        if (shouldTrigger(reminder)) {
            LOG_INFO(QString("触发提醒 [%1]").arg(id));
            showNotification(reminder);
            calculateNextTrigger(reminder);
            it.value() = reminder;
            saveReminders();
        }
    }
}

void ReminderManager::onReminderTriggered(const QJsonObject &reminder)
{
    LOG_INFO(QString("提醒已触发: %1").arg(reminder["id"].toString()));
    emit reminderTriggered(reminder);
}

void ReminderManager::calculateNextTrigger(QJsonObject &reminder)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString type = reminder["type"].toString();
    QDateTime nextTrigger;

    LOG_DEBUG(QString("计算下次触发时间 [%1]: 类型 = %2")
        .arg(reminder["id"].toString())
        .arg(type));

    if (type == QCoreApplication::translate("ReminderManager", "一次性")) {
        // 一次性提醒触发后，将nextTrigger设置为一个很远的未来时间，防止重复触发
        nextTrigger = currentTime.addYears(100);
    } else if (type == QCoreApplication::translate("ReminderManager", "每天")) {
        nextTrigger = currentTime.addDays(1);
    } else if (type == QCoreApplication::translate("ReminderManager", "每周")) {
        nextTrigger = currentTime.addDays(7);
    } else if (type == QCoreApplication::translate("ReminderManager", "每月")) {
        nextTrigger = currentTime.addMonths(1);
    }

    reminder["nextTrigger"] = nextTrigger.toString(Qt::ISODate);
    LOG_INFO(QString("下次触发时间设置为: %1").arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
}

bool ReminderManager::shouldTrigger(const QJsonObject &reminder) const
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime nextTrigger = QDateTime::fromString(reminder["nextTrigger"].toString(), Qt::ISODate);
    
    bool shouldTrigger = nextTrigger <= currentTime;
    LOG_DEBUG(QString("检查提醒 [%1] 是否触发: 当前时间 = %2, 触发时间 = %3, 结果 = %4")
        .arg(reminder["id"].toString())
        .arg(currentTime.toString("yyyy-MM-dd HH:mm:ss"))
        .arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss"))
        .arg(shouldTrigger ? "是" : "否"));
        
    return shouldTrigger;
}

void ReminderManager::showNotification(const QJsonObject &reminder)
{
    QString title = reminder["title"].toString();
    QString message = reminder["message"].toString();
    QIcon icon(":/img/tray_icon.png");
    
    LOG_INFO(QString("显示通知: 标题 = %1, 消息 = %2")
        .arg(title)
        .arg(message));
    
    NotificationPopup *popup = new NotificationPopup(title, message, icon, 5000);
    popup->show();
}

void ReminderManager::updateReminderNextTrigger(const QString &id, const QDateTime &nextTrigger)
{
    if (reminders.contains(id)) {
        QJsonObject reminder = reminders[id];
        reminder["nextTrigger"] = nextTrigger.toString(Qt::ISODate);
        reminders[id] = reminder;
        LOG_INFO(QString("更新提醒 [%1] 的下次触发时间: %2")
            .arg(id)
            .arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
        saveReminders();
    }
} 