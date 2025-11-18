#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

#include "activereminderwindow.h"
#include "completedreminderwindow.h"
#include "configmanager.h"
#include "logger.h"
#include "remoteclient.h"
#include "thememanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isPaused( ConfigManager::instance().isPaused())
    , autoStartEnabled(false)
    , soundEnabled(true)
    , themeSelector(nullptr)
{
    setWindowFlags(
        Qt::Window                                   
      | Qt::CustomizeWindowHint                       
      | Qt::WindowTitleHint                           
      | Qt::WindowCloseButtonHint                     
    );
    ui->setupUi(this);

    setupUI();

    if (themeSelector) {
        const QString savedTheme = ConfigManager::instance().theme();
        int savedIndex = themeSelector->findData(savedTheme);
        if (savedIndex < 0) {
            savedIndex = themeSelector->findData(QStringLiteral("light"));
        }
        if (savedIndex >= 0) {
            themeSelector->setCurrentIndex(savedIndex);
        }
        onThemeChanged(themeSelector->currentIndex());
    }

    // 创建提醒管理器(运行于工作线程)
    reminderManager = new ReminderManager();
    connect(reminderManager, &ReminderManager::reminderTriggered,
            this, &MainWindow::displayNotification);

    remoteClient = new RemoteClient(QUrl(ConfigManager::instance().remoteUrl()), this);
    connect(remoteClient, &RemoteClient::remoteMessageReceived,
            this, &MainWindow::onRemoteMessage);
    remoteClient->start();

    // 连接提醒列表和提醒管理器
    activeWindow->setReminderManager(reminderManager);
    completedWindow->setReminderManager(reminderManager);
    
    // 创建系统托盘图标
    createTrayIcon();
    createActions();
    setupConnections();

    // 加载配置
    autoStartEnabled = ConfigManager::instance().isAutoStart();
    soundEnabled = ConfigManager::instance().isSoundEnabled();
    if (isPaused) {
        pauseAction->setText(tr("关闭勿扰模式"));
        reminderManager->pauseAll();
    }
    if (autoStartEnabled) {
        autoStartAction->setText(tr("取消开机启动"));
    }
    if (soundEnabled) {
        soundAction->setText(tr("关闭声音提醒"));
    } else {
        soundAction->setText(tr("开启声音提醒"));
    }
}

void MainWindow::displayNotification(const Reminder &reminder)
{
    NotificationPopup *popup = new NotificationPopup(reminder.name(), reminder.priority(), soundEnabled);
    popup->show();
}

void MainWindow::onRemoteMessage(const QString &message)
{
    NotificationPopup *popup = new NotificationPopup(message, Reminder::Priority::Medium, soundEnabled);
    popup->show();
}

MainWindow::~MainWindow()
{
    LOG_INFO("MainWindow 析构");
    delete reminderManager;
    delete remoteClient;
    delete activeWindow;
    delete completedWindow;
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle(tr("EasyNotify"));

    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName(QStringLiteral("centralWidget"));
    setCentralWidget(centralWidget);

    // 创建垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 添加主题切换条
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    QLabel *themeLabel = new QLabel(tr("界面主题"), centralWidget);
    themeSelector = new QComboBox(centralWidget);
    themeSelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    themeSelector->setMinimumWidth(160);
    const QVector<ThemeManager::Theme> themes = ThemeManager::instance().availableThemes();
    for (ThemeManager::Theme theme : themes) {
        themeSelector->addItem(
            ThemeManager::instance().displayName(theme),
            ThemeManager::instance().key(theme));
    }
    toolbarLayout->addWidget(themeLabel);
    toolbarLayout->addWidget(themeSelector);
    toolbarLayout->addStretch();
    mainLayout->addLayout(toolbarLayout);

    // 创建标签页控件
    QTabWidget *tabWidget = new QTabWidget(centralWidget);
    mainLayout->addWidget(tabWidget);

    // 创建两个提醒窗口
    activeWindow = new ActiveReminderWindow(tabWidget);
    completedWindow = new CompletedReminderWindow(tabWidget);

    // 将窗口添加到标签页
    tabWidget->addTab(activeWindow, tr("当前提醒"));
    tabWidget->addTab(completedWindow, tr("已完成提醒"));

    // 设置窗口大小
    setFixedSize(600, 400);
}

