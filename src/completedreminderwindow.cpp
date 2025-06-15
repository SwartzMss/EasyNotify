#include "completedreminderwindow.h"
#include "ui_completedreminderwindow.h"
#include <QPushButton>
#include <QTableView>
#include <QModelIndex>

CompletedReminderWindow::CompletedReminderWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CompletedReminderWindow)
    , reminderManager(nullptr)
{
    ui->setupUi(this);

    if (ui->completedList) {
        ui->completedList->addButton()->setVisible(false);
        ui->completedList->importButton()->setVisible(false);
        ui->completedList->exportButton()->setVisible(false);
        connect(ui->completedList->deleteButton(), &QPushButton::clicked,
                this, [this]() { ui->completedList->onDeleteClicked(); refreshReminders(); });
    }
}

CompletedReminderWindow::~CompletedReminderWindow()
{
    delete ui;
}

void CompletedReminderWindow::setReminderManager(ReminderManager *manager)
{
    reminderManager = manager;
    if (ui->completedList)
        ui->completedList->setReminderManager(manager);
    if (reminderManager) {
        connect(reminderManager, &ReminderManager::reminderTriggered,
                this, &CompletedReminderWindow::refreshReminders, Qt::UniqueConnection);
    }
    refreshReminders();
}

void CompletedReminderWindow::refreshReminders()
{
    if (!reminderManager)
        return;
    QList<Reminder> filtered;
    const QVector<Reminder> all = reminderManager->getReminders();
    for (const Reminder &r : all) {
        if (r.completed())
            filtered.append(r);
    }
    ui->completedList->loadReminders(filtered);
}
