#include "reminderlist.h"
#include "./ui_reminderlist.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDateTime>
#include <QFileDialog>
#include "remindermanager.h"
#include "logger.h"
#include "reminderedit.h"
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

enum ColumnIndex {
    Id = 0,
    Name = 1,
    Type = 2,
    NextTrigger = 3,
    Status = 4,
    Action = 5
};

ReminderList::ReminderList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReminderList)
    , reminderManager(nullptr)
    , model(nullptr)
    , proxyModel(nullptr)
    , editDialog(new ReminderEdit(this))
{
    ui->setupUi(this);
    setupUI();
    setupModel();
    setupConnections();
}

ReminderList::~ReminderList()
{
    delete ui;
}

void ReminderList::setReminderManager(ReminderManager *manager)
{
    reminderManager = manager;
    if (reminderManager) {
        connect(reminderManager, &ReminderManager::reminderTriggered,
                this, &ReminderList::onReminderTriggered);
        loadReminders(reminderManager->getReminders());
    }
}

void ReminderList::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建顶部工具栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText(tr("搜索提醒..."));
    searchEdit->setMinimumWidth(200);
    
    addButton = new QPushButton(tr("添加提醒"), this);
    importButton = new QPushButton(tr("导入"), this);
    exportButton = new QPushButton(tr("导出"), this);
    
    toolbarLayout->addWidget(searchEdit);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(addButton);
    toolbarLayout->addWidget(importButton);
    toolbarLayout->addWidget(exportButton);
    
    mainLayout->addLayout(toolbarLayout);
    
    // 创建表格视图
    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->verticalHeader()->hide();
    
    // 创建数据模型
    model->setHorizontalHeaderLabels({
        tr("ID"),
        tr("任务名称"),
        tr("类型"),
        tr("下次触发"),
        tr("状态"),
        tr("操作")
    });
    
    tableView->setModel(model);
    
    mainLayout->addWidget(tableView);
    
    setLayout(mainLayout);
}

void ReminderList::setupConnections()
{
    connect(searchEdit, &QLineEdit::textChanged,
            this, &ReminderList::onSearchTextChanged);
    connect(addButton, &QPushButton::clicked,
            this, &ReminderList::onAddClicked);
    connect(importButton, &QPushButton::clicked,
            this, &ReminderList::onImportClicked);
    connect(exportButton, &QPushButton::clicked,
            this, &ReminderList::onExportClicked);
    connect(tableView, &QTableView::doubleClicked,
            this, &ReminderList::onEditClicked);
}

void ReminderList::addNewReminder()
{
    if (editDialog->exec() == QDialog::Accepted) {
        QJsonObject reminder = editDialog->getReminderData();
        if (reminderManager) {
            reminderManager->addReminder(reminder);
            addReminderToModel(reminder);
        }
    }
}

void ReminderList::onAddClicked()
{
    ReminderEdit dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject reminder = dialog.getReminderData();
        reminderManager->addReminder(reminder);
        LOG_INFO(QString("添加提醒: %1").arg(reminder["Name"].toString()));
        loadReminders();
    }
}

void ReminderList::editReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString id = model->data(model->index(sourceIndex.row(), Id)).toString();
    editDialog->loadReminderData(getReminderData(id));
    if (editDialog->exec() == QDialog::Accepted) {
        QJsonObject reminder = editDialog->getReminderData();
        updateReminderInModel(reminder);
        if (reminderManager) {
            reminderManager->updateReminder(id, reminder);
        }
    }
}

void ReminderList::onEditClicked()
{
    QModelIndex currentIndex = tableView->currentIndex();
    if (!currentIndex.isValid()) {
        LOG_WARNING("尝试编辑未选中的提醒");
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个提醒"));
        return;
    }

    QString id = currentIndex.data(Qt::UserRole).toString();
    QJsonObject reminder = getReminderData(id);
    if (reminder.isEmpty()) {
        LOG_ERROR(QString("无法获取提醒数据: %1").arg(id));
        return;
    }

    ReminderEdit dialog(this);
    dialog.loadReminderData(reminder);
    if (dialog.exec() == QDialog::Accepted) {
        reminder = dialog.getReminderData();
        reminderManager->updateReminder(id, reminder);
        LOG_INFO(QString("更新提醒: %1").arg(reminder["Name"].toString()));
        loadReminders();
    }
}

void ReminderList::deleteReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString name = model->data(model->index(sourceIndex.row(), Name)).toString();
    QString id = model->data(model->index(sourceIndex.row(), Id)).toString();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除提醒\"%1\"吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        model->removeRow(sourceIndex.row());
        if (reminderManager) {
            reminderManager->deleteReminder(id);
        }
    }
}

