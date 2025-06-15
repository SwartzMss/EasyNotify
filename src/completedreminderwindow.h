#ifndef COMPLETEDREMINDERWINDOW_H
#define COMPLETEDREMINDERWINDOW_H

#include <QMainWindow>
#include "reminderlist.h"

namespace Ui {
class CompletedReminderWindow;
}

class CompletedReminderWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CompletedReminderWindow(QWidget *parent = nullptr);
    ~CompletedReminderWindow();

    void setReminderManager(ReminderManager *manager);

private:
    Ui::CompletedReminderWindow *ui;
    ReminderList *reminderList;
};

#endif // COMPLETEDREMINDERWINDOW_H
