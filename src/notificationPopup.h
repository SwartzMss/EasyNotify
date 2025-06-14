// NotificationPopup.h
#pragma once
#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QScopedPointer>
#include "ui_notificationPopup.h"

class NotificationPopup : public QWidget {
    Q_OBJECT
public:
    enum class Priority {
        Information,
        Warning,
        Critical
    };
    NotificationPopup(const QString &title,
                      Priority priority = Priority::Information,
                      int timeoutMs = 5000,
                      QWidget *parent = nullptr);
    
    void show();

private:
    QScopedPointer<Ui::NotificationPopup> ui;
    QPropertyAnimation *fadeIn, *fadeOut;
    QTimer *closeTimer;
};
