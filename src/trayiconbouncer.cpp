#include "trayiconbouncer.h"

TrayIconBouncer::TrayIconBouncer(QSystemTrayIcon *icon, QObject *parent)
    : QObject(parent), trayIcon(icon), showingActiveIcon(false)
{
    timer.setInterval(300);
    connect(&timer, &QTimer::timeout, this, &TrayIconBouncer::toggleIcon);
}

void TrayIconBouncer::start()
{
    if (!timer.isActive()) {
        showingActiveIcon = false;
        timer.start();
    }
}

void TrayIconBouncer::stop()
{
    if (timer.isActive()) {
        timer.stop();
        trayIcon->setIcon(QIcon(":/img/tray_icon.png"));
    }
}

void TrayIconBouncer::toggleIcon()
{
    showingActiveIcon = !showingActiveIcon;
    QString res = showingActiveIcon ? ":/img/tray_icon_active.png" : ":/img/tray_icon.png";
    trayIcon->setIcon(QIcon(res));
}
