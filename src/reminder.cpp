#include "reminder.h"
#include <QJsonArray>
#include <QUuid>
#include "logger.h"

Reminder::Reminder(const QString &name, Type type)
    : m_name(name)
    , m_type(type)
    , m_isEnabled(true)
{
    LOG_INFO(QString("创建新提醒: 名称='%1', ID='%2'").arg(name).arg(m_id));
}

QJsonObject Reminder::toJson() const
{
    LOG_INFO(QString("序列化提醒: ID='%1', 名称='%2'").arg(m_id).arg(m_name));
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["type"] = static_cast<int>(m_type);
    json["isEnabled"] = m_isEnabled;
    json["nextTrigger"] = m_nextTrigger.toString(Qt::ISODate);
    
    QJsonArray weekDaysArray;
    for (int day : m_weekDays) {
        weekDaysArray.append(day);
    }
    json["weekDays"] = weekDaysArray;

    QJsonArray monthDaysArray;
    for (int day : m_monthDays) {
        monthDaysArray.append(day);
    }
    json["monthDays"] = monthDaysArray;

    LOG_INFO(QString("提醒序列化完成: ID='%1'").arg(m_id));
    return json;
}

Reminder Reminder::fromJson(const QJsonObject &json)
{
    QString id = json["id"].toString();
    QString name = json["name"].toString();
    LOG_INFO(QString("反序列化提醒: ID='%1', 名称='%2'").arg(id).arg(name));
    
    Reminder reminder;
    reminder.m_id = id;
    reminder.m_name = name;
    reminder.m_type = static_cast<Type>(json["type"].toInt());
    reminder.m_isEnabled = json["isEnabled"].toBool();
    reminder.m_nextTrigger = QDateTime::fromString(json["nextTrigger"].toString(), Qt::ISODate);

    QJsonArray weekDaysArray = json["weekDays"].toArray();
    for (const QJsonValue &value : weekDaysArray) {
        reminder.m_weekDays.insert(value.toInt());
    }

    QJsonArray monthDaysArray = json["monthDays"].toArray();
    for (const QJsonValue &value : monthDaysArray) {
        reminder.m_monthDays.insert(value.toInt());
    }

    LOG_INFO(QString("提醒反序列化完成: ID='%1', 类型=%2, 启用状态=%3")
             .arg(id)
             .arg(static_cast<int>(reminder.m_type))
             .arg(reminder.m_isEnabled));
    return reminder;
}
