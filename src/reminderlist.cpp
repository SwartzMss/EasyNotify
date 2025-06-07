#include "reminderlist.h"
#include "./ui_reminderlist.h"
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
#include <QCoreApplication>

enum ColumnIndex {
    Name = 0,
    Type = 1,
    Status = 2,
    NextTrigger = 3
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

void ReminderList::setupConnections()
{
    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &ReminderList::onSearchTextChanged);
    connect(ui->addButton, &QPushButton::clicked,
            this, &ReminderList::onAddClicked);
    connect(ui->deleteButton, &QPushButton::clicked,
            this, &ReminderList::onDeleteClicked);
    connect(ui->importButton, &QPushButton::clicked,
            this, &ReminderList::onImportClicked);
    connect(ui->exportButton, &QPushButton::clicked,
            this, &ReminderList::onExportClicked);
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &ReminderList::onEditClicked);
}

void ReminderList::setupModel()
{
    // 创建数据模型
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({
        tr("任务名称"),
        tr("类型"),
        tr("状态"),
        tr("下次触发")
    });

    // 创建代理模型
    proxyModel = new QSortFilterProxyModel(this);
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
        if (reminder.contains("Name") && !reminder["Name"].toString().isEmpty()) {
            // 确保提醒对象包含所有必要字段
            reminder["id"] = reminder["Name"].toString(); // 使用名称作为ID
            reminder["title"] = reminder["Name"].toString();
            reminder["message"] = QCoreApplication::translate("ReminderManager", "提醒时间到了！");
            reminder["type"] = reminder["Type"].toString();
            reminder["nextTrigger"] = reminder["NextTrigger"].toString();
            reminder["isEnabled"] = true;

            reminderManager->addReminder(reminder);
            LOG_INFO(QString("添加提醒: %1, 类型: %2, 下次触发时间: %3")
                .arg(reminder["id"].toString())
                .arg(reminder["type"].toString())
                .arg(reminder["nextTrigger"].toString()));
            loadReminders();
        } else {
            LOG_WARNING("尝试添加没有名称的提醒");
            QMessageBox::warning(this, tr("警告"), tr("提醒名称不能为空"));
        }
    }
}

void ReminderList::editReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString name = model->data(model->index(sourceIndex.row(), Name)).toString();
    editDialog->loadReminderData(getReminderData(name));
    if (editDialog->exec() == QDialog::Accepted) {
        QJsonObject reminder = editDialog->getReminderData();
        updateReminderInModel(reminder);
        if (reminderManager) {
            reminderManager->updateReminder(name, reminder);
        }
    }
}

void ReminderList::onEditClicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        LOG_WARNING("尝试编辑未选中的提醒");
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个提醒"));
        return;
    }

    // 获取源模型中的索引
    QModelIndex sourceIndex = proxyModel->mapToSource(currentIndex);
    // 获取名称列的数据
    QString name = model->data(model->index(sourceIndex.row(), Name)).toString();
    
    QJsonObject reminder = getReminderData(name);
    if (reminder.isEmpty()) {
        LOG_ERROR(QString("无法获取提醒数据: %1").arg(name));
        return;
    }

    ReminderEdit dialog(this);
    dialog.loadReminderData(reminder);
    if (dialog.exec() == QDialog::Accepted) {
        reminder = dialog.getReminderData();
        reminderManager->updateReminder(name, reminder);
        LOG_INFO(QString("更新提醒: %1").arg(reminder["Name"].toString()));
        loadReminders();
        reminderManager->saveReminders();
    }
}

void ReminderList::deleteReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString name = model->data(model->index(sourceIndex.row(), Name)).toString();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除提醒\"%1\"吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        model->removeRow(sourceIndex.row());
        if (reminderManager) {
            reminderManager->deleteReminder(name);
        }
    }
}

