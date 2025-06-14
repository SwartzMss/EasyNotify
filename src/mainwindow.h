#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "reminderlist.h"
#include <QTabWidget>
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
    void onPauseReminders();
    void onQuit();

private:
    void setupUI();
    void setupConnections();
    void createTrayIcon();
    void createActions();

    Ui::MainWindow *ui;
    ReminderList *reminderList;
    ReminderList *completedList;
    QTabWidget *tabWidget;
    ReminderManager *reminderManager;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *showAction;
    QAction *pauseAction;
    QAction *quitAction;
    bool isPaused;

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H 