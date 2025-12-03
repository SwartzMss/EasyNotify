#include "core/network/remoteclient.h"
#include "core/logging/logger.h"

RemoteClient::RemoteClient(const QUrl &url, QObject *parent)
    : QObject(parent), m_url(url)
{
    connect(&m_socket, &QTcpSocket::connected,
            this, &RemoteClient::onConnected);
    connect(&m_socket, &QTcpSocket::readyRead,
            this, &RemoteClient::onReadyRead);
    connect(&m_socket, &QTcpSocket::disconnected,
            this, &RemoteClient::onDisconnected);
}

void RemoteClient::start()
{
    LOG_INFO(QString("RemoteClient connecting to %1").arg(m_url.toString()));
    m_socket.connectToHost(m_url.host(), m_url.port(12345));
}

void RemoteClient::stop()
{
    m_socket.disconnectFromHost();
}

void RemoteClient::onConnected()
{
    LOG_INFO("RemoteClient connected");
}

void RemoteClient::onReadyRead()
{
    QByteArray data = m_socket.readAll();
    QString msg = QString::fromUtf8(data).trimmed();
    if (!msg.isEmpty())
        emit remoteMessageReceived(msg);
}

void RemoteClient::onDisconnected()
{
    LOG_INFO("RemoteClient disconnected");
}

