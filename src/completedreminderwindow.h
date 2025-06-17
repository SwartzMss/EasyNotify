#ifndef COMPLETEDREMINDERWINDOW_H
#define COMPLETEDREMINDERWINDOW_H

#include <QWidget>
#include <QShowEvent>
#include "completed_reminderlist.h"

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

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::CompletedReminderWindow *ui;
    ReminderManager *reminderManager;
};

#endif // COMPLETEDREMINDERWINDOW_H
