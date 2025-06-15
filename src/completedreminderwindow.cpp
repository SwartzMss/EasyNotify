#include "completedreminderwindow.h"
#include "ui_completedreminderwindow.h"

CompletedReminderWindow::CompletedReminderWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::CompletedReminderWindow)
{
    ui->setupUi(this);
}

CompletedReminderWindow::~CompletedReminderWindow()
{
    delete ui;
}

void CompletedReminderWindow::setReminderManager(ReminderManager *manager)
{
    if (ui->completedList)
        ui->completedList->setReminderManager(manager);
}