void MainWindow::setupConnections()
{
    connect(trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::onTrayIconActivated);
    connect(showAction, &QAction::triggered,
            this, &MainWindow::onShowMainWindow);
    connect(pauseAction, &QAction::triggered,
            this, &MainWindow::onPauseReminders);
    connect(autoStartAction, &QAction::triggered,
            this, &MainWindow::onToggleAutoStart);
    connect(soundAction, &QAction::triggered,
            this, &MainWindow::onToggleSound);
    connect(quitAction, &QAction::triggered,
            this, &MainWindow::onQuit);
    if (themeSelector) {
        connect(themeSelector, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &MainWindow::onThemeChanged);
    }
}

void MainWindow::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(isPaused ? ":/img/tray_icon_paused.png" : ":/img/tray_icon.png"));
    trayIcon->setToolTip(tr("EasyNotify"));
    trayIcon->show();
}

void MainWindow::createActions()
{
    trayIconMenu = new QMenu(this);

    showAction = new QAction(tr("显示主界面"), this);
    pauseAction = new QAction(tr("开启勿扰模式"), this);
    autoStartAction = new QAction(tr("开机启动"), this);
    soundAction = new QAction(tr("关闭声音提醒"), this);
    quitAction = new QAction(tr("退出"), this);

    trayIconMenu->addAction(showAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(pauseAction);
    trayIconMenu->addAction(autoStartAction);
    trayIconMenu->addAction(soundAction);
    trayIconMenu->addAction(quitAction);
    
    trayIcon->setContextMenu(trayIconMenu);
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    LOG_DEBUG(QString("托盘图标激活，原因: %1").arg(reason));
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowMainWindow();
    }
}

void MainWindow::onShowMainWindow()
{
    LOG_DEBUG("显示主界面");
    show();
    activateWindow();
    raise();
}

void MainWindow::onPauseReminders()
{
    LOG_DEBUG("切换提醒暂停状态");
    isPaused = !isPaused;
    if (isPaused) {
        pauseAction->setText(tr("关闭勿扰模式"));
        reminderManager->pauseAll();
        trayIcon->setIcon(QIcon(":/img/tray_icon_paused.png"));
    } else {
        pauseAction->setText(tr("开启勿扰模式"));
        reminderManager->resumeAll();
        trayIcon->setIcon(QIcon(":/img/tray_icon.png"));
    }
    ConfigManager::instance().setPaused(isPaused);
    LOG_INFO(QString("勿扰模式已%1").arg(isPaused ? "开启" : "关闭"));
}

void MainWindow::onToggleAutoStart()
{
    LOG_DEBUG("切换开机启动状态");
    autoStartEnabled = !autoStartEnabled;
    if (autoStartEnabled) {
        autoStartAction->setText(tr("取消开机启动"));
    } else {
        autoStartAction->setText(tr("开机启动"));
    }
    ConfigManager::instance().setAutoStart(autoStartEnabled);
    LOG_INFO(QString("开机启动已%1").arg(autoStartEnabled ? "启用" : "禁用"));
}

void MainWindow::onToggleSound()
{
    LOG_DEBUG("切换声音提醒状态");
    soundEnabled = !soundEnabled;
    if (soundEnabled) {
        soundAction->setText(tr("关闭声音提醒"));
    } else {
        soundAction->setText(tr("开启声音提醒"));
    }
    ConfigManager::instance().setSoundEnabled(soundEnabled);
    LOG_INFO(QString("声音提醒已%1").arg(soundEnabled ? "开启" : "关闭"));
}

void MainWindow::onThemeChanged(int index)
{
    if (!themeSelector || index < 0) {
        return;
    }
    const QString themeKey = themeSelector->itemData(index).toString();
    ThemeManager &manager = ThemeManager::instance();
    ThemeManager::Theme theme = manager.themeFromKey(themeKey);
    manager.applyTheme(theme);
    ConfigManager::instance().setTheme(manager.key(theme));
    LOG_INFO(QString("用户切换主题: %1").arg(manager.key(theme)));
}

void MainWindow::onQuit()
{
    LOG_INFO("用户选择退出");
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("确认退出"),
                                tr("确定要退出程序吗？"),
                                QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        LOG_INFO("用户确认退出");
        QApplication::quit();
    } else {
        LOG_INFO("用户取消退出");
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG("closeEvent 触发");
    if (trayIcon->isVisible()) {
        LOG_DEBUG("托盘图标可见，隐藏窗口");
        hide();
        event->ignore();
    } else {
        LOG_DEBUG("托盘图标不可见，正常关闭窗口");
        QMainWindow::closeEvent(event);
    }
}
