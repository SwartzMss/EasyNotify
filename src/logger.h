#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT

public:
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    static Logger& instance();
    void log(LogLevel level, const QString& message);

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
    void init();
    void writeToFile(const QString& message);
    QString getLogFileName() const;
    QString formatMessage(LogLevel level, const QString& message) const;

    QFile logFile;
    QTextStream logStream;
    QMutex mutex;
    static const QString LOG_DIR;
};

// 定义日志宏
#define LOG_DEBUG(msg) Logger::instance().log(Logger::LogLevel::Debug, msg)
#define LOG_INFO(msg) Logger::instance().log(Logger::LogLevel::Info, msg)
#define LOG_WARNING(msg) Logger::instance().log(Logger::LogLevel::Warning, msg)
#define LOG_ERROR(msg) Logger::instance().log(Logger::LogLevel::Error, msg)

#endif // LOGGER_H 