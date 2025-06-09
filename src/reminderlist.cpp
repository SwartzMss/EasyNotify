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
    , model(new ReminderTableModel(this))
    , proxyModel(new QSortFilterProxyModel(this))
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
                this, &ReminderList::onReminderTriggered, Qt::AutoConnection);
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
}

void ReminderList::loadReminders(const QList<Reminder> &reminders)
{
    model->loadFromJson(reminders);
}

QJsonArray ReminderList::getReminders() const
{
    return model->saveToJson();
}

void ReminderList::addNewReminder()
{
    if (editDialog->exec() == QDialog::Accepted) {
        QJsonObject reminder = editDialog->getReminderData();
        if (reminderManager) {
            Reminder reminderObj = Reminder::fromJson(reminder);
            reminderManager->addReminder(reminderObj);
            addReminderToModel(reminder);
            reminderManager->saveReminders();
        }
    }
}

void ReminderList::addReminderToModel(const QJsonObject &reminder)
{
    Reminder newReminder;
    newReminder.setName(reminder["name"].toString());
    newReminder.setType(static_cast<Reminder::Type>(reminder["type"].toInt()));
    newReminder.setEnabled(reminder["isEnabled"].toBool());
    newReminder.setNextTrigger(QDateTime::fromString(reminder["nextTrigger"].toString(), Qt::ISODate));
    
    QJsonArray weekDaysArray = reminder["weekDays"].toArray();
    QSet<int> weekDays;
    for (const QJsonValue &value : weekDaysArray) {
        weekDays.insert(value.toInt());
    }
    newReminder.setWeekDays(weekDays);
    
    QJsonArray monthDaysArray = reminder["monthDays"].toArray();
    QSet<int> monthDays;
    for (const QJsonValue &value : monthDaysArray) {
        monthDays.insert(value.toInt());
    }
    newReminder.setMonthDays(monthDays);
    
    model->addReminder(newReminder);
}

void ReminderList::updateReminderInModel(const QJsonObject &reminder)
{
    for (int i = 0; i < model->rowCount(); ++i) {
        Reminder existingReminder = model->getReminder(i);
        if (existingReminder.name() == reminder["name"].toString()) {
            Reminder updatedReminder;
            updatedReminder.setName(reminder["name"].toString());
            updatedReminder.setType(static_cast<Reminder::Type>(reminder["type"].toInt()));
            updatedReminder.setEnabled(reminder["isEnabled"].toBool());
            updatedReminder.setNextTrigger(QDateTime::fromString(reminder["nextTrigger"].toString(), Qt::ISODate));
            
            QJsonArray weekDaysArray = reminder["weekDays"].toArray();
            QSet<int> weekDays;
            for (const QJsonValue &value : weekDaysArray) {
                weekDays.insert(value.toInt());
            }
            updatedReminder.setWeekDays(weekDays);
            
            QJsonArray monthDaysArray = reminder["monthDays"].toArray();
            QSet<int> monthDays;
            for (const QJsonValue &value : monthDaysArray) {
                monthDays.insert(value.toInt());
            }
            updatedReminder.setMonthDays(monthDays);
            
            model->updateReminder(i, updatedReminder);
            break;
        }
    }
}

void ReminderList::editReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    Reminder reminder = model->getReminder(sourceIndex.row());
    QJsonObject reminderData = reminder.toJson();
    editDialog->loadReminderData(reminderData);
    if (editDialog->exec() == QDialog::Accepted) {
        QJsonObject updatedData = editDialog->getReminderData();
        updateReminderInModel(updatedData);
        if (reminderManager) {
            Reminder updatedReminder = Reminder::fromJson(updatedData);
            int index = -1;
            for (int i = 0; i < reminderManager->getReminders().size(); ++i) {
                if (reminderManager->getReminders()[i].name() == updatedReminder.name()) {
                    index = i;
                    break;
                }
            }
            if (index != -1) {
                reminderManager->updateReminder(index, updatedReminder);
            }
        }
    }
}

void ReminderList::deleteReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    Reminder reminder = model->getReminder(sourceIndex.row());
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除提醒 '%1' 吗？").arg(reminder.name()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        model->removeReminder(sourceIndex.row());
        if (reminderManager) {
            int index = -1;
            for (int i = 0; i < reminderManager->getReminders().size(); ++i) {
                if (reminderManager->getReminders()[i].name() == reminder.name()) {
                    index = i;
                    break;
                }
            }
            if (index != -1) {
                reminderManager->deleteReminder(index);
            }
        }
    }
}

void ReminderList::toggleReminder(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    model->toggleReminder(sourceIndex.row());
}

void ReminderList::refreshList()
{
    model->search(m_searchText);
}

void ReminderList::searchReminders(const QString &text)
{
    model->search(text);
}

QJsonObject ReminderList::getReminderData(const QString &name) const
{
    for (int i = 0; i < model->rowCount(); ++i) {
        Reminder reminder = model->getReminder(i);
        if (reminder.name() == name) {
            return reminder.toJson();
        }
    }
    return QJsonObject();
}

void ReminderList::onReminderTriggered(const Reminder &reminder)
{
    refreshList();
}

void ReminderList::onAddClicked()
{
    addNewReminder();
}

void ReminderList::onEditClicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid()) {
        editReminder(currentIndex);
    }
}

void ReminderList::onDeleteClicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid()) {
        deleteReminder(currentIndex);
    }
}

void ReminderList::onImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("导入提醒"), "",
        tr("JSON文件 (*.json);;所有文件 (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("错误"),
            tr("无法打开文件 %1:\n%2").arg(fileName).arg(file.errorString()));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isArray()) {
        QList<Reminder> reminders;
        for (const QJsonValue &value : doc.array()) {
            reminders.append(Reminder::fromJson(value.toObject()));
        }
        loadReminders(reminders);
        if (reminderManager) {
            reminderManager->saveReminders();
        }
    } else {
        QMessageBox::warning(this, tr("错误"),
            tr("文件格式错误"));
    }
}

void ReminderList::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("导出提醒"), "",
        tr("JSON文件 (*.json);;所有文件 (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("错误"),
            tr("无法保存文件 %1:\n%2").arg(fileName).arg(file.errorString()));
        return;
    }

    QJsonDocument doc(getReminders());
    file.write(doc.toJson());
}

void ReminderList::onSearchTextChanged(const QString &text)
{
    m_searchText = text;
    searchReminders(text);
} 