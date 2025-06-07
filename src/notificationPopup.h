// NotificationPopup.h
#pragma once
#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

class NotificationPopup : public QWidget {
    Q_OBJECT
public:
    NotificationPopup(const QString &title,
                      const QString &message,
                      const QIcon &icon = {},
                      int timeoutMs = 5000,
                      QWidget *parent = nullptr);

    // 在系统托盘图标的位置附近弹出
    void showNearTray(const QRect &trayGeom);
    
    // 在指定位置显示
    void showAt(const QPoint &pos);

private:
    QLabel *iconLabel, *titleLabel, *msgLabel;
    QPropertyAnimation *fadeIn, *fadeOut;
    QTimer *closeTimer;
};
