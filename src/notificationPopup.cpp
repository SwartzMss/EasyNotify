// NotificationPopup.cpp
#include <QScreen>
#include <QApplication>
#include <QPushButton>
#include <QStyle>
#include "notificationPopup.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ui_notificationPopup.h"
#include <QScopedPointer>
#include <QCursor>
#include <QList>
#include <QPointer>

QList<QPointer<NotificationPopup>> NotificationPopup::s_popups;

NotificationPopup::NotificationPopup(const QString &title,
                                     Priority priority,
                                     QWidget *parent)
  : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint),
    ui(new Ui::NotificationPopup),
    m_priority(priority)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_StyledBackground);
    setAutoFillBackground(true);

    // Slight shadow to lift the popup off the screen
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(12);
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);

    // 设置标题图标和消息
    ui->titleLabel->setPixmap(QIcon(":/img/tray_icon_active.png").pixmap(24, 24));
    ui->titleTextLabel->clear();
    ui->messageLabel->setText(title);

    // 根据优先级选择图标
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
    ui->priorityLabel->setPixmap(icon.pixmap(20, 20));
    
    // 关闭按钮
    connect(ui->closeButton, &QPushButton::clicked, this, &NotificationPopup::close);
    
    // 淡入动画
    fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(1);

    setStyleSheet(R"(
      QWidget {
        background: #2C2C2C;
        border-radius: 10px;
      }
    )");

    setAttribute(Qt::WA_DeleteOnClose);

    // 设置头部和内容区域不同背景色
    // 头部采用稍浅的强调色，并在底部加入边框与内容区域区分
    ui->headerWidget->setStyleSheet(
        "background: #3A3F44;"
        " border-top-left-radius: 10px;"
        " border-top-right-radius: 10px;"
        " border-bottom: 1px solid #232323;");
    // 内容区域保持较暗背景色
    ui->contentWidget->setStyleSheet(
        "background: #232323;"
        " border-bottom-left-radius: 10px;"
        " border-bottom-right-radius: 10px;");
    
    setFixedSize(250, 100);
}

void NotificationPopup::show()
{
    adjustSize();

    QPoint cursorPos = QCursor::pos();
    QScreen *screen = QGuiApplication::screenAt(cursorPos);
    if (!screen) screen = QGuiApplication::primaryScreen();

    QRect avail = screen->availableGeometry();

    const int margin = 8;
    int index = s_popups.size();
    int x = avail.x() + avail.width()  - width()  - margin;
    int y = avail.y() + avail.height() - ((index + 1) * (height() + margin));
    move(x, y);

    setWindowOpacity(0);
    QWidget::show();
    fadeIn->start();

    s_popups.append(this);
}

void NotificationPopup::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);

    int index = s_popups.indexOf(this);
    if (index != -1)
        s_popups.removeAt(index);

    repositionPopups();
}

void NotificationPopup::repositionPopups()
{
    if (s_popups.isEmpty())
        return;

    const int margin = 8;
    for (int i = 0; i < s_popups.size(); ++i) {
        NotificationPopup *popup = s_popups.at(i);
        if (!popup)
            continue;

        QPoint pos = popup->pos();
        QScreen *screen = QGuiApplication::screenAt(pos);
        if (!screen) screen = QGuiApplication::primaryScreen();

        QRect avail = screen->availableGeometry();
        int x = avail.x() + avail.width() - popup->width() - margin;
        int y = avail.y() + avail.height() - ((i + 1) * (popup->height() + margin));
        popup->move(x, y);
    }
}
