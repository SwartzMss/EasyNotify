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
    enum Priority { Low, Medium, High };
    NotificationPopup(const QString &title,
                      const QString &message = {},
                      Priority priority = Priority::Medium,
                      int timeoutMs = 5000,
                      QWidget *parent = nullptr);
    
    void show();

private:
    QScopedPointer<Ui::NotificationPopup> ui;
    QPropertyAnimation *fadeIn, *fadeOut;
    QTimer *closeTimer;
    QString m_message;
    Priority m_priority;
};
