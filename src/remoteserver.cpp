#include "remoteserver.h"
#include "logger.h"

RemoteServer::RemoteServer(quint16 port, QObject *parent)
    : QObject(parent),
      m_port(port)
{
    connect(&m_server, &QTcpServer::newConnection,
            this, &RemoteServer::onNewConnection);
}

bool RemoteServer::start()
{
    if (m_server.listen(QHostAddress::Any, m_port)) {
        LOG_INFO(QString("RemoteServer listening on port %1").arg(m_port));
        return true;
    }
    LOG_ERROR(QString("RemoteServer failed to listen: %1")
              .arg(m_server.errorString()));
    return false;
}

void RemoteServer::stop()
{
    for (QTcpSocket *client : m_clients) {
        client->disconnect(this);
        client->close();
        client->deleteLater();
    }
    m_clients.clear();
    m_server.close();
}

void RemoteServer::onNewConnection()
{
    while (m_server.hasPendingConnections()) {
        QTcpSocket *client = m_server.nextPendingConnection();
        if (!client)
            continue;
        m_clients.append(client);
        connect(client, &QTcpSocket::readyRead,
                this, &RemoteServer::onReadyRead);
        connect(client, &QTcpSocket::disconnected,
                this, &RemoteServer::onClientDisconnected);
    }
}

void RemoteServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client)
        return;
    QByteArray data = client->readAll();
    QString msg = QString::fromUtf8(data).trimmed();
    if (!msg.isEmpty()) {
        emit remoteMessageReceived(msg);
    }
}

void RemoteServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client)
        return;
    m_clients.removeAll(client);
    client->deleteLater();
}
