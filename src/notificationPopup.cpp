// NotificationPopup.cpp
#include <QScreen>
#include <QApplication>
#include <QPushButton>
#include "NotificationPopup.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ui_notificationPopup.h"
#include <QScopedPointer>
#include <QStyle>
#include <QGuiApplication>
#include <QCursor>

NotificationPopup::NotificationPopup(const QString &title,
                                     const QString &message,
                                     NotificationPopup::Priority priority,
                                     const QIcon &icon,
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

    // 设置标题
    ui->titleLabel->setText(title);
    if (message.isEmpty()) {
        ui->messageLabel->setText(tr("你有新的提醒"));
    } else {
        ui->messageLabel->setText(message);
    }

    QIcon finalIcon = icon;
    if (finalIcon.isNull()) {
        QStyle::StandardPixmap sp = QStyle::SP_MessageBoxInformation;
        switch (priority) {
        case NotificationPopup::Priority::Low:
            sp = QStyle::SP_MessageBoxInformation;
            break;
        case NotificationPopup::Priority::Medium:
            sp = QStyle::SP_MessageBoxWarning;
            break;
        case NotificationPopup::Priority::High:
            sp = QStyle::SP_MessageBoxCritical;
            break;
        }
        finalIcon = style()->standardIcon(sp);
    }
    ui->priorityLabel->setPixmap(finalIcon.pixmap(24, 24));
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

NotificationPopup::NotificationPopup(const QString &title,
                                     const QIcon &icon,
                                     int timeoutMs,
                                     QWidget *parent)
    : NotificationPopup(title, {}, NotificationPopup::Priority::Medium, icon, timeoutMs, parent)
{
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
