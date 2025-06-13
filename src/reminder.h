#ifndef REMINDER_H
#define REMINDER_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include "logger.h"

class Reminder {
public:
    enum class Type {
        Once,
        Daily
    };

    Reminder() = default;

    // Getters
    QString name() const { return m_name; }
    Type type() const { return m_type; }
    QDateTime nextTrigger() const { return m_nextTrigger; }
    QString id() const { return m_id; }

    // Setters
    void setName(const QString &name) { m_name = name; }
    void setType(Type type) { m_type = type; }
    void setNextTrigger(const QDateTime &trigger) { m_nextTrigger = trigger; }
    void setId(const QString &id) { m_id = id; }

    // JSON serialization
    QJsonObject toJson() const;
    static Reminder fromJson(const QJsonObject &json);

    // 相等运算符
    bool operator==(const Reminder &other) const {
        return m_name == other.m_name &&
               m_type == other.m_type &&
               m_nextTrigger == other.m_nextTrigger &&
               m_id == other.m_id;
    }

    bool operator!=(const Reminder &other) const {
        return !(*this == other);
    }

private:
    QString m_name;
    Type m_type = Type::Once;
    QDateTime m_nextTrigger;
    QString m_id;
};

#endif // REMINDER_H