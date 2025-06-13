#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include "logger.h"

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager& instance();

    // 提醒相关配置
    bool isPaused() const;
    void setPaused(bool paused);
    QJsonArray getReminders() const;
    void setReminders(const QJsonArray &reminders);

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();
    void init();
    void initDefaultConfig();
    void saveConfig();
    void loadConfig();
    QString getConfigPath() const;

    QJsonObject config;
    static const QString CONFIG_FILE;
    static const QString REMINDERS_KEY;
    static const QString PAUSED_KEY;
};

#endif // CONFIGMANAGER_H 