#ifndef TRAYICONBOUNCER_H
#define TRAYICONBOUNCER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>

class TrayIconBouncer : public QObject
{
    Q_OBJECT
public:
    explicit TrayIconBouncer(QSystemTrayIcon *icon, QObject *parent = nullptr);
    void start();
    void stop();

private slots:
    void toggleIcon();

private:
    QSystemTrayIcon *trayIcon;
    QTimer timer;
    bool showingActiveIcon;
};

#endif // TRAYICONBOUNCER_H
