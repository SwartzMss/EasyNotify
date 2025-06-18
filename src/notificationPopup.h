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
#include <QSoundEffect>

class NotificationPopup : public QWidget {
    Q_OBJECT
public:
    using Priority = Reminder::Priority;
    NotificationPopup(const QString &title,
                      Priority priority = Priority::Medium,
                      bool soundEnabled = true,
                      QWidget *parent = nullptr);
    
    void show();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void startFadeOut();

private:
    static QList<QPointer<NotificationPopup>> s_popups;
    void repositionPopups();
    QScopedPointer<Ui::NotificationPopup> ui;
    QPropertyAnimation *fadeIn;
    QPropertyAnimation *fadeOut;
    Priority m_priority;
    QSoundEffect *soundEffect;
    bool m_soundEnabled;
};
