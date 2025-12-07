#include "ui/widgets/active_reminderlist.h"
#include "ui_active_reminderlist.h"
#include <QHeaderView>
#include <QMessageBox>
#include "core/reminders/remindermanager.h"
#include "core/logging/logger.h"
#include "ui/widgets/active_reminderedit.h"

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
    applyWidgetStyles();
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
    proxyModel->setDynamicSortFilter(true); // 启用动态过滤

    // 设置表格视图
    ui->tableView->setModel(proxyModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setShowGrid(false);
    ui->tableView->setMouseTracking(true); // 启用鼠标追踪
    
    // 确保在模型重置时清除选择
    connect(model, &QAbstractItemModel::modelReset, this, [this]() {
        ui->tableView->clearSelection();
        ui->tableView->setCurrentIndex(QModelIndex());
    });
    
    // 确保在布局改变时更新视图
    connect(model, &QAbstractItemModel::layoutChanged, this, [this]() {
        ui->tableView->viewport()->update();
    });
    
    LOG_INFO("数据模型设置完成");
}

void ActiveReminderList::applyWidgetStyles()
{
    ui->searchEdit->setClearButtonEnabled(true);
    ui->searchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "    border: 1px solid #d5deef;"
        "    border-radius: 18px;"
        "    padding: 6px 30px 6px 12px;"
        "    background-color: #ffffff;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #2563eb;"
        "    background-color: #ffffff;"
        "}"
    ));

    const auto styleButton = [](QPushButton *button,
                                const QString &bg,
                                const QString &fg,
                                const QString &border,
                                const QString &hoverBg)
    {
        if (!button)
            return;
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "    background-color: %1;"
            "    color: %2;"
            "    border: 1px solid %3;"
            "    border-radius: 6px;"
            "    padding: 6px 16px;"
            "}"
            "QPushButton:hover {"
            "    background-color: %4;"
            "}"
        ).arg(bg, fg, border, hoverBg));
    };

    styleButton(ui->addButton, "#2563eb", "#ffffff", "#1d4ed8", "#1d4ed8");
    styleButton(ui->deleteButton, "#ffffff", "#111827", "#e5e7eb", "#f3f4f6");

    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setStyleSheet(QStringLiteral(
        "QTableView {"
        "    border: 1px solid #e0e6f4;"
        "    border-radius: 12px;"
        "    background-color: #ffffff;"
        "    gridline-color: transparent;"
        "    selection-background-color: #e3f2ff;"
        "    selection-color: #111827;"
        "}"
        "QTableView::item {"
        "    padding: 6px;"
        "}"
        "QHeaderView::section {"
        "    background-color: #f5f7fb;"
        "    border: none;"
        "    padding: 8px 4px;"
        "    font-weight: 600;"
        "    color: #1f2937;"
        "}"
    ));
}

void ActiveReminderList::loadReminders(const QList<Reminder> &reminders)
{
    LOG_INFO(QString("加载提醒列表，共 %1 个提醒").arg(reminders.size()));
    model->loadFromJson(reminders);
}

void ActiveReminderList::addNewReminder()
{
    LOG_INFO("添加新提醒");
    editDialog->prepareNewReminder();
    if (editDialog->exec() == QDialog::Accepted) {
        Reminder reminder = editDialog->getReminder();
        // 再次验证提醒数据
        if (!reminder.name().isEmpty()) {
            if (reminderManager) {
                reminderManager->addReminder(reminder);
                model->addReminder(reminder);
                LOG_INFO(QString("新提醒添加成功: 名称='%1', ID='%2'")
                        .arg(reminder.name())
                        .arg(reminder.id()));
            }
        } else {
            LOG_WARNING("提醒名称为空，取消添加");
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
    
    // 更新模型前先暂停选择模型的信号
    ui->tableView->selectionModel()->blockSignals(true);
    
    // 更新模型
    for (int i = 0; i < model->rowCount(); ++i) {
        Reminder existingReminder = model->getReminder(i);
        if (existingReminder.id() == reminder.id()) {
            // 在更新数据之前先清除选择
            ui->tableView->clearSelection();
            
            model->updateReminder(i, reminder);
            LOG_INFO(QString("提醒更新成功: 名称='%1', 类型=%2")
                    .arg(reminder.name())
                    .arg(static_cast<int>(reminder.type())));
            
            // 发出数据改变信号，而不是重置整个模型
            QModelIndex topLeft = model->index(i, 0);
            QModelIndex bottomRight = model->index(i, model->columnCount() - 1);
            emit model->dataChanged(topLeft, bottomRight);
            
            // 恢复选择
            if (!currentId.isEmpty()) {
                QModelIndex newSourceIndex = model->index(i, 0);
                QModelIndex newProxyIndex = proxyModel->mapFromSource(newSourceIndex);
                if (newProxyIndex.isValid()) {
                    ui->tableView->setCurrentIndex(newProxyIndex);
                    ui->tableView->selectionModel()->select(
                        newProxyIndex,
                        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
                    );
                }
            }
            break;
        }
    }
    
    // 恢复选择模型的信号
    ui->tableView->selectionModel()->blockSignals(false);
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
    if (!currentIndex.isValid()) {
        return;
    }
    
    // 在编辑之前暂时禁用双击
    ui->tableView->setEnabled(false);
    
    editReminder(currentIndex);
    
    // 编辑完成后重新启用
    ui->tableView->setEnabled(true);
}

void ActiveReminderList::onDeleteClicked()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if (currentIndex.isValid()) {
        deleteReminder(currentIndex);
    }
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

QTableView *ActiveReminderList::tableView() const
{
    return ui->tableView;
}
