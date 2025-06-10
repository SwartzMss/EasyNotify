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
    void log(LogLevel level, const QString& message, const QString& file, int line);

    // 便捷的日志方法
    static void debug(const QString& message, const QString& file, int line) {
        instance().log(LogLevel::Debug, message, file, line);
    }
    static void info(const QString& message, const QString& file, int line) {
        instance().log(LogLevel::Info, message, file, line);
    }
    static void warning(const QString& message, const QString& file, int line) {
        instance().log(LogLevel::Warning, message, file, line);
    }
    static void error(const QString& message, const QString& file, int line) {
        instance().log(LogLevel::Error, message, file, line);
    }

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
    void init();
    void writeToFile(const QString& message);
    QString getLogFileName() const;
    QString formatMessage(LogLevel level, const QString& message, const QString& file, int line) const;

    QFile logFile;
    QTextStream logStream;
    QMutex mutex;
    static const QString LOG_DIR;
};

// 定义日志宏
#define LOG_DEBUG(msg) Logger::debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::error(msg, __FILE__, __LINE__)

#endif // LOGGER_H 