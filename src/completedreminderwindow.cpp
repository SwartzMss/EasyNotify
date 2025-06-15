#include "completedreminderwindow.h"
#include "ui_completedreminderwindow.h"

CompletedReminderWindow::CompletedReminderWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::CompletedReminderWindow),
      reminderList(nullptr)
{
    ui->setupUi(this);
    reminderList = ui->completedList;
    if (reminderList)
        ;
}

CompletedReminderWindow::~CompletedReminderWindow()
{
    delete ui;
}

void CompletedReminderWindow::setReminderManager(ReminderManager *manager)
{
    if (reminderList)
        reminderList->setReminderManager(manager);
}
