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
    
    void show();

private:
    QLabel *iconLabel, *titleLabel, *msgLabel;
    QPropertyAnimation *fadeIn, *fadeOut;
    QTimer *closeTimer;
};
