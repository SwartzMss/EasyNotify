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
#include "core/reminders/reminder.h"
#include <QList>
#include <QPointer>
#include <QCloseEvent>
#include <QSoundEffect>
#include <QScreen>

class NotificationPopup : public QWidget {
    Q_OBJECT
public:
    using Priority = Reminder::Priority;
    NotificationPopup(const QString &title,
                      Priority priority = Priority::Medium,
                      bool soundEnabled = true,
                      QWidget *parent = nullptr,
                      QScreen *targetScreen = nullptr);
    
    void show();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void startFadeOut();

private:
    static QList<QPointer<NotificationPopup>> s_cornerPopups;
    static QPointer<NotificationPopup> s_centerPopup;
    void attachToCornerStack();
    void moveToCenter();
    void repositionCornerPopups();
    void scheduleAutoClose();
    QScreen *placementScreen() const;
    QScopedPointer<Ui::NotificationPopup> ui;
    QPropertyAnimation *fadeIn;
    QPropertyAnimation *fadeOut;
    Priority m_priority;
    QSoundEffect *soundEffect;
    bool m_soundEnabled;
    QPointer<QWidget> m_anchorWidget;
    QPointer<QScreen> m_targetScreen;
    bool m_isCornerPopup = false;
};
