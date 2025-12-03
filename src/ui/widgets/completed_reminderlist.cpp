#include "ui/widgets/completed_reminderlist.h"
#include "ui_completed_reminderlist.h"
#include <QHeaderView>
#include <QMessageBox>
#include "core/reminders/remindermanager.h"
#include "core/logging/logger.h"

enum ColumnIndex {
    Name = 0,
    Type = 1,
    PriorityCol = 2,
    NextTrigger = 3
};

CompletedReminderList::CompletedReminderList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CompletedReminderList)
    , reminderManager(nullptr)
    , model(new CompletedReminderTableModel(this))
    , proxyModel(new QSortFilterProxyModel(this))
{
    LOG_INFO("创建提醒列表界面");
    ui->setupUi(this);
    setupModel();
    setupConnections();
    LOG_INFO("提醒列表界面初始化完成");
}

CompletedReminderList::~CompletedReminderList()
{
    LOG_INFO("销毁提醒列表界面");
    delete ui;
}

void CompletedReminderList::setReminderManager(ReminderManager *manager)
{
    LOG_INFO("设置提醒管理器");
    reminderManager = manager;
}

void CompletedReminderList::setupConnections()
{
    LOG_INFO("设置信号连接");
    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &CompletedReminderList::onSearchTextChanged);
    LOG_INFO("信号连接设置完成");
}

void CompletedReminderList::setupModel()
{
    LOG_INFO("设置数据模型");
    // 设置代理模型
    proxyModel->setSourceModel(model);
    proxyModel->setFilterKeyColumn(-1); // 设置为-1表示搜索所有列
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive); // 不区分大小写

    // 设置表格视图
    ui->tableView->setModel(proxyModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setShowGrid(false);
    LOG_INFO("数据模型设置完成");
}

void CompletedReminderList::loadReminders(const QList<Reminder> &reminders)
{
    LOG_INFO(QString("加载提醒列表，共 %1 个提醒").arg(reminders.size()));
    model->loadFromJson(reminders);
}

void CompletedReminderList::deleteReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    Reminder reminder = model->getReminder(sourceIndex.row());
    LOG_INFO(QString("准备删除提醒: 名称='%1'").arg(reminder.name()));
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除提醒 '%1' 吗？").arg(reminder.name()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        model->removeReminder(sourceIndex.row());
        if (reminderManager) {
            reminderManager->deleteReminder(reminder);
            LOG_INFO(QString("提醒删除成功: 名称='%1'").arg(reminder.name()));
        }
    } else {
        LOG_INFO("取消删除提醒");
    }
}


void CompletedReminderList::refreshList()
{
    LOG_INFO(QString("刷新提醒列表，搜索文本: '%1'").arg(m_searchText));
    model->search(m_searchText);
}

void CompletedReminderList::searchReminders(const QString &text)
{
    LOG_INFO(QString("搜索提醒: '%1'").arg(text));
    model->search(text);
}

void CompletedReminderList::onDeleteClicked()
{
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除选中的 %1 个提醒吗？").arg(selectedIndexes.size()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        // 先收集所有要删除的提醒
        QList<QPair<int, Reminder>> toDelete;
        for (const QModelIndex &index : selectedIndexes) {
            QModelIndex sourceIndex = proxyModel->mapToSource(index);
            if (sourceIndex.isValid()) {
                Reminder reminder = model->getReminder(sourceIndex.row());
                toDelete.append({sourceIndex.row(), reminder});
            }
        }

        // 在删除前清空选区，避免模型更新过程中访问无效索引
        ui->tableView->clearSelection();

        // 执行批量删除
        model->removeReminders(toDelete);
        
        // 从提醒管理器中删除
        if (reminderManager) {
            for (const auto &pair : toDelete) {
                reminderManager->deleteReminder(pair.second);
            }
        }
    }
}

void CompletedReminderList::onSearchTextChanged(const QString &text)
{
    LOG_INFO(QString("搜索文本变更: '%1'").arg(text));
    m_searchText = text;
    searchReminders(text);
}

QPushButton *CompletedReminderList::deleteButton() const
{
    return ui->deleteButton;
}

QTableView *CompletedReminderList::tableView() const
{
    return ui->tableView;
}
