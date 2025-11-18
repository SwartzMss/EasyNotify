#ifndef WORKDAYCALENDAR_H
#define WORKDAYCALENDAR_H

#include <QDate>
#include <QSet>

class QJsonDocument;

class WorkdayCalendar
{
public:
    static WorkdayCalendar& instance();

    bool isHoliday(const QDate &date) const;
    bool isMakeupWorkday(const QDate &date) const;
    bool isWorkday(const QDate &date) const;
    QDate nextWorkday(const QDate &fromDate, bool includeCurrentDay = false) const;

private:
    WorkdayCalendar();

    void loadCalendar();
    bool loadFromFile(const QString &path);
    bool loadFromResource(const QString &resourcePath);
    bool parseDocument(const QJsonDocument &doc);
    static QDate parseDate(const QString &text);

    QSet<QDate> m_holidays;
    QSet<QDate> m_makeupWorkdays;
};

#endif // WORKDAYCALENDAR_H
