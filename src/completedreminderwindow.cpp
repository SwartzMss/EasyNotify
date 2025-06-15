#include "completedreminderwindow.h"
#include "ui_completedreminderwindow.h"

CompletedReminderWindow::CompletedReminderWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::CompletedReminderWindow),
      reminderList(nullptr)
{
    ui->setupUi(this);
    reminderList = new ReminderList(ReminderList::Mode::Completed, this);
    ui->verticalLayout->addWidget(reminderList);
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
