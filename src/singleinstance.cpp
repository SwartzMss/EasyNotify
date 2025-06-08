#include "singleinstance.h"
#include "logger.h"
#include <QDataStream>
#include <QTimer>

const QString SingleInstance::SHARED_MEMORY_KEY = "EasyNotify_SharedMemory";
const QString SingleInstance::LOCAL_SERVER_NAME = "EasyNotify_LocalServer";

SingleInstance& SingleInstance::instance()
{
    static SingleInstance instance;
    return instance;
}

SingleInstance::SingleInstance(QObject *parent)
    : QObject(parent)
    , sharedMemory(SHARED_MEMORY_KEY)
    , isRunning_(false)
{
    init();
}

SingleInstance::~SingleInstance()
{
    if (sharedMemory.isAttached()) {
        sharedMemory.detach();
    }
}

void SingleInstance::init()
{
    // 尝试创建共享内存
    if (!createSharedMemory()) {
        LOG_INFO("程序已经在运行");
        isRunning_ = true;
        return;
    }

    // 创建本地服务器
    if (!createLocalServer()) {
        LOG_ERROR("无法创建本地服务器");
        return;
    }

    LOG_INFO("程序首次启动");
    isRunning_ = false;
}

bool SingleInstance::createSharedMemory()
{
    // 尝试创建共享内存
    if (!sharedMemory.create(1)) {
        if (sharedMemory.error() == QSharedMemory::AlreadyExists) {
            // 尝试附加到已存在的共享内存
            if (!sharedMemory.attach()) {
                LOG_ERROR("无法附加到共享内存");
                return false;
            }
            return false;
        }
        LOG_ERROR("无法创建共享内存");
        return false;
    }
    return true;
}

bool SingleInstance::createLocalServer()
{
    // 删除可能存在的旧服务器
    QLocalServer::removeServer(LOCAL_SERVER_NAME);

    // 创建新的本地服务器
    if (!localServer.listen(LOCAL_SERVER_NAME)) {
        LOG_ERROR("无法创建本地服务器");
        return false;
    }

    // 连接新消息信号
    connect(&localServer, &QLocalServer::newConnection, this, [this]() {
        QLocalSocket *socket = localServer.nextPendingConnection();
        if (socket) {
            if (socket->waitForReadyRead(TIMEOUT_MS)) {
                QByteArray data = socket->readAll();
                QString message = QString::fromUtf8(data);
                handleMessage(message);
            }
            socket->disconnectFromServer();
            delete socket;
        }
    });

    return true;
}

bool SingleInstance::isRunning()
{
    return isRunning_;
}

bool SingleInstance::sendMessage(const QString &message)
{
    if (!isRunning_) {
        return false;
    }

    QLocalSocket socket;
    socket.connectToServer(LOCAL_SERVER_NAME);
    if (!socket.waitForConnected(TIMEOUT_MS)) {
        LOG_ERROR("无法连接到本地服务器");
        return false;
    }

    socket.write(message.toUtf8());
    if (!socket.waitForBytesWritten(TIMEOUT_MS)) {
        LOG_ERROR("无法发送消息");
        return false;
    }

    socket.disconnectFromServer();
    return true;
}

void SingleInstance::handleMessage(const QString &message)
{
    LOG_INFO(QString("收到消息: %1").arg(message));
    emit messageReceived(message);
} 