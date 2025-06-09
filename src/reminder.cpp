#include "reminder.h"
#include <QJsonArray>

Reminder::Reminder(const QString &name, Type type)
    : m_name(name)
    , m_type(type)
    , m_isEnabled(true)
{
}

QJsonObject Reminder::toJson() const
{
    QJsonObject json;
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

    return json;
}

Reminder Reminder::fromJson(const QJsonObject &json)
{
    Reminder reminder;
    reminder.m_name = json["name"].toString();
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

    return reminder;
}