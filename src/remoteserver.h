#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class RemoteServer : public QObject
{
    Q_OBJECT
public:
    explicit RemoteServer(quint16 port, QObject *parent = nullptr);
    bool start();
    void stop();

signals:
    void remoteMessageReceived(const QString &message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer m_server;
    QList<QTcpSocket*> m_clients;
    quint16 m_port;
};

#endif // REMOTESERVER_H
