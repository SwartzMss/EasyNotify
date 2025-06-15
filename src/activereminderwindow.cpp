#include "activereminderwindow.h"
#include "ui_activereminderwindow.h"
#include <QPushButton>
#include <QTableView>

ActiveReminderWindow::ActiveReminderWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ActiveReminderWindow),
      reminderList(nullptr),
      reminderManager(nullptr)
{
    ui->setupUi(this);
    reminderList = new ReminderList(this);
    ui->verticalLayout->addWidget(reminderList);

    // connect buttons to reminder list slots
    if (QPushButton *btn = reminderList->findChild<QPushButton*>("addButton"))
        connect(btn, &QPushButton::clicked, reminderList, &ReminderList::onAddClicked);
    if (QPushButton *btn = reminderList->findChild<QPushButton*>("deleteButton"))
        connect(btn, &QPushButton::clicked, reminderList, &ReminderList::onDeleteClicked);
    if (QPushButton *btn = reminderList->findChild<QPushButton*>("importButton"))
        connect(btn, &QPushButton::clicked, reminderList, &ReminderList::onImportClicked);
    if (QPushButton *btn = reminderList->findChild<QPushButton*>("exportButton"))
        connect(btn, &QPushButton::clicked, reminderList, &ReminderList::onExportClicked);
    if (QTableView *tv = reminderList->findChild<QTableView*>("tableView"))
        connect(tv, &QTableView::doubleClicked, reminderList, &ReminderList::onEditClicked);
}

ActiveReminderWindow::~ActiveReminderWindow()
{
    delete ui;
}

void ActiveReminderWindow::setReminderManager(ReminderManager *manager)
{
    if (reminderManager)
        disconnect(reminderManager, &ReminderManager::reminderTriggered, this, &ActiveReminderWindow::refreshReminders);
    reminderManager = manager;
    if (reminderList)
        reminderList->setReminderManager(manager);
    if (reminderManager) {
        connect(reminderManager, &ReminderManager::reminderTriggered, this, &ActiveReminderWindow::refreshReminders);
        refreshReminders();
    }
}

void ActiveReminderWindow::refreshReminders()
{
    if (!reminderManager)
        return;
    QList<Reminder> filtered;
    for (const Reminder &r : reminderManager->getReminders()) {
        if (!r.completed())
            filtered.append(r);
    }
    reminderList->updateList(filtered);
}
