// NotificationPopup.cpp
#include <QScreen>
#include <QApplication>
#include <QPushButton>
#include <QStyle>
#include "NotificationPopup.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ui_notificationPopup.h"
#include <QScopedPointer>

NotificationPopup::NotificationPopup(const QString &title,
                                     const QString &message,
                                     Priority priority,
                                     int timeoutMs,
                                     QWidget *parent)
  : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint),
    ui(new Ui::NotificationPopup),
    m_message(message),
    m_priority(priority)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_StyledBackground);
    setAutoFillBackground(true);

    // set title and message
    ui->titleLabel->setText(title);
    ui->messageLabel->setText(m_message.isEmpty() ? tr("Notification") : m_message);

    // choose icon based on priority
    QStyle *style = QApplication::style();
    QIcon icon;
    switch (m_priority) {
    case Priority::Low:
        icon = style->standardIcon(QStyle::SP_MessageBoxInformation);
        break;
    case Priority::High:
        icon = style->standardIcon(QStyle::SP_MessageBoxCritical);
        break;
    case Priority::Medium:
    default:
        icon = style->standardIcon(QStyle::SP_MessageBoxWarning);
        break;
    }
    ui->priorityLabel->setPixmap(icon.pixmap(24, 24));
    // 关闭按钮
    connect(ui->closeButton, &QPushButton::clicked, this, &NotificationPopup::close);
    // 动画和定时器逻辑保持不变
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
    if (timeoutMs > 0) {
        closeTimer->start();
    }

    setStyleSheet(R"(
      QWidget {
        background: #FFD600;
        border-radius: 10px;
      }
    )");
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
