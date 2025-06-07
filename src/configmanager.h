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
    void addReminder(const QJsonObject &reminder);
    void updateReminder(const QString &id, const QJsonObject &reminder);
    void deleteReminder(const QString &id);

    // 保存和加载配置
    void saveConfig();
    void loadConfig();

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();
    void init();
    void initDefaultConfig();
    QString getConfigPath() const;

    QJsonObject config;
    static const QString CONFIG_FILE;
    static const QString REMINDERS_KEY;
    static const QString PAUSED_KEY;
};

#endif // CONFIGMANAGER_H 