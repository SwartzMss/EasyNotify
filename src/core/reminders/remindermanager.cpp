#include "core/reminders/remindermanager.h"
#include <QJsonArray>
#include <QIcon>
#include "core/logging/logger.h"
#include <QDateTime>
#include "core/config/configmanager.h"
#include "core/reminders/reminder.h"
#include <QTimer>
#include <QMetaType>
#include "core/calendar/workdaycalendar.h"

ReminderManager::ReminderManager(QObject *parent)
    : QObject(nullptr)
    , checkTimer(new QTimer(this))
    , isPaused(false)
    , workerThread(new QThread)
{
    Q_UNUSED(parent);
    qRegisterMetaType<Reminder>("Reminder");
    LOG_INFO("ReminderManager 初始化");
    setupTimer();
    loadReminders();
    moveToThread(workerThread);
    connect(workerThread, &QThread::started, checkTimer, qOverload<>(&QTimer::start));
    workerThread->start();
}

ReminderManager::~ReminderManager()
{
    LOG_INFO("ReminderManager 析构");
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
        workerThread = nullptr;
    }
    saveReminders();
}

void ReminderManager::setupTimer()
{
    LOG_INFO("设置定时器");
    connect(checkTimer, &QTimer::timeout, this, &ReminderManager::checkReminders);
    checkTimer->setInterval(5000); // 每5秒检查一次
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
    QMutexLocker locker(&mutex);
    for (const Reminder &r : m_reminders) {
        if (r.id() == reminder.id()) {
            LOG_WARNING(QString("尝试添加重复的提醒 ID: %1").arg(reminder.id()));
            return;
        }
    }
    m_reminders.append(reminder);
    saveReminders();
}

void ReminderManager::updateReminder(const Reminder &reminder)
{
    QMutexLocker locker(&mutex);
    for (int i = 0; i < m_reminders.size(); ++i) {
        if (m_reminders[i].id() == reminder.id()) {
            m_reminders[i] = reminder;
            saveReminders();
            break;
        }
    }
}

void ReminderManager::deleteReminder(const Reminder &reminder)
{
    QMutexLocker locker(&mutex);
    for (int i = 0; i < m_reminders.size(); ++i) {
        if (m_reminders[i].id() == reminder.id()) {
            m_reminders.removeAt(i);
            saveReminders();
            break;
        }
    }
}

void ReminderManager::pauseAll()
{
    LOG_INFO("暂停所有提醒");
    QMutexLocker locker(&mutex);
    isPaused = true;
    ConfigManager::instance().setPaused(true);
}

void ReminderManager::resumeAll()
{
    LOG_INFO("恢复所有提醒");
    QMutexLocker locker(&mutex);
    isPaused = false;
    ConfigManager::instance().setPaused(false);
}

QVector<Reminder> ReminderManager::getReminders() const
{
    QMutexLocker locker(&mutex);
    return m_reminders;
}

void ReminderManager::saveReminders()
{
    LOG_INFO("保存提醒数据");
    QMutexLocker locker(&mutex);
    QJsonArray array;
    for (const Reminder &reminder : m_reminders) {
        array.append(reminder.toJson());
    }
    ConfigManager::instance().setReminders(array);
}

void ReminderManager::checkReminders()
{
    QMutexLocker locker(&mutex);
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
            emit reminderTriggered(reminder);
            calculateNextTrigger(reminder);
            saveReminders();
        }
    }
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
        nextTrigger = reminder.nextTrigger().addDays(1);
        LOG_INFO(QString("每日提醒，下次触发时间: %1").arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
    } else if (type == Reminder::Type::Workday) {
        const bool hasValidTrigger = reminder.nextTrigger().isValid();
        QDate baseDate = hasValidTrigger
            ? reminder.nextTrigger().date().addDays(1)
            : QDate::currentDate();
        if (!baseDate.isValid()) {
            baseDate = QDate::currentDate();
        }
        WorkdayCalendar &calendar = WorkdayCalendar::instance();
        const QDate nextDate = calendar.nextWorkday(baseDate, true);
        const QTime triggerTime = hasValidTrigger ? reminder.nextTrigger().time() : QTime::currentTime();
        if (nextDate.isValid()) {
            nextTrigger = QDateTime(nextDate, triggerTime);
        } else {
            nextTrigger = QDateTime(baseDate, triggerTime);
        }
        LOG_INFO(QString("工作日提醒，下次触发时间: %1").arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
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


QJsonArray ReminderManager::getRemindersJson() const
{
    QJsonArray array;
    for (const Reminder &reminder : m_reminders) {
        array.append(reminder.toJson());
    }
    return array;
}
