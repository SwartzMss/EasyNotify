#ifndef ACTIVEREMINDERWINDOW_H
#define ACTIVEREMINDERWINDOW_H

#include <QWidget>
#include <QShowEvent>
#include "active_reminderlist.h"

namespace Ui {
class ActiveReminderWindow;
}

class ActiveReminderWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ActiveReminderWindow(QWidget *parent = nullptr);
    ~ActiveReminderWindow();

    void setReminderManager(ReminderManager *manager);

private slots:
    void refreshReminders();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::ActiveReminderWindow *ui;
    ReminderManager *reminderManager;
};

#endif // ACTIVEREMINDERWINDOW_H
