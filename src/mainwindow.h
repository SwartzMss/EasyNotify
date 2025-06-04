#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "reminderlist.h"
#include "remindermanager.h"

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
    void onAddReminder();
    void onPauseReminders();
    void onQuit();

private:
    void setupUI();
    void setupConnections();
    void createTrayIcon();
    void createActions();
    void loadSettings();
    void saveSettings();

    Ui::MainWindow *ui;
    ReminderList *reminderList;
    ReminderManager *reminderManager;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *showAction;
    QAction *addAction;
    QAction *pauseAction;
    QAction *quitAction;
    bool isPaused;
};
#endif // MAINWINDOW_H 