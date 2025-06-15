#include "activereminderwindow.h"
#include "ui_activereminderwindow.h"

ActiveReminderWindow::ActiveReminderWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::ActiveReminderWindow),
      reminderList(nullptr)
{
    ui->setupUi(this);
    reminderList = ui->activeList;
    if (reminderList)
        ; // placeholder to avoid unused variable warning
}

ActiveReminderWindow::~ActiveReminderWindow()
{
    delete ui;
}

void ActiveReminderWindow::setReminderManager(ReminderManager *manager)
{
    if (reminderList)
        reminderList->setReminderManager(manager);
}
