#ifndef REMINDER_H
#define REMINDER_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QSet>
#include "logger.h"

class Reminder {
public:
    enum class Type {
        Once,
        Daily
    };

    Reminder() = default;
    Reminder(const QString &name, Type type = Type::Once);

    // Getters
    QString name() const { return m_name; }
    Type type() const { return m_type; }
    bool isEnabled() const { return m_isEnabled; }
    QDateTime nextTrigger() const { return m_nextTrigger; }
    QSet<int> weekDays() const { return m_weekDays; }
    QSet<int> monthDays() const { return m_monthDays; }
    QString id() const { return m_id; }
    QString title() const { return m_name; }

    // Setters
    void setName(const QString &name) { m_name = name; }
    void setType(Type type) { m_type = type; }
    void setEnabled(bool enabled) { m_isEnabled = enabled; }
    void setNextTrigger(const QDateTime &trigger) { m_nextTrigger = trigger; }
    void setWeekDays(const QSet<int> &days) { m_weekDays = days; }
    void setMonthDays(const QSet<int> &days) { m_monthDays = days; }
    void setId(const QString &id) { m_id = id; }

    // JSON serialization
    QJsonObject toJson() const;
    static Reminder fromJson(const QJsonObject &json);

    // 相等运算符
    bool operator==(const Reminder &other) const {
        return m_name == other.m_name &&
               m_type == other.m_type &&
               m_isEnabled == other.m_isEnabled &&
               m_nextTrigger == other.m_nextTrigger &&
               m_id == other.m_id;
    }

    bool operator!=(const Reminder &other) const {
        return !(*this == other);
    }

private:
    QString m_name;
    Type m_type = Type::Once;
    bool m_isEnabled = true;
    QDateTime m_nextTrigger;
    QSet<int> m_weekDays;
    QSet<int> m_monthDays;
    QString m_id;
};

#endif // REMINDER_H