#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QVariant>
#include "core/logging/logger.h"

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager& instance();

    // 提醒相关配置
    bool isPaused() const;
    void setPaused(bool paused);
    bool isAutoStart() const;
    void setAutoStart(bool autoStart);
    bool isSoundEnabled() const;
    void setSoundEnabled(bool enabled);
    int remotePort() const;
    void setRemotePort(int port);
    QString remoteUrl() const;
    void setRemoteUrl(const QString &url);
    QJsonArray getReminders() const;
    void setReminders(const QJsonArray &reminders);

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();
    void init();
    void initDefaultConfig();
    void loadConfig();
    void loadLegacyJsonIfNeeded();
    bool openDatabase();
    void ensureTables();
    QString getConfigPath() const;
    QVariant readSetting(const QString &key, const QVariant &defaultValue) const;
    void writeSetting(const QString &key, const QVariant &value);
    QJsonArray readRemindersFromDb() const;
    void writeRemindersToDb(const QJsonArray &reminders);
    static void deduplicate(QJsonArray &reminders);

    static const QString CONFIG_FILE;
    static const QString CONFIG_DB;
    static const QString REMINDERS_KEY;
    static const QString PAUSED_KEY;
    static const QString AUTO_START_KEY;
    static const QString SOUND_ENABLED_KEY;
    static const QString PORT_KEY;
    static const QString URL_KEY;
    QSqlDatabase db;
};

#endif // CONFIGMANAGER_H 
