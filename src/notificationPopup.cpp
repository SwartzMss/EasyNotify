// NotificationPopup.cpp
#include "NotificationPopup.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScreen>
#include <QApplication>

NotificationPopup::NotificationPopup(const QString &title,
                                     const QString &message,
                                     const QIcon &icon,
                                     int timeoutMs,
                                     QWidget *parent)
  : QWidget(nullptr, 
      Qt::FramelessWindowHint 
    | Qt::WindowStaysOnTopHint 
    | Qt::X11BypassWindowManagerHint)
{
    // 透明背景
    setAttribute(Qt::WA_TranslucentBackground);
    // 不抢焦点
    setAttribute(Qt::WA_ShowWithoutActivating);

    // 圆角+半透明黑背景
    setStyleSheet(R"(
      QWidget {
        background: rgba(30,30,30,220);
        border-radius: 10px;
      }
    )");

    // 阴影
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 160));
    setGraphicsEffect(shadow);

    // 内容
    iconLabel  = new QLabel; if (!icon.isNull()) iconLabel->setPixmap(icon.pixmap(32));
    titleLabel = new QLabel(title);
    msgLabel   = new QLabel(message);

    titleLabel->setStyleSheet("color:white; font-weight:bold;");
    msgLabel  ->setStyleSheet("color:white;");

    auto *vbox = new QVBoxLayout;
    vbox->addWidget(titleLabel);
    vbox->addWidget(msgLabel);

    auto *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(12, 12, 12, 12);
    if (!icon.isNull()) hbox->addWidget(iconLabel);
    hbox->addLayout(vbox);

    // 动画
    fadeIn  = new QPropertyAnimation(this, "windowOpacity", this);
    fadeIn ->setDuration(300);
    fadeIn ->setStartValue(0);
    fadeIn ->setEndValue(1);

    fadeOut = new QPropertyAnimation(this, "windowOpacity", this);
    fadeOut->setDuration(300);
    fadeOut->setStartValue(1);
    fadeOut->setEndValue(0);
    connect(fadeOut, &QPropertyAnimation::finished, this, &NotificationPopup::deleteLater);

    closeTimer = new QTimer(this);
    closeTimer->setSingleShot(true);
    closeTimer->setInterval(timeoutMs);
    connect(closeTimer, &QTimer::timeout, this, [this]() {
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}


void NotificationPopup::show()
{
    adjustSize();

    QPoint cursorPos = QCursor::pos();
    QScreen *screen = QGuiApplication::screenAt(cursorPos);
    if (!screen) screen = QGuiApplication::primaryScreen();

    QRect avail = screen->availableGeometry();

    int x = avail.x() + avail.width()  - width()  - 8;
    int y = avail.y() + avail.height() - height() - 8;
    move(x, y);

    setWindowOpacity(0);
    QWidget::show();
    fadeIn->start();
    closeTimer->start();
}
