#include "configmanager.h"
#include "logger.h"
#include <QSettings>
#include <QSet>

const QString ConfigManager::CONFIG_FILE = "config.json";
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
    LOG_INFO("ConfigManager 析构函数被调用，准备保存配置");
    saveConfig();
}

void ConfigManager::init()
{
    LOG_INFO("初始化配置管理器");
    loadConfig();
}

QString ConfigManager::getConfigPath() const
{
    QString path = QCoreApplication::applicationDirPath() + "/" + CONFIG_FILE;
    LOG_INFO(QString("获取配置文件路径: %1").arg(path));
    return path;
}

bool ConfigManager::isPaused() const
{
    bool paused = config[PAUSED_KEY].toBool();
    LOG_INFO(QString("获取暂停状态: %1").arg(paused));
    return paused;
}

void ConfigManager::setPaused(bool paused)
{
    LOG_INFO(QString("设置暂停状态: %1").arg(paused));
    config[PAUSED_KEY] = paused;
    saveConfig();
}

bool ConfigManager::isAutoStart() const
{
    bool autoStart = config[AUTO_START_KEY].toBool();
    LOG_INFO(QString("获取开机启动状态: %1").arg(autoStart));
    return autoStart;
}

bool ConfigManager::isSoundEnabled() const
{
    bool enabled = config[SOUND_ENABLED_KEY].toBool(true);
    LOG_INFO(QString("获取声音提醒状态: %1").arg(enabled));
    return enabled;
}

int ConfigManager::remotePort() const
{
    int port = config[PORT_KEY].toInt(12345);
    LOG_INFO(QString("获取远程端口: %1").arg(port));
    return port;
}

QString ConfigManager::remoteUrl() const
{
    QString url = config[URL_KEY].toString("tcp://localhost:12345");
    LOG_INFO(QString("获取远程地址: %1").arg(url));
    return url;
}

void ConfigManager::setAutoStart(bool autoStart)
{
    LOG_INFO(QString("设置开机启动: %1").arg(autoStart));
    config[AUTO_START_KEY] = autoStart;
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (autoStart) {
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        settings.setValue(QCoreApplication::applicationName(), appPath);
    } else {
        settings.remove(QCoreApplication::applicationName());
    }
#endif
    saveConfig();
}

void ConfigManager::setSoundEnabled(bool enabled)
{
    LOG_INFO(QString("设置声音提醒: %1").arg(enabled));
    config[SOUND_ENABLED_KEY] = enabled;
    saveConfig();
}

void ConfigManager::setRemotePort(int port)
{
    LOG_INFO(QString("设置远程端口: %1").arg(port));
    config[PORT_KEY] = port;
    saveConfig();
}

void ConfigManager::setRemoteUrl(const QString &url)
{
    LOG_INFO(QString("设置远程地址: %1").arg(url));
    config[URL_KEY] = url;
    saveConfig();
}

QJsonArray ConfigManager::getReminders() const
{
    QJsonArray reminders = config[REMINDERS_KEY].toArray();
    deduplicate(reminders);
    LOG_INFO(QString("获取提醒列表，共 %1 个提醒").arg(reminders.size()));
    return reminders;
}

void ConfigManager::setReminders(const QJsonArray &reminders)
{
    QJsonArray unique = reminders;
    deduplicate(unique);
    LOG_INFO(QString("设置提醒列表，共 %1 个提醒").arg(unique.size()));
    config[REMINDERS_KEY] = unique;
    saveConfig();
}

void ConfigManager::saveConfig()
{
    QString path = getConfigPath();
    LOG_INFO(QString("保存配置到文件: %1").arg(path));
    
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(config);
        QByteArray jsonData = doc.toJson();
        qint64 bytesWritten = file.write(jsonData);
        file.close();
        
        if (bytesWritten == jsonData.size()) {
            LOG_INFO(QString("配置保存成功，数据大小: %1 字节").arg(jsonData.size()));
        } else {
            LOG_ERROR(QString("配置保存不完整，预期写入 %1 字节，实际写入 %2 字节")
                     .arg(jsonData.size())
                     .arg(bytesWritten));
        }
    } else {
        LOG_ERROR(QString("保存配置失败: %1").arg(file.errorString()));
    }
}

void ConfigManager::loadConfig()
{
    QString path = getConfigPath();
    LOG_INFO(QString("从文件加载配置: %1").arg(path));
    
    QFile file(path);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                config = doc.object();
                LOG_INFO(QString("配置加载成功，数据大小: %1 字节").arg(data.size()));
                if (!config.contains(SOUND_ENABLED_KEY))
                    config[SOUND_ENABLED_KEY] = true;
                if (!config.contains(PORT_KEY))
                    config[PORT_KEY] = 12345;
                if (!config.contains(URL_KEY))
                    config[URL_KEY] = "tcp://localhost:12345";
            } else {
                LOG_ERROR(QString("配置文件格式错误，数据大小: %1 字节").arg(data.size()));
                initDefaultConfig();
            }
            file.close();
        } else {
            LOG_ERROR(QString("打开配置文件失败: %1").arg(file.errorString()));
            initDefaultConfig();
        }
    } else {
        LOG_INFO("配置文件不存在，创建默认配置");
        initDefaultConfig();
    }

    if (config.contains(REMINDERS_KEY) && config[REMINDERS_KEY].isArray()) {
        QJsonArray reminders = config[REMINDERS_KEY].toArray();
        deduplicate(reminders);
        config[REMINDERS_KEY] = reminders;
        saveConfig();
    }
}

void ConfigManager::initDefaultConfig()
{
    LOG_INFO("初始化默认配置");
    config = QJsonObject();
    config[PAUSED_KEY] = false;
    config[AUTO_START_KEY] = false;
    config[SOUND_ENABLED_KEY] = true;
    config[PORT_KEY] = 12345;
    config[URL_KEY] = "tcp://localhost:12345";
    config[REMINDERS_KEY] = QJsonArray();
    saveConfig();
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
