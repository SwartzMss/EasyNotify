#include "core/config/configmanager.h"
#include "core/logging/logger.h"
#include <QSettings>
#include <QSet>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

const QString ConfigManager::CONFIG_FILE = "config.json";
const QString ConfigManager::CONFIG_DB = "config.db";
const QString ConfigManager::REMINDERS_KEY = "reminders";
const QString ConfigManager::PAUSED_KEY = "isPaused";
const QString ConfigManager::AUTO_START_KEY = "autoStart";
const QString ConfigManager::SOUND_ENABLED_KEY = "soundEnabled";
const QString ConfigManager::PORT_KEY = "remotePort";
const QString ConfigManager::URL_KEY = "remoteUrl";

ConfigManager& ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    init();
}

ConfigManager::~ConfigManager()
{
    LOG_INFO("ConfigManager 析构函数被调用");
}

void ConfigManager::init()
{
    LOG_INFO("初始化配置管理器");
    if (!openDatabase()) {
        LOG_ERROR("配置数据库打开失败，将尝试使用默认配置");
        return;
    }
    ensureTables();
    loadConfig();
}

QString ConfigManager::getConfigPath() const
{
    QString path = QCoreApplication::applicationDirPath() + "/" + CONFIG_DB;
    LOG_INFO(QString("获取配置文件路径: %1").arg(path));
    return path;
}

bool ConfigManager::isPaused() const
{
    bool paused = readSetting(PAUSED_KEY, false).toBool();
    LOG_INFO(QString("获取暂停状态: %1").arg(paused));
    return paused;
}

void ConfigManager::setPaused(bool paused)
{
    LOG_INFO(QString("设置暂停状态: %1").arg(paused));
    writeSetting(PAUSED_KEY, paused);
}

bool ConfigManager::isAutoStart() const
{
    bool autoStart = readSetting(AUTO_START_KEY, false).toBool();
    LOG_INFO(QString("获取开机启动状态: %1").arg(autoStart));
    return autoStart;
}

bool ConfigManager::isSoundEnabled() const
{
    bool enabled = readSetting(SOUND_ENABLED_KEY, true).toBool();
    LOG_INFO(QString("获取声音提醒状态: %1").arg(enabled));
    return enabled;
}

int ConfigManager::remotePort() const
{
    int port = readSetting(PORT_KEY, 12345).toInt();
    LOG_INFO(QString("获取远程端口: %1").arg(port));
    return port;
}

QString ConfigManager::remoteUrl() const
{
    QString url = readSetting(URL_KEY, QStringLiteral("tcp://localhost:12345")).toString();
    LOG_INFO(QString("获取远程地址: %1").arg(url));
    return url;
}

void ConfigManager::setAutoStart(bool autoStart)
{
    LOG_INFO(QString("设置开机启动: %1").arg(autoStart));
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (autoStart) {
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        settings.setValue(QCoreApplication::applicationName(), appPath);
    } else {
        settings.remove(QCoreApplication::applicationName());
    }
#endif
    writeSetting(AUTO_START_KEY, autoStart);
}

void ConfigManager::setSoundEnabled(bool enabled)
{
    LOG_INFO(QString("设置声音提醒: %1").arg(enabled));
    writeSetting(SOUND_ENABLED_KEY, enabled);
}

void ConfigManager::setRemotePort(int port)
{
    LOG_INFO(QString("设置远程端口: %1").arg(port));
    writeSetting(PORT_KEY, port);
}

void ConfigManager::setRemoteUrl(const QString &url)
{
    LOG_INFO(QString("设置远程地址: %1").arg(url));
    writeSetting(URL_KEY, url);
}

QJsonArray ConfigManager::getReminders() const
{
    QJsonArray reminders = readRemindersFromDb();
    LOG_INFO(QString("获取提醒列表，共 %1 个提醒").arg(reminders.size()));
    return reminders;
}

void ConfigManager::setReminders(const QJsonArray &reminders)
{
    QJsonArray unique = reminders;
    deduplicate(unique);
    LOG_INFO(QString("设置提醒列表，共 %1 个提醒").arg(unique.size()));
    writeRemindersToDb(unique);
}

void ConfigManager::loadConfig()
{
    // 如果数据库没有任何设置，尝试从旧版 JSON 导入，否则填充默认值
    QSqlQuery query(db);
    query.exec(QStringLiteral("SELECT COUNT(*) FROM settings"));
    int count = 0;
    if (query.next()) {
        count = query.value(0).toInt();
    }
    if (count == 0) {
        LOG_INFO("设置表为空，尝试从旧版 JSON 迁移");
        loadLegacyJsonIfNeeded();
        // 如果仍为空则写入默认配置
        query.exec(QStringLiteral("SELECT COUNT(*) FROM settings"));
        if (query.next() && query.value(0).toInt() == 0) {
            initDefaultConfig();
        }
    }
}

void ConfigManager::initDefaultConfig()
{
    LOG_INFO("初始化默认配置");
    writeSetting(PAUSED_KEY, false);
    writeSetting(AUTO_START_KEY, false);
    writeSetting(SOUND_ENABLED_KEY, true);
    writeSetting(PORT_KEY, 12345);
    writeSetting(URL_KEY, QStringLiteral("tcp://localhost:12345"));
    writeRemindersToDb(QJsonArray());
}

