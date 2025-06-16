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
#include "reminder.h"
#include <QList>
#include <QPointer>
#include <QCloseEvent>

class NotificationPopup : public QWidget {
    Q_OBJECT
public:
    using Priority = Reminder::Priority;
    NotificationPopup(const QString &title,
                      const QString &message = {},
                      Priority priority = Priority::Medium,
                      QWidget *parent = nullptr);
    
    void show();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    static QList<QPointer<NotificationPopup>> s_popups;
    void repositionPopups();
    QScopedPointer<Ui::NotificationPopup> ui;
    QPropertyAnimation *fadeIn, *fadeOut;
    QString m_message;
    Priority m_priority;
};
