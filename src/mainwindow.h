#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "activereminderwindow.h"
#include "completedreminderwindow.h"
#include "remindermanager.h"
#include "notificationPopup.h"
#include "trayiconbouncer.h"

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
    void onQuit();
    void displayNotification(const Reminder &reminder);

private:
    void setupUI();
    void setupConnections();
    void createTrayIcon();
    void createActions();

    Ui::MainWindow *ui;
    ActiveReminderWindow *activeWindow;
    CompletedReminderWindow *completedWindow;
    ReminderManager *reminderManager;
    QSystemTrayIcon *trayIcon;
    TrayIconBouncer *trayBouncer;
    QMenu *trayIconMenu;
    QAction *showAction;
    QAction *pauseAction;
    QAction *autoStartAction;
    QAction *soundAction;
    QAction *quitAction;
    bool isPaused;
    bool autoStartEnabled;
    bool soundEnabled;

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H 