#include "core/calendar/workdaycalendar.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include "core/logging/logger.h"

namespace {
constexpr const char *DEFAULT_RESOURCE_PATH = ":/data/workdays.json";
QString localCalendarPath()
{
    return QCoreApplication::applicationDirPath() + "/workdays.json";
}
}

WorkdayCalendar &WorkdayCalendar::instance()
{
    static WorkdayCalendar calendar;
    return calendar;
}

WorkdayCalendar::WorkdayCalendar()
{
    loadCalendar();
}

bool WorkdayCalendar::isHoliday(const QDate &date) const
{
    return m_holidays.contains(date);
}

bool WorkdayCalendar::isMakeupWorkday(const QDate &date) const
{
    return m_makeupWorkdays.contains(date);
}

bool WorkdayCalendar::isWorkday(const QDate &date) const
{
    if (!date.isValid()) {
        return false;
    }

    if (isMakeupWorkday(date)) {
        return true;
    }

    if (isHoliday(date)) {
        return false;
    }

    const int dayOfWeek = date.dayOfWeek();
    return dayOfWeek >= Qt::Monday && dayOfWeek <= Qt::Friday;
}

QDate WorkdayCalendar::nextWorkday(const QDate &fromDate, bool includeCurrentDay) const
{
    if (!fromDate.isValid()) {
        return QDate();
    }

    QDate date = includeCurrentDay ? fromDate : fromDate.addDays(1);
    for (int i = 0; i < 3660; ++i) { // Up to ~10 years safeguard
        if (isWorkday(date)) {
            return date;
        }
        date = date.addDays(1);
    }

    LOG_WARNING("无法在合理范围内计算下一个工作日");
    return QDate();
}

void WorkdayCalendar::loadCalendar()
{
    const QString localPath = localCalendarPath();
    if (loadFromFile(localPath)) {
        LOG_INFO(QString("已加载自定义工作日配置: %1").arg(localPath));
        return;
    }

    if (loadFromResource(DEFAULT_RESOURCE_PATH)) {
        LOG_INFO("已加载内置工作日配置");
        return;
    }

    LOG_ERROR("无法加载工作日配置，将使用空配置");
}

bool WorkdayCalendar::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.exists()) {
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        LOG_WARNING(QString("打开工作日配置失败: %1").arg(file.errorString()));
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!parseDocument(doc)) {
        LOG_WARNING(QString("解析工作日配置失败: %1").arg(path));
        return false;
    }

    LOG_INFO(QString("从文件加载工作日配置成功: %1").arg(path));
    return true;
}

bool WorkdayCalendar::loadFromResource(const QString &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_WARNING(QString("打开内置工作日资源失败: %1").arg(resourcePath));
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!parseDocument(doc)) {
        LOG_WARNING("解析内置工作日配置失败");
        return false;
    }

    return true;
}

bool WorkdayCalendar::parseDocument(const QJsonDocument &doc)
{
    if (!doc.isObject()) {
        return false;
    }

    const QJsonObject obj = doc.object();
    QSet<QDate> holidaysSet;
    QSet<QDate> makeupSet;

    auto ingestArrays = [&](const QJsonArray &holidays, const QJsonArray &makeups) {
        for (const QJsonValue &value : holidays) {
            const QDate date = parseDate(value.toString());
            if (date.isValid()) {
                holidaysSet.insert(date);
            }
        }
        for (const QJsonValue &value : makeups) {
            const QDate date = parseDate(value.toString());
            if (date.isValid()) {
                makeupSet.insert(date);
            }
        }
    };

    // 兼容旧格式（顶层两个数组）
    if (obj.contains("holidays") || obj.contains("makeupDays")) {
        ingestArrays(obj.value("holidays").toArray(), obj.value("makeupDays").toArray());
    }

    // 支持按年份分组的结构（years 或直接以年份为键）
    const QRegularExpression yearRe(QStringLiteral("^\\d{4}$"));
    QJsonObject yearsObj = obj.value("years").toObject();
    if (yearsObj.isEmpty()) {
        yearsObj = obj; // 直接遍历顶层年份键
    }
    for (auto it = yearsObj.constBegin(); it != yearsObj.constEnd(); ++it) {
        if (!yearRe.match(it.key()).hasMatch() || !it.value().isObject()) {
            continue;
        }
        const QJsonObject yearBlock = it.value().toObject();
        ingestArrays(yearBlock.value("holidays").toArray(), yearBlock.value("makeupDays").toArray());
    }

    m_holidays = holidaysSet;
    m_makeupWorkdays = makeupSet;
    LOG_INFO(QString("工作日配置：节假日 %1 天，调休 %2 天")
                 .arg(m_holidays.size())
                 .arg(m_makeupWorkdays.size()));
    return true;
}

QDate WorkdayCalendar::parseDate(const QString &text)
{
    const QDate date = QDate::fromString(text, Qt::ISODate);
    if (!date.isValid() && !text.isEmpty()) {
        LOG_WARNING(QString("无法解析日期: %1").arg(text));
    }
    return date;
}
