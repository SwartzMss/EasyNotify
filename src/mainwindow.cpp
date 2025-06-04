#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isPaused(false)
{
    ui->setupUi(this);
    setupUI();
    
    // 创建提醒管理器
    reminderManager = new ReminderManager(this);
    
    // 连接提醒列表和提醒管理器
    reminderList->setReminderManager(reminderManager);
    
    // 创建系统托盘图标
    createTrayIcon();
    createActions();
    setupConnections();
    
    // 加载设置
    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle(tr("EasyNotify"));
    resize(800, 600);

    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 创建提醒列表
    reminderList = new ReminderList(this);
    mainLayout->addWidget(reminderList);

    centralWidget->setLayout(mainLayout);
}

void MainWindow::setupConnections()
{
    connect(trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::onTrayIconActivated);
    connect(showAction, &QAction::triggered,
            this, &MainWindow::onShowMainWindow);
    connect(addAction, &QAction::triggered,
            this, &MainWindow::onAddReminder);
    connect(pauseAction, &QAction::triggered,
            this, &MainWindow::onPauseReminders);
    connect(quitAction, &QAction::triggered,
            this, &MainWindow::onQuit);
}

void MainWindow::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/img/tray_icon.png"));
    trayIcon->setToolTip(tr("EasyNotify"));
    trayIcon->show();
}

void MainWindow::createActions()
{
    trayIconMenu = new QMenu(this);
    
    showAction = new QAction(tr("显示主界面"), this);
    addAction = new QAction(tr("添加提醒"), this);
    pauseAction = new QAction(tr("暂停提醒"), this);
    quitAction = new QAction(tr("退出"), this);
    
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(addAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(pauseAction);
    trayIconMenu->addAction(quitAction);
    
    trayIcon->setContextMenu(trayIconMenu);
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowMainWindow();
    }
}

void MainWindow::onShowMainWindow()
{
    show();
    activateWindow();
    raise();
}

void MainWindow::onAddReminder()
{
    reminderList->onAddClicked();
}

void MainWindow::onPauseReminders()
{
    isPaused = !isPaused;
    if (isPaused) {
        pauseAction->setText(tr("恢复提醒"));
        reminderManager->pauseAll();
    } else {
        pauseAction->setText(tr("暂停提醒"));
        reminderManager->resumeAll();
    }
}

void MainWindow::onQuit()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("确认退出"),
                                tr("确定要退出程序吗？"),
                                QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        saveSettings();
        QApplication::quit();
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    isPaused = settings.value("isPaused", false).toBool();
    
    if (isPaused) {
        pauseAction->setText(tr("恢复提醒"));
        reminderManager->pauseAll();
    }
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("isPaused", isPaused);
} 