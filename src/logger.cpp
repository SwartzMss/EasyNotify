#include "logger.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

const QString Logger::LOG_DIR = "logs";

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
{
    init();
}

Logger::~Logger()
{
    if (logFile.isOpen()) {
        logStream.flush();
        logFile.close();
    }
}

void Logger::init()
{
    // 创建日志目录
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists(LOG_DIR)) {
        dir.mkdir(LOG_DIR);
    }

    // 打开日志文件
    QString logPath = dir.filePath(LOG_DIR + "/" + getLogFileName());
    logFile.setFileName(logPath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        logStream.setDevice(&logFile);
    } else {
        qWarning() << "无法打开日志文件:" << logPath;
    }
}

void Logger::log(LogLevel level, const QString& message)
{
    QMutexLocker locker(&mutex);
    QString formattedMessage = formatMessage(level, message);
    writeToFile(formattedMessage);
    qDebug().noquote() << formattedMessage;
}

void Logger::writeToFile(const QString& message)
{
    if (logFile.isOpen()) {
        logStream << message << Qt::endl;
        logStream.flush();
    }
}

QString Logger::getLogFileName() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log";
}

QString Logger::formatMessage(LogLevel level, const QString& message) const
{
    QString levelStr;
    switch (level) {
        case LogLevel::Debug:   levelStr = "DEBUG"; break;
        case LogLevel::Info:    levelStr = "INFO"; break;
        case LogLevel::Warning: levelStr = "WARNING"; break;
        case LogLevel::Error:   levelStr = "ERROR"; break;
    }

    return QString("[%1] [%2] %3")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
        .arg(levelStr)
        .arg(message);
}

void Logger::setLogLevel(const QString& level)
{
    QMutexLocker locker(&mutex);
    // 在 Qt6 中，我们不再需要设置编码
}

QString Logger::formatLogMessage(const QString& message, const QString& level) const
{
    return QString("[%1] [%2] %3")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
        .arg(level)
        .arg(message);
} 