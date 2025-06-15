#ifndef COMPLETEDREMINDERWINDOW_H
#define COMPLETEDREMINDERWINDOW_H

#include <QWidget>
#include "reminderlist.h"

namespace Ui {
class CompletedReminderWindow;
}

class CompletedReminderWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CompletedReminderWindow(QWidget *parent = nullptr);
    ~CompletedReminderWindow();

    void setReminderManager(ReminderManager *manager);

private slots:
    void refreshReminders();

private:
    Ui::CompletedReminderWindow *ui;
    ReminderManager *reminderManager;
};

#endif // COMPLETEDREMINDERWINDOW_H