void ReminderList::toggleReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    bool currentStatus = model->data(model->index(sourceIndex.row(), Status)).toBool();
    QString id = model->data(model->index(sourceIndex.row(), Id)).toString();
    QString name = model->data(model->index(sourceIndex.row(), Name)).toString();
    
    updateReminderStatus(sourceIndex, !currentStatus);
    
    QJsonObject reminder = getReminderData(id);
    reminder["IsEnabled"] = !currentStatus;
    if (reminderManager) {
        reminderManager->updateReminder(reminder["Id"].toString(), reminder);
    }
    
    saveReminders();
    QString status = !currentStatus ? "启用" : "禁用";
    QString message = QString("用户%1提醒：%2").arg(status).arg(name);
    LOG_INFO(message);
}

void ReminderList::refreshList()
{
    loadReminders();
}

void ReminderList::searchReminders(const QString &text)
{
    proxyModel->setFilterFixedString(text);
    LOG_DEBUG(QString("搜索提醒，关键字：%1").arg(text));
}

void ReminderList::onReminderTriggered(const QJsonObject &reminder)
{
    // 更新列表中的提醒状态
    QString id = reminder["Id"].toString();
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->data(model->index(row, Id)).toString() == id) {
            updateReminderInModel(reminder);
            break;
        }
    }
}

void ReminderList::onImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("导入提醒"), QString(), tr("JSON 文件 (*.json)"));
    
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("无法打开文件: %1").arg(fileName));
        QMessageBox::critical(
            this,
            tr("错误"),
            tr("无法打开文件")
        );
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isArray()) {
        LOG_ERROR(QString("无效的JSON格式: %1").arg(fileName));
        QMessageBox::critical(
            this,
            tr("错误"),
            tr("无效的JSON格式")
        );
        return;
    }

    QJsonArray reminders = doc.array();
    for (const QJsonValue &value : reminders) {
        if (value.isObject()) {
            QJsonObject reminder = value.toObject();
            reminderManager->addReminder(reminder);
        }
    }

    LOG_INFO(QString("从 %1 导入 %2 个提醒").arg(fileName).arg(reminders.size()));
    loadReminders();
    QMessageBox::information(
        this,
        tr("成功"),
        tr("导入完成")
    );
}

void ReminderList::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("导出提醒"), QString(), tr("JSON 文件 (*.json)"));
    
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QString("无法创建文件: %1").arg(fileName));
        QMessageBox::critical(this, tr("错误"), tr("无法创建文件"));
        return;
    }

    QJsonArray reminders = reminderManager->getReminders();
    QJsonDocument doc(reminders);
    file.write(doc.toJson());

    LOG_INFO(QString("导出 %1 个提醒到 %2").arg(reminders.size()).arg(fileName));
    QMessageBox::information(this, tr("成功"), tr("导出完成"));
}

bool ReminderList::importReminders(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(
            this,
            tr("导入失败"),
            tr("无法打开文件：%1").arg(fileName)
        );
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isArray()) {
        QMessageBox::warning(
            this,
            tr("导入失败"),
            tr("文件格式不正确")
        );
        return false;
    }
    
    QJsonArray reminders = doc.array();
    for (const QJsonValue &value : reminders) {
        QJsonObject reminder = value.toObject();
        handleImportConflict(reminder);
    }
    
    return true;
}

bool ReminderList::exportReminders(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(
            this,
            tr("导出失败"),
            tr("无法创建文件：%1").arg(fileName)
        );
        return false;
    }
    
    QJsonArray reminders;
    for (int row = 0; row < model->rowCount(); ++row) {
        QJsonObject reminder;
        reminder["Id"] = model->data(model->index(row, Id)).toString();
        reminder["Name"] = model->data(model->index(row, Name)).toString();
        reminder["Type"] = model->data(model->index(row, Type)).toString();
        reminder["NextTrigger"] = model->data(model->index(row, NextTrigger)).toString();
        reminder["IsEnabled"] = model->data(model->index(row, Status)).toBool();
        
        // 添加额外的时间信息
        if (reminder["Type"] == "Weekly") {
            QJsonArray weekDays;
            QJsonObject reminderData = getReminderData(reminder["Id"].toString());
            if (reminderData.contains("WeekDays")) {
                weekDays = reminderData["WeekDays"].toArray();
            }
            reminder["WeekDays"] = weekDays;
        } else if (reminder["Type"] == "Monthly") {
            QJsonArray monthDays;
            QJsonObject reminderData = getReminderData(reminder["Id"].toString());
            if (reminderData.contains("MonthDays")) {
                monthDays = reminderData["MonthDays"].toArray();
            }
            reminder["MonthDays"] = monthDays;
        }
        
        reminders.append(reminder);
    }
    
    QJsonDocument doc(reminders);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

