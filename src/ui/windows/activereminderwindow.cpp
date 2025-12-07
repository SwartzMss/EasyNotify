#include "ui/windows/activereminderwindow.h"
#include "ui_activereminderwindow.h"
#include <QPushButton>
#include <QTableView>
#include <QModelIndex>
#include <QShowEvent>

ActiveReminderWindow::ActiveReminderWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActiveReminderWindow)
    , reminderManager(nullptr)
{
    ui->setupUi(this);

    if (ui->activeList) {
        connect(ui->activeList->addButton(), &QPushButton::clicked,
                this, [this]() { ui->activeList->onAddClicked(); refreshReminders(); });
        connect(ui->activeList->deleteButton(), &QPushButton::clicked,
                this, [this]() { ui->activeList->onDeleteClicked(); refreshReminders(); });
        connect(ui->activeList->tableView(), &QTableView::doubleClicked,
                this, [this](const QModelIndex &) {
                    ui->activeList->onEditClicked();
                    // 刷新列表需要在事件处理完成后再执行，
                    // 否则在双击事件链中重置模型可能导致崩溃
                    QMetaObject::invokeMethod(this,
                                            &ActiveReminderWindow::refreshReminders,
                                            Qt::QueuedConnection);
                });
    }
}

ActiveReminderWindow::~ActiveReminderWindow()
{
    delete ui;
}

void ActiveReminderWindow::setReminderManager(ReminderManager *manager)
{
    reminderManager = manager;
    if (ui->activeList)
        ui->activeList->setReminderManager(manager);
    if (reminderManager) {
        connect(reminderManager, &ReminderManager::reminderTriggered,
                this, &ActiveReminderWindow::refreshReminders, Qt::UniqueConnection);
    }
    refreshReminders();
}

void ActiveReminderWindow::refreshReminders()
{
    if (!reminderManager)
        return;
    QList<Reminder> filtered;
    const QVector<Reminder> all = reminderManager->getReminders();
    for (const Reminder &r : all) {
        if (!r.completed())
            filtered.append(r);
    }
    ui->activeList->loadReminders(filtered);
}

void ActiveReminderWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshReminders();
}
