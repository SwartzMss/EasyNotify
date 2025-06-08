#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#include <QObject>
#include <QSharedMemory>
#include <QLocalServer>
#include <QLocalSocket>
#include <QApplication>

class SingleInstance : public QObject
{
    Q_OBJECT

public:
    static SingleInstance& instance();
    bool isRunning();
    bool sendMessage(const QString &message);

signals:
    void messageReceived(const QString &message);

private:
    explicit SingleInstance(QObject *parent = nullptr);
    ~SingleInstance();
    void init();
    bool createSharedMemory();
    bool createLocalServer();
    void handleMessage(const QString &message);

    static const QString SHARED_MEMORY_KEY;
    static const QString LOCAL_SERVER_NAME;
    static const int TIMEOUT_MS = 1000;

    QSharedMemory sharedMemory;
    QLocalServer localServer;
    bool isRunning_;
};

#endif // SINGLEINSTANCE_H 