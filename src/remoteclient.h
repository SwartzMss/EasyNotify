#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QObject>
#include <QTcpSocket>

class RemoteClient : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClient(const QUrl &url, QObject *parent = nullptr);
    void start();
    void stop();

signals:
    void remoteMessageReceived(const QString &message);

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpSocket m_socket;
    QUrl m_url;
};

#endif // REMOTECLIENT_H