void ReminderList::handleImportConflict(const QJsonObject &importedReminder)
{
    QString importedId = importedReminder["Id"].toString();
    QString importedName = importedReminder["Name"].toString();
    bool hasConflict = false;
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->data(model->index(row, Id)).toString() == importedId) {
            hasConflict = true;
            break;
        }
    }
    if (hasConflict) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("导入冲突"),
            QString(tr("发现重复的提醒\"%1\"，是否覆盖？")).arg(importedName),
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::Yes) {
            updateReminderInModel(importedReminder);
            if (reminderManager) {
                reminderManager->updateReminder(importedId, importedReminder);
            }
        }
    } else {
        addReminderToModel(importedReminder);
        if (reminderManager) {
            reminderManager->addReminder(importedReminder);
        }
    }
}

void ReminderList::loadReminders(const QJsonArray &reminders)
{
    model->removeRows(0, model->rowCount());
    for (const QJsonValue &value : reminders) {
        addReminderToModel(value.toObject());
    }
}

void ReminderList::saveReminders()
{
    if (!reminderManager) {
        return;
    }

    QJsonArray reminders;
    for (int row = 0; row < model->rowCount(); ++row) {
        QJsonObject reminder;
        reminder["Id"] = model->data(model->index(row, Id)).toString();
        reminder["Name"] = model->data(model->index(row, Name)).toString();
        reminder["Type"] = model->data(model->index(row, Type)).toString();
        reminder["NextTrigger"] = model->data(model->index(row, NextTrigger)).toString();
        reminder["IsEnabled"] = model->data(model->index(row, Status)).toBool();
        
        reminders.append(reminder);
    }
    
    reminderManager->saveReminders();
}

void ReminderList::updateReminderStatus(const QModelIndex &index, bool enabled)
{
    model->setData(model->index(index.row(), Status), enabled);
    model->setData(model->index(index.row(), Status), 
                  enabled ? tr("启用") : tr("禁用"), Qt::DisplayRole);
}

void ReminderList::addReminderToModel(const QJsonObject &reminder)
{
    QList<QStandardItem*> row;
    row << new QStandardItem(reminder["Id"].toString());
    row << new QStandardItem(reminder["Name"].toString());
    row << new QStandardItem(reminder["Type"].toString());
    row << new QStandardItem(reminder["NextTrigger"].toString());
    row << new QStandardItem(reminder["IsEnabled"].toBool() ? tr("启用") : tr("禁用"));
    row << new QStandardItem(tr("编辑"));
    
    model->appendRow(row);
}

void ReminderList::updateReminderInModel(const QJsonObject &reminder)
{
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->data(model->index(row, Id)).toString() == reminder["Id"].toString()) {
            model->setData(model->index(row, Name), reminder["Name"].toString());
            model->setData(model->index(row, Type), reminder["Type"].toString());
            model->setData(model->index(row, NextTrigger), reminder["NextTrigger"].toString());
            model->setData(model->index(row, Status), reminder["IsEnabled"].toBool());
            model->setData(model->index(row, Status), 
                          reminder["IsEnabled"].toBool() ? tr("启用") : tr("禁用"), Qt::DisplayRole);
            break;
        }
    }
}

QJsonObject ReminderList::getReminderData(const QString &id) const
{
    QJsonObject reminder;
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->data(model->index(row, Id)).toString() == id) {
            reminder["Id"] = model->data(model->index(row, Id)).toString();
            reminder["Name"] = model->data(model->index(row, Name)).toString();
            reminder["Type"] = model->data(model->index(row, Type)).toString();
            reminder["NextTrigger"] = model->data(model->index(row, NextTrigger)).toString();
            reminder["IsEnabled"] = model->data(model->index(row, Status)).toBool();
            break;
        }
    }
    return reminder;
}

void ReminderList::onSearchTextChanged(const QString &text)
{
    proxyModel->setFilterFixedString(text);
}

void ReminderList::setupModel()
{
    // 创建数据模型
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({
        tr("ID"),
        tr("任务名称"),
        tr("类型"),
        tr("下次触发"),
        tr("状态"),
        tr("操作")
    });

    // 创建代理模型
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterKeyColumn(1); // 按名称列过滤

    // 设置表格视图
    tableView->setModel(proxyModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->verticalHeader()->hide();
}

void ReminderList::loadReminders()
{
    if (!reminderManager) {
        return;
    }

    QJsonArray reminders = reminderManager->getReminders();
    model->removeRows(0, model->rowCount());
    for (const QJsonValue &value : reminders) {
        addReminderToModel(value.toObject());
    }
}

void ReminderList::onDeleteClicked()
{
    QModelIndex currentIndex = tableView->currentIndex();
    if (!currentIndex.isValid()) {
        LOG_WARNING("尝试删除未选中的提醒");
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个提醒"));
        return;
    }

    QString id = currentIndex.data(Qt::UserRole).toString();
    QJsonObject reminder = getReminderData(id);
    if (reminder.isEmpty()) {
        LOG_ERROR(QString("无法获取提醒数据: %1").arg(id));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除提醒\"%1\"吗？").arg(reminder["Name"].toString()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        reminderManager->deleteReminder(id);
        LOG_INFO(QString("删除提醒: %1").arg(reminder["Name"].toString()));
        loadReminders();
    }
} 