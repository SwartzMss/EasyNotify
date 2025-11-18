#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include "activereminderwindow.h"
#include "completedreminderwindow.h"
#include "remindermanager.h"
#include "notificationPopup.h"
#include "remoteclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowMainWindow();
    void onPauseReminders();
    void onToggleAutoStart();
    void onToggleSound();
    void onThemeChanged(int index);
    void onQuit();
    void displayNotification(const Reminder &reminder);
    void onRemoteMessage(const QString &message);

private:
    void setupUI();
    void setupConnections();
    void createTrayIcon();
    void createActions();

    Ui::MainWindow *ui;
    ActiveReminderWindow *activeWindow;
    CompletedReminderWindow *completedWindow;
    ReminderManager *reminderManager;
    RemoteClient *remoteClient;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *showAction;
    QAction *pauseAction;
    QAction *autoStartAction;
    QAction *soundAction;
    QAction *quitAction;
    QComboBox *themeSelector;
    bool isPaused;
    bool autoStartEnabled;
    bool soundEnabled;

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H 
