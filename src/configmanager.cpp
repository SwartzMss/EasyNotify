#include "configmanager.h"

const QString ConfigManager::CONFIG_FILE = "config.json";
const QString ConfigManager::REMINDERS_KEY = "reminders";
const QString ConfigManager::PAUSED_KEY = "isPaused";

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
    saveConfig();
}

void ConfigManager::init()
{
    LOG_INFO("初始化配置管理器");
    loadConfig();
}

QString ConfigManager::getConfigPath() const
{
    return QCoreApplication::applicationDirPath() + "/" + CONFIG_FILE;
}

bool ConfigManager::isPaused() const
{
    return config[PAUSED_KEY].toBool();
}

void ConfigManager::setPaused(bool paused)
{
    config[PAUSED_KEY] = paused;
    saveConfig();
}

QJsonArray ConfigManager::getReminders() const
{
    return config[REMINDERS_KEY].toArray();
}

void ConfigManager::setReminders(const QJsonArray &reminders)
{
    config[REMINDERS_KEY] = reminders;
    saveConfig();
}

void ConfigManager::addReminder(const QJsonObject &reminder)
{
    QJsonArray reminders = config[REMINDERS_KEY].toArray();
    reminders.append(reminder);
    config[REMINDERS_KEY] = reminders;
    saveConfig();
}

void ConfigManager::updateReminder(const QString &id, const QJsonObject &reminder)
{
    QJsonArray reminders = config[REMINDERS_KEY].toArray();
    for (int i = 0; i < reminders.size(); ++i) {
        QJsonObject obj = reminders[i].toObject();
        if (obj["id"].toString() == id) {
            reminders[i] = reminder;
            break;
        }
    }
    config[REMINDERS_KEY] = reminders;
    saveConfig();
}

void ConfigManager::deleteReminder(const QString &id)
{
    QJsonArray reminders = config[REMINDERS_KEY].toArray();
    QJsonArray newReminders;
    for (const QJsonValue &value : reminders) {
        QJsonObject obj = value.toObject();
        if (obj["id"].toString() != id) {
            newReminders.append(obj);
        }
    }
    config[REMINDERS_KEY] = newReminders;
    saveConfig();
}

void ConfigManager::saveConfig()
{
    LOG_INFO("保存配置");
    QFile file(getConfigPath());
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(config);
        QByteArray jsonData = doc.toJson();
        file.write(jsonData);
        file.close();
        LOG_INFO(QString("配置保存成功，数据大小: %1 字节").arg(jsonData.size()));
    } else {
        LOG_ERROR(QString("保存配置失败: %1").arg(file.errorString()));
    }
}

void ConfigManager::loadConfig()
{
    LOG_INFO("加载配置");
    QFile file(getConfigPath());
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                config = doc.object();
                LOG_INFO("配置加载成功");
            } else {
                LOG_ERROR("配置文件格式错误");
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
}

void ConfigManager::initDefaultConfig()
{
    config = QJsonObject();
    config[PAUSED_KEY] = false;
    config[REMINDERS_KEY] = QJsonArray();
    saveConfig();
} 