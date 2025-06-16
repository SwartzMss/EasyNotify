#include "active_reminderlist.h"
#include "ui_active_reminderlist.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QFileDialog>
#include "remindermanager.h"
#include "logger.h"
#include "active_reminderedit.h"
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QCoreApplication>

enum ColumnIndex {
    Name = 0,
    Type = 1,
    PriorityCol = 2,
    NextTrigger = 3
};

ActiveReminderList::ActiveReminderList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ActiveReminderList)
    , reminderManager(nullptr)
    , model(new ActiveReminderTableModel(this))
    , proxyModel(new QSortFilterProxyModel(this))
    , editDialog(new ActiveReminderEdit(this))
{
    LOG_INFO("创建提醒列表界面");
    ui->setupUi(this);
    setupModel();
    setupConnections();
    LOG_INFO("提醒列表界面初始化完成");
}

ActiveReminderList::~ActiveReminderList()
{
    LOG_INFO("销毁提醒列表界面");
    delete ui;
}

void ActiveReminderList::setReminderManager(ReminderManager *manager)
{
    LOG_INFO("设置提醒管理器");
    reminderManager = manager;
}

void ActiveReminderList::setupConnections()
{
    LOG_INFO("设置信号连接");
    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &ActiveReminderList::onSearchTextChanged);
    LOG_INFO("信号连接设置完成");
}

void ActiveReminderList::setupModel()
{
    LOG_INFO("设置数据模型");
    // 设置代理模型
    proxyModel->setSourceModel(model);
    proxyModel->setFilterKeyColumn(-1); // 设置为-1表示搜索所有列
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive); // 不区分大小写

    // 设置表格视图
    ui->tableView->setModel(proxyModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setShowGrid(false);
    LOG_INFO("数据模型设置完成");
}

void ActiveReminderList::loadReminders(const QList<Reminder> &reminders)
{
    LOG_INFO(QString("加载提醒列表，共 %1 个提醒").arg(reminders.size()));
    model->loadFromJson(reminders);
}

QJsonArray ActiveReminderList::getReminders() const
{
    return model->saveToJson();
}

void ActiveReminderList::addNewReminder()
{
    LOG_INFO("添加新提醒");
    editDialog->prepareNewReminder();
    if (editDialog->exec() == QDialog::Accepted) {
        Reminder reminder = editDialog->getReminder();
        if (reminderManager) {
            reminderManager->addReminder(reminder);
            addReminderToModel(reminder);
            LOG_INFO(QString("新提醒添加成功: 名称='%1', ID='%2'")
                    .arg(reminder.name())
                    .arg(reminder.id()));
        }
    } else {
        LOG_INFO("取消添加新提醒");
    }
}

void ActiveReminderList::addReminderToModel(const Reminder &reminder)
{
    LOG_INFO(QString("添加提醒到模型: 名称='%1'").arg(reminder.name()));
    model->addReminder(reminder);
    LOG_INFO("提醒已添加到模型");
}

void ActiveReminderList::updateReminderInModel(const Reminder &reminder)
{
    LOG_INFO(QString("更新提醒: 名称='%1'").arg(reminder.name()));
    
    // 保存当前选择的ID
    QString currentId;
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid()) {
        QModelIndex sourceIndex = proxyModel->mapToSource(currentIndex);
        if (sourceIndex.isValid()) {
            currentId = model->getReminder(sourceIndex.row()).id();
        }
    }
    
    // 更新模型
    for (int i = 0; i < model->rowCount(); ++i) {
        Reminder existingReminder = model->getReminder(i);
        if (existingReminder.id() == reminder.id()) {
            model->updateReminder(i, reminder);
            LOG_INFO(QString("提醒更新成功: 名称='%1', 类型=%2")
                    .arg(reminder.name())
                    .arg(static_cast<int>(reminder.type())));
            break;
        }
    }
    
    // 重置代理模型
    proxyModel->invalidate();
    
    // 恢复选择
    if (!currentId.isEmpty()) {
        for (int i = 0; i < model->rowCount(); ++i) {
            if (model->getReminder(i).id() == currentId) {
                QModelIndex newIndex = proxyModel->mapFromSource(model->index(i, 0));
                if (newIndex.isValid()) {
                    ui->tableView->setCurrentIndex(newIndex);
                }
                break;
            }
        }
    }
}

void ActiveReminderList::editReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    Reminder reminder = model->getReminder(sourceIndex.row());
    LOG_INFO(QString("编辑提醒: 名称='%1'").arg(reminder.name()));
    
    editDialog->prepareEditReminder(reminder);
    if (editDialog->exec() == QDialog::Accepted) {
        Reminder updatedReminder = editDialog->getReminder();
        updateReminderInModel(updatedReminder);
        if (reminderManager) {
            reminderManager->updateReminder(updatedReminder);
            LOG_INFO(QString("提醒管理器更新成功: 名称='%1'").arg(updatedReminder.name()));
        }
    } else {
        LOG_INFO("取消编辑提醒");
    }
}

void ActiveReminderList::deleteReminder(const QModelIndex &index)
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


void ActiveReminderList::refreshList()
{
    LOG_INFO(QString("刷新提醒列表，搜索文本: '%1'").arg(m_searchText));
    model->search(m_searchText);
}

void ActiveReminderList::searchReminders(const QString &text)
{
    LOG_INFO(QString("搜索提醒: '%1'").arg(text));
    model->search(text);
}


void ActiveReminderList::onAddClicked()
{
    addNewReminder();
}

void ActiveReminderList::onEditClicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid()) {
        editReminder(currentIndex);
    }
}

void ActiveReminderList::onDeleteClicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid()) {
        deleteReminder(currentIndex);
    }
}

void ActiveReminderList::onImportClicked()
{
    LOG_INFO("开始导入提醒");
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("导入提醒"), "",
        tr("JSON文件 (*.json);;所有文件 (*)"));

    if (fileName.isEmpty()) {
        LOG_INFO("取消导入提醒");
        return;
    }

    LOG_INFO(QString("从文件导入提醒: %1").arg(fileName));
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("无法打开文件: %1, 错误: %2")
                 .arg(fileName)
                 .arg(file.errorString()));
        QMessageBox::warning(this, tr("错误"),
            tr("无法打开文件 %1:\n%2").arg(fileName).arg(file.errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isArray()) {
        QList<Reminder> imported;
        for (const QJsonValue &value : doc.array()) {
            Reminder reminder = Reminder::fromJson(value.toObject());
            imported.append(reminder);
            if (reminderManager) {
                reminderManager->addReminder(reminder);
            }
        }
        LOG_INFO(QString("成功导入 %1 个提醒").arg(imported.size()));
        loadReminders(imported);
    } else {
        LOG_ERROR("导入文件格式错误");
        QMessageBox::warning(this, tr("错误"),
            tr("文件格式错误"));
    }
}

void ActiveReminderList::onExportClicked()
{
    LOG_INFO("开始导出提醒");
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("导出提醒"), "",
        tr("JSON文件 (*.json);;所有文件 (*)"));

    if (fileName.isEmpty()) {
        LOG_INFO("取消导出提醒");
        return;
    }

    LOG_INFO(QString("导出提醒到文件: %1").arg(fileName));
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("无法保存文件: %1, 错误: %2")
                 .arg(fileName)
                 .arg(file.errorString()));
        QMessageBox::warning(this, tr("错误"),
            tr("无法保存文件 %1:\n%2").arg(fileName).arg(file.errorString()));
        return;
    }

    QJsonDocument doc(getReminders());
    file.write(doc.toJson());
    LOG_INFO(QString("成功导出 %1 个提醒").arg(doc.array().size()));
}

void ActiveReminderList::onSearchTextChanged(const QString &text)
{
    LOG_INFO(QString("搜索文本变更: '%1'").arg(text));
    m_searchText = text;
    searchReminders(text);
}

QPushButton *ActiveReminderList::addButton() const
{
    return ui->addButton;
}

QPushButton *ActiveReminderList::deleteButton() const
{
    return ui->deleteButton;
}

QPushButton *ActiveReminderList::importButton() const
{
    return ui->importButton;
}

QPushButton *ActiveReminderList::exportButton() const
{
    return ui->exportButton;
}

QTableView *ActiveReminderList::tableView() const
{
    return ui->tableView;
}