void ConfigManager::deduplicate(QJsonArray &reminders)
{
    QSet<QString> ids;
    QJsonArray unique;
    for (const QJsonValue &val : reminders) {
        if (!val.isObject())
            continue;
        QJsonObject obj = val.toObject();
        QString id = obj.value("id").toString();
        if (ids.contains(id)) {
            LOG_WARNING(QString("发现重复的提醒 ID: %1，已移除").arg(id));
            continue;
        }
        ids.insert(id);
        unique.append(obj);
    }
    reminders = unique;
}

bool ConfigManager::openDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "config_connection");
    db.setDatabaseName(getConfigPath());
    if (!db.open()) {
        LOG_ERROR(QString("打开配置数据库失败: %1").arg(db.lastError().text()));
        return false;
    }
    return true;
}

void ConfigManager::ensureTables()
{
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS settings ("
                                   "key TEXT PRIMARY KEY,"
                                   "value TEXT)"))) {
        LOG_ERROR(QString("创建 settings 表失败: %1").arg(query.lastError().text()));
    }
    if (!query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS reminders ("
                                   "id TEXT PRIMARY KEY,"
                                   "name TEXT,"
                                   "type INTEGER,"
                                   "priority INTEGER,"
                                   "next_trigger TEXT,"
                                   "completed INTEGER)"))) {
        LOG_ERROR(QString("创建 reminders 表失败: %1").arg(query.lastError().text()));
    }
}

QVariant ConfigManager::readSetting(const QString &key, const QVariant &defaultValue) const
{
    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT value FROM settings WHERE key = ?"));
    query.addBindValue(key);
    if (query.exec() && query.next()) {
        return query.value(0);
    }
    return defaultValue;
}

void ConfigManager::writeSetting(const QString &key, const QVariant &value)
{
    QSqlQuery query(db);
    query.prepare(QStringLiteral("REPLACE INTO settings (key, value) VALUES (?, ?)"));
    query.addBindValue(key);
    query.addBindValue(value);
    if (!query.exec()) {
        LOG_ERROR(QString("写入设置失败 [%1]: %2").arg(key, query.lastError().text()));
    }
}

QJsonArray ConfigManager::readRemindersFromDb() const
{
    QJsonArray array;
    QSqlQuery query(db);
    if (query.exec(QStringLiteral("SELECT id, name, type, priority, next_trigger, completed FROM reminders"))) {
        while (query.next()) {
            QJsonObject obj;
            obj["id"] = query.value(0).toString();
            obj["name"] = query.value(1).toString();
            obj["type"] = query.value(2).toInt();
            obj["priority"] = query.value(3).toInt();
            obj["nextTrigger"] = query.value(4).toString();
            obj["completed"] = query.value(5).toBool();
            array.append(obj);
        }
    } else {
        LOG_ERROR(QString("读取提醒失败: %1").arg(query.lastError().text()));
    }
    return array;
}

void ConfigManager::writeRemindersToDb(const QJsonArray &reminders)
{
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("DELETE FROM reminders"))) {
        LOG_ERROR(QString("清空提醒表失败: %1").arg(query.lastError().text()));
        return;
    }
    query.prepare(QStringLiteral("INSERT INTO reminders (id, name, type, priority, next_trigger, completed) "
                                 "VALUES (?, ?, ?, ?, ?, ?)"));
    for (const QJsonValue &val : reminders) {
        if (!val.isObject())
            continue;
        QJsonObject obj = val.toObject();
        query.addBindValue(obj.value("id").toString());
        query.addBindValue(obj.value("name").toString());
        query.addBindValue(obj.value("type").toInt());
        query.addBindValue(obj.value("priority").toInt());
        query.addBindValue(obj.value("nextTrigger").toString());
        query.addBindValue(obj.value("completed").toBool() ? 1 : 0);
        if (!query.exec()) {
            LOG_ERROR(QString("写入提醒失败 (ID=%1): %2")
                      .arg(obj.value("id").toString(),
                           query.lastError().text()));
        }
        query.clear();
        query.prepare(QStringLiteral("INSERT INTO reminders (id, name, type, priority, next_trigger, completed) "
                                     "VALUES (?, ?, ?, ?, ?, ?)"));
    }
}

void ConfigManager::loadLegacyJsonIfNeeded()
{
    QString legacyPath = QCoreApplication::applicationDirPath() + "/" + CONFIG_FILE;
    QFile file(legacyPath);
    if (!file.exists()) {
        initDefaultConfig();
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("无法读取旧版配置文件: %1").arg(file.errorString()));
        initDefaultConfig();
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    file.close();
    if (!doc.isObject()) {
        LOG_ERROR("旧版配置格式错误，使用默认配置");
        initDefaultConfig();
        return;
    }
    QJsonObject legacy = doc.object();
    writeSetting(PAUSED_KEY, legacy.value(PAUSED_KEY).toBool(false));
    writeSetting(AUTO_START_KEY, legacy.value(AUTO_START_KEY).toBool(false));
    writeSetting(SOUND_ENABLED_KEY, legacy.value(SOUND_ENABLED_KEY).toBool(true));
    writeSetting(PORT_KEY, legacy.value(PORT_KEY).toInt(12345));
    writeSetting(URL_KEY, legacy.value(URL_KEY).toString(QStringLiteral("tcp://localhost:12345")));
    QJsonArray reminders = legacy.value(REMINDERS_KEY).toArray();
    deduplicate(reminders);
    writeRemindersToDb(reminders);
}
