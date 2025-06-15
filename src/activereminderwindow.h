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

private:
    Ui::ActiveReminderWindow *ui;
};

#endif // ACTIVEREMINDERWINDOW_H
