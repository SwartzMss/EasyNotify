#ifndef ACTIVEREMINDERWINDOW_H
#define ACTIVEREMINDERWINDOW_H

#include <QMainWindow>
#include "reminderlist.h"

namespace Ui {
class ActiveReminderWindow;
}

class ActiveReminderWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ActiveReminderWindow(QWidget *parent = nullptr);
    ~ActiveReminderWindow();

    void setReminderManager(ReminderManager *manager);

private:
    Ui::ActiveReminderWindow *ui;
    ReminderList *reminderList;
};

#endif // ACTIVEREMINDERWINDOW_H
