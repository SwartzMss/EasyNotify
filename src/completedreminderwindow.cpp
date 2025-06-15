#include "completedreminderwindow.h"
#include "ui_completedreminderwindow.h"
#include <QPushButton>

CompletedReminderWindow::CompletedReminderWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::CompletedReminderWindow),
      reminderList(nullptr),
      reminderManager(nullptr)
{
    ui->setupUi(this);
    reminderList = new ReminderList(this);
    ui->verticalLayout->addWidget(reminderList);

    // hide editing buttons
    if (QPushButton *btn = reminderList->findChild<QPushButton*>("addButton"))
        btn->setVisible(false);
    if (QPushButton *btn = reminderList->findChild<QPushButton*>("deleteButton"))
        btn->setVisible(false);
    if (QPushButton *btn = reminderList->findChild<QPushButton*>("importButton"))
        btn->setVisible(false);
    if (QPushButton *btn = reminderList->findChild<QPushButton*>("exportButton"))
        btn->setVisible(false);
}

CompletedReminderWindow::~CompletedReminderWindow()
{
    delete ui;
}

void CompletedReminderWindow::setReminderManager(ReminderManager *manager)
{
    if (reminderManager)
        disconnect(reminderManager, &ReminderManager::reminderTriggered, this, &CompletedReminderWindow::refreshReminders);
    reminderManager = manager;
    if (reminderList)
        reminderList->setReminderManager(manager);
    if (reminderManager) {
        connect(reminderManager, &ReminderManager::reminderTriggered, this, &CompletedReminderWindow::refreshReminders);
        refreshReminders();
    }
}

void CompletedReminderWindow::refreshReminders()
{
    if (!reminderManager)
        return;
    QList<Reminder> filtered;
    for (const Reminder &r : reminderManager->getReminders()) {
        if (r.completed())
            filtered.append(r);
    }
    reminderList->updateList(filtered);
}
