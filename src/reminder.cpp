#include "reminder.h"
#include <QUuid>
#include "logger.h"


QJsonObject Reminder::toJson() const
{
    LOG_INFO(QString("序列化提醒: ID='%1', 名称='%2'").arg(m_id).arg(m_name));
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["type"] = static_cast<int>(m_type);
    json["nextTrigger"] = m_nextTrigger.toString(Qt::ISODate);
    json["completed"] = m_completed;

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
    reminder.m_nextTrigger = QDateTime::fromString(json["nextTrigger"].toString(), Qt::ISODate);
    reminder.m_completed = json.contains("completed") ? json["completed"].toBool() : false;

    LOG_INFO(QString("提醒反序列化完成: ID='%1', 类型=%2")
             .arg(id)
             .arg(static_cast<int>(reminder.m_type)));
    return reminder;
}
