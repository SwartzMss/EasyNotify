#ifndef ACTIVEREMINDERWINDOW_H
#define ACTIVEREMINDERWINDOW_H

#include <QWidget>
#include "reminderlist.h"

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

private:
    Ui::ActiveReminderWindow *ui;
    ReminderList *reminderList;
    ReminderManager *reminderManager;
};

#endif // ACTIVEREMINDERWINDOW_H
