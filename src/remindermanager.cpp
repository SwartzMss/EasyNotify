#include "ReminderManager.h"
#include <QJsonArray>
#include <QIcon>
#include "logger.h"
#include <QDateTime>
#include "configmanager.h"
#include "reminder.h"
#include <QTimer>

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
    checkTimer->start(5000); // 每秒检查一次
}

void ReminderManager::loadReminders()
{
    LOG_INFO("开始加载提醒");
    QJsonArray reminders = ConfigManager::instance().getReminders();
    m_reminders.clear();
    for (const QJsonValue &value : reminders) {
        if (value.isObject()) {
            m_reminders.append(Reminder::fromJson(value.toObject()));
        }
    }
    
    // 同步暂停状态
    isPaused = ConfigManager::instance().isPaused();
    
    LOG_INFO(QString("共加载 %1 个提醒").arg(m_reminders.size()));
}

void ReminderManager::addReminder(const Reminder &reminder)
{
    m_reminders.append(reminder);
    saveReminders();
}

void ReminderManager::updateReminder(int index, const Reminder &reminder)
{
    if (index >= 0 && index < m_reminders.size()) {
        m_reminders[index] = reminder;
        saveReminders();
    }
}

void ReminderManager::deleteReminder(int index)
{
    if (index >= 0 && index < m_reminders.size()) {
        m_reminders.removeAt(index);
        saveReminders();
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

QVector<Reminder> ReminderManager::getReminders() const
{
    return m_reminders;
}

void ReminderManager::saveReminders()
{
    LOG_INFO("保存提醒数据");
    QJsonArray array;
    for (const Reminder &reminder : m_reminders) {
        array.append(reminder.toJson());
    }
    ConfigManager::instance().setReminders(array);
}

void ReminderManager::checkReminders()
{
    if (isPaused) {
        return;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    LOG_DEBUG(QString("检查提醒，当前时间: %1").arg(currentTime.toString("yyyy-MM-dd HH:mm:ss")));
    
    for (auto it = m_reminders.begin(); it != m_reminders.end(); ++it) {
        Reminder &reminder = *it;
        QString id = reminder.id();
        QDateTime nextTrigger = reminder.nextTrigger();
        
        LOG_DEBUG(QString("检查提醒 [%1]: 下次触发时间 = %2")
            .arg(id)
            .arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
            
        if (shouldTrigger(reminder)) {
            LOG_INFO(QString("触发提醒 [%1]").arg(id));
            showNotification(reminder);
            calculateNextTrigger(reminder);
            saveReminders();
        }
    }
}

void ReminderManager::onReminderTriggered(const Reminder &reminder)
{
    LOG_INFO(QString("提醒已触发: %1").arg(reminder.id()));
    emit reminderTriggered(reminder);
}

void ReminderManager::calculateNextTrigger(Reminder &reminder)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    Reminder::Type type = reminder.type();
    QDateTime nextTrigger;

    if (type == Reminder::Type::Once) {
        // 一次性提醒触发后，标记为已完成
        reminder.setCompleted(true);
        LOG_INFO(QString("一次性提醒已完成，标记 completed"));
        return;
    } else if (type == Reminder::Type::Daily) {
        nextTrigger = currentTime.addDays(1);
        LOG_INFO(QString("每日提醒，下次触发时间: %1").arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
    }

    reminder.setNextTrigger(nextTrigger);
    LOG_INFO(QString("下次触发时间设置为: %1").arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
}

bool ReminderManager::shouldTrigger(const Reminder &reminder) const
{
    if (reminder.completed()) {
        LOG_DEBUG(QString("提醒 [%1] 已完成，跳过触发检查").arg(reminder.id()));
        return false;
    }
    
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime nextTrigger = reminder.nextTrigger();
    
    if (!nextTrigger.isValid()) {
        LOG_ERROR(QString("检查提醒 [%1] 是否触发: 无效的触发时间格式")
            .arg(reminder.id()));
        return false;
    }
    
    // 计算时间差（毫秒）
    qint64 timeDiff = currentTime.msecsTo(nextTrigger);
    bool shouldTrigger = timeDiff <= 0;
    
    LOG_DEBUG(QString("检查提醒 [%1] 是否触发: 当前时间 = %2, 触发时间 = %3, 时间差 = %4ms, 结果 = %5")
        .arg(reminder.id())
        .arg(currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz"))
        .arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss.zzz"))
        .arg(timeDiff)
        .arg(shouldTrigger ? "是" : "否"));
        
    return shouldTrigger;
}

void ReminderManager::showNotification(const Reminder &reminder)
{
    NotificationPopup *popup = new NotificationPopup(
        reminder.name(),
        NotificationPopup::Priority::Information);
    popup->show();
}


QJsonArray ReminderManager::getRemindersJson() const
{
    QJsonArray array;
    for (const Reminder &reminder : m_reminders) {
        array.append(reminder.toJson());
    }
    return array;
}

