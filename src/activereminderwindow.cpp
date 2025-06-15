#include "activereminderwindow.h"
#include "ui_activereminderwindow.h"

ActiveReminderWindow::ActiveReminderWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ActiveReminderWindow),
      reminderList(nullptr)
{
    ui->setupUi(this);
    reminderList = new ReminderList(ReminderList::Mode::Active, this);
    ui->verticalLayout->addWidget(reminderList);
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