void ReminderList::toggleReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    bool currentStatus = model->data(model->index(sourceIndex.row(), Status)).toBool();
    QString name = model->data(model->index(sourceIndex.row(), Name)).toString();
    
    updateReminderStatus(sourceIndex, !currentStatus);
    
    QJsonObject reminder = getReminderData(name);
    reminder["IsEnabled"] = !currentStatus;
    if (reminderManager) {
        reminderManager->updateReminder(name, reminder);
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
    QString name = reminder["Name"].toString();
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->data(model->index(row, Name)).toString() == name) {
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

    QJsonArray reminders;
    for (int row = 0; row < model->rowCount(); ++row) {
        QJsonObject reminder;
        reminder["Name"] = model->data(model->index(row, Name)).toString();
        reminder["Type"] = model->data(model->index(row, Type)).toString();
        reminder["NextTrigger"] = model->data(model->index(row, NextTrigger)).toString();
        reminder["IsEnabled"] = model->data(model->index(row, Status)).toBool();
        
        // 添加额外的时间信息
        if (reminder["Type"] == "Weekly") {
            QJsonArray weekDays;
            QJsonObject reminderData = getReminderData(reminder["Name"].toString());
            if (reminderData.contains("WeekDays")) {
                weekDays = reminderData["WeekDays"].toArray();
            }
            reminder["WeekDays"] = weekDays;
        } else if (reminder["Type"] == "Monthly") {
            QJsonArray monthDays;
            QJsonObject reminderData = getReminderData(reminder["Name"].toString());
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
        reminder["Name"] = model->data(model->index(row, Name)).toString();
        reminder["Type"] = model->data(model->index(row, Type)).toString();
        reminder["NextTrigger"] = model->data(model->index(row, NextTrigger)).toString();
        reminder["IsEnabled"] = model->data(model->index(row, Status)).toBool();
        
        // 添加额外的时间信息
        if (reminder["Type"] == "Weekly") {
            QJsonArray weekDays;
            QJsonObject reminderData = getReminderData(reminder["Name"].toString());
            if (reminderData.contains("WeekDays")) {
                weekDays = reminderData["WeekDays"].toArray();
            }
            reminder["WeekDays"] = weekDays;
        } else if (reminder["Type"] == "Monthly") {
            QJsonArray monthDays;
            QJsonObject reminderData = getReminderData(reminder["Name"].toString());
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
    QString importedName = importedReminder["Name"].toString();
    bool hasConflict = false;
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->data(model->index(row, Name)).toString() == importedName) {
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
                reminderManager->updateReminder(importedName, importedReminder);
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
    row << new QStandardItem(reminder["Name"].toString());
    
    // 转换类型为中文
    QString type = reminder["Type"].toString();
    QString typeText;
    if (type == "OneTime") typeText = tr("一次性");
    else if (type == "Daily") typeText = tr("每天");
    else if (type == "Workday") typeText = tr("工作日");
    else if (type == "Weekly") typeText = tr("每周");
    else if (type == "Monthly") typeText = tr("每月");
    else typeText = type;
    
    row << new QStandardItem(typeText);
    row << new QStandardItem(reminder["IsEnabled"].toBool() ? tr("启用") : tr("禁用"));
    row << new QStandardItem(reminder["NextTrigger"].toString());
    
    model->appendRow(row);
}

void ReminderList::updateReminderInModel(const QJsonObject &reminder)
{
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->data(model->index(row, Name)).toString() == reminder["Name"].toString()) {
            model->setData(model->index(row, Name), reminder["Name"].toString());
            
            // 转换类型为中文
            QString type = reminder["Type"].toString();
            QString typeText;
            if (type == "OneTime") typeText = tr("一次性");
            else if (type == "Daily") typeText = tr("每天");
            else if (type == "Workday") typeText = tr("工作日");
            else if (type == "Weekly") typeText = tr("每周");
            else if (type == "Monthly") typeText = tr("每月");
            else typeText = type;
            
            model->setData(model->index(row, Type), typeText);
            model->setData(model->index(row, Status), reminder["IsEnabled"].toBool());
            model->setData(model->index(row, Status), 
                          reminder["IsEnabled"].toBool() ? tr("启用") : tr("禁用"), Qt::DisplayRole);
            model->setData(model->index(row, NextTrigger), reminder["NextTrigger"].toString());
            break;
        }
    }
}

QJsonObject ReminderList::getReminderData(const QString &name) const
{
    QJsonObject reminder;
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->data(model->index(row, Name)).toString() == name) {
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
    // 设置过滤规则
    proxyModel->setFilterFixedString(text);
    LOG_DEBUG(QString("搜索提醒，关键字：%1").arg(text));
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
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid()) {
        LOG_WARNING("尝试删除未选中的提醒");
        QMessageBox::warning(this, tr("警告"), tr("请先选择一个提醒"));
        return;
    }

    // 获取源模型中的索引
    QModelIndex sourceIndex = proxyModel->mapToSource(currentIndex);
    // 获取名称列的数据
    QString name = model->data(model->index(sourceIndex.row(), Name)).toString();
    
    QJsonObject reminder = getReminderData(name);
    if (reminder.isEmpty()) {
        LOG_ERROR(QString("无法获取提醒数据: %1").arg(name));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除提醒\"%1\"吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        reminderManager->deleteReminder(name);
        LOG_INFO(QString("删除提醒: %1").arg(name));
        loadReminders();
    }
} 