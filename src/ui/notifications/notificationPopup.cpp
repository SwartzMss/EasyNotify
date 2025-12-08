// NotificationPopup.cpp
#include <QScreen>
#include <QApplication>
#include <QPushButton>
#include "ui/notifications/notificationPopup.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ui_notificationPopup.h"
#include <QScopedPointer>
#include <QList>
#include <QPointer>
#include "core/providers/priorityiconprovider.h"

QList<QPointer<NotificationPopup>> NotificationPopup::s_cornerPopups;
QPointer<NotificationPopup> NotificationPopup::s_centerPopup;

NotificationPopup::NotificationPopup(const QString &title,
                                     Priority priority,
                                     bool soundEnabled,
                                     QWidget *parent)
  : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint),
    ui(new Ui::NotificationPopup),
    m_priority(priority),
    soundEffect(new QSoundEffect(this)),
    m_soundEnabled(soundEnabled),
    m_anchorWidget(parent)
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

    // 根据优先级选择卡通图标
    QIcon icon = PriorityIconProvider::icon(m_priority);
    ui->priorityLabel->setPixmap(icon.pixmap(24, 24));
    
    // 关闭按钮
    connect(ui->closeButton, &QPushButton::clicked, this, &NotificationPopup::startFadeOut);
    
    // 淡入动画
    fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(1);

    // 淡出动画
    fadeOut = new QPropertyAnimation(this, "windowOpacity", this);
    fadeOut->setDuration(300);
    fadeOut->setStartValue(1);
    fadeOut->setEndValue(0);
    connect(fadeOut, &QPropertyAnimation::finished, this, &NotificationPopup::close);

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
    
    setFixedSize(220, 100);

    soundEffect->setSource(QUrl(QStringLiteral("qrc:/sound/Ding.wav")));
}

void NotificationPopup::show()
{
    adjustSize();

    if (m_priority == Priority::High) {
        if (s_centerPopup && s_centerPopup != this) {
            s_centerPopup->attachToCornerStack();
            s_centerPopup.clear();
        }
        moveToCenter();
        s_centerPopup = this;
    } else {
        attachToCornerStack();
    }

    setWindowOpacity(0);
    QWidget::show();
    fadeIn->start();

    if (m_soundEnabled)
    {
        LOG_INFO("play sound");
        soundEffect->play();
    }

    scheduleAutoClose();
}

void NotificationPopup::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);

    if (m_isCornerPopup) {
        int index = s_cornerPopups.indexOf(this);
        if (index != -1)
            s_cornerPopups.removeAt(index);
        repositionCornerPopups();
        m_isCornerPopup = false;
    }

    if (s_centerPopup == this) {
        s_centerPopup.clear();
    }
}

void NotificationPopup::attachToCornerStack()
{
    if (!m_isCornerPopup) {
        s_cornerPopups.append(this);
        m_isCornerPopup = true;
    }
    repositionCornerPopups();
}

void NotificationPopup::moveToCenter()
{
    m_isCornerPopup = false;

    if (m_anchorWidget) {
        QRect anchorRect = m_anchorWidget->frameGeometry();
        if (anchorRect.isValid()) {
            int x = anchorRect.x() + (anchorRect.width() - width()) / 2;
            int y = anchorRect.y() + (anchorRect.height() - height()) / 2;
            move(x, y);
            return;
        }
    }

    QScreen *screen = m_anchorWidget ? m_anchorWidget->screen() : QGuiApplication::primaryScreen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    QRect avail = screen->availableGeometry();
    int x = avail.x() + (avail.width() - width()) / 2;
    int y = avail.y() + (avail.height() - height()) / 2;
    move(x, y);
}

void NotificationPopup::repositionCornerPopups()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    QRect avail = screen->availableGeometry();
    const int margin = 8;

    int visibleIndex = 0;
    for (int i = 0; i < s_cornerPopups.size(); ++i) {
        NotificationPopup *popup = s_cornerPopups.at(i);
        if (!popup) {
            s_cornerPopups.removeAt(i);
            --i;
            continue;
        }

        int x = avail.x() + avail.width() - popup->width() - margin;
        int y = avail.y() + avail.height() - ((visibleIndex + 1) * (popup->height() + margin));
        popup->move(x, y);
        ++visibleIndex;
    }
}

void NotificationPopup::startFadeOut()
{
    fadeOut->start();
}

void NotificationPopup::scheduleAutoClose()
{
    if (m_priority != Priority::Low)
        return;

    constexpr int kAutoCloseMs = 5 * 60 * 1000;
    QTimer::singleShot(kAutoCloseMs, this, [this]() {
        if (!isVisible())
            return;
        startFadeOut();
    });
}
