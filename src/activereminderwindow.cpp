#include "activereminderwindow.h"
#include "ui_activereminderwindow.h"

ActiveReminderWindow::ActiveReminderWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ActiveReminderWindow)
{
    ui->setupUi(this);
}

ActiveReminderWindow::~ActiveReminderWindow()
{
    delete ui;
}

void ActiveReminderWindow::setReminderManager(ReminderManager *manager)
{
    if (ui->activeList)
        ui->activeList->setReminderManager(manager);
}
