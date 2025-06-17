#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSettings>
#include "activereminderwindow.h"
#include "completedreminderwindow.h"
#include <QCloseEvent>
#include "configmanager.h"
#include "logger.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isPaused(false)
    , autoStartEnabled(false)
{
    setWindowFlags(
        Qt::Window                                   
      | Qt::CustomizeWindowHint                       
      | Qt::WindowTitleHint                           
      | Qt::WindowCloseButtonHint                     
    );
    ui->setupUi(this);

    setupUI();

    // 创建提醒管理器
    reminderManager = new ReminderManager(this);

    // 连接提醒列表和提醒管理器
    activeWindow->setReminderManager(reminderManager);
    completedWindow->setReminderManager(reminderManager);
    
    // 创建系统托盘图标
    createTrayIcon();
    createActions();
    setupConnections();

    // 加载配置
    isPaused = ConfigManager::instance().isPaused();
    autoStartEnabled = ConfigManager::instance().isAutoStart();
    if (isPaused) {
        pauseAction->setText(tr("恢复提醒"));
        reminderManager->pauseAll();
    }
    if (autoStartEnabled) {
        autoStartAction->setText(tr("取消开机启动"));
    }
}

MainWindow::~MainWindow()
{
    LOG_INFO("MainWindow 析构");
    delete activeWindow;
    delete completedWindow;
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle(tr("EasyNotify"));

    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

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
    connect(quitAction, &QAction::triggered,
            this, &MainWindow::onQuit);
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
    pauseAction = new QAction(tr("暂停提醒"), this);
    autoStartAction = new QAction(tr("开机启动"), this);
    quitAction = new QAction(tr("退出"), this);

    trayIconMenu->addAction(showAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(pauseAction);
    trayIconMenu->addAction(autoStartAction);
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
    if (activeWindow)
        activeWindow->show(), activeWindow->activateWindow(), activeWindow->raise();
    if (completedWindow)
        completedWindow->show(), completedWindow->activateWindow(), completedWindow->raise();
}

void MainWindow::onPauseReminders()
{
    LOG_DEBUG("切换提醒暂停状态");
    isPaused = !isPaused;
    if (isPaused) {
        pauseAction->setText(tr("恢复提醒"));
        reminderManager->pauseAll();
        trayIcon->setIcon(QIcon(":/img/tray_icon_paused.png"));
    } else {
        pauseAction->setText(tr("暂停提醒"));
        reminderManager->resumeAll();
        trayIcon->setIcon(QIcon(":/img/tray_icon.png"));
    }
    ConfigManager::instance().setPaused(isPaused);
    LOG_INFO(QString("提醒已%1").arg(isPaused ? "暂停" : "恢复"));
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
        if (activeWindow)
            activeWindow->hide();
        if (completedWindow)
            completedWindow->hide();
        event->ignore();
    } else {
        LOG_DEBUG("托盘图标不可见，正常关闭窗口");
        QMainWindow::closeEvent(event);
    }
}
