#include "logger.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QThread>

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
    } 
}

void Logger::log(LogLevel level, const QString& message, const QString& file, int line)
{
    QMutexLocker locker(&mutex);
    QString formattedMessage = formatMessage(level, message, file, line);
    writeToFile(formattedMessage);
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

QString Logger::formatMessage(LogLevel level, const QString& message, const QString& file, int line) const
{
    QString levelStr;
    switch (level) {
        case LogLevel::Debug:   levelStr = "DEBUG"; break;
        case LogLevel::Info:    levelStr = "INFO"; break;
        case LogLevel::Warning: levelStr = "WARNING"; break;
        case LogLevel::Error:   levelStr = "ERROR"; break;
    }

    // 获取文件名（不包含路径）
    QFileInfo fileInfo(file);
    QString fileName = fileInfo.fileName();

    // 获取当前线程ID
    Qt::HANDLE threadId = QThread::currentThreadId();

    return QString("[%1] [%2] [Thread: %3] [%4:%5] %6")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
        .arg(levelStr)
        .arg(reinterpret_cast<quintptr>(threadId))
        .arg(fileName)
        .arg(line)
        .arg(message);
}

