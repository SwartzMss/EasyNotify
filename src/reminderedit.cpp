#include "reminderedit.h"
#include "./ui_reminderedit.h"
#include <QMessageBox>
#include <QUuid>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QPushButton>
#include "logger.h"

// 提醒类型枚举
enum class ReminderType {
    OneTime = 0,
    Daily = 1,
    Weekly = 2,
    Monthly = 3
};

ReminderEdit::ReminderEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReminderEdit)
{
    LOG_INFO("创建提醒编辑对话框");
    ui->setupUi(this);
    setWindowTitle(tr("新增提醒"));
    setupConnections();
    // 设置默认值
    QDateTime now = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(now);
    ui->timeEdit->setTime(now.time());
    reminderData["isEnabled"] = true;
    reminderData["type"] = static_cast<int>(ReminderType::OneTime);
    reminderData["nextTrigger"] = now.toString(Qt::ISODate);
    // 初始化控件显示
    onTypeChanged(ui->typeCombo->currentIndex());
    LOG_INFO("提醒编辑对话框初始化完成");
}

ReminderEdit::~ReminderEdit()
{
    LOG_INFO("销毁提醒编辑对话框");
    delete ui;
}

void ReminderEdit::reset()
{
    // 重置所有控件到默认值
    ui->nameEdit->clear();
    QDateTime now = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(now);
    ui->timeEdit->setTime(now.time());
    ui->typeCombo->setCurrentIndex(0);
    
    // 重置reminderData
    reminderData = QJsonObject();
    reminderData["isEnabled"] = true;
    reminderData["type"] = static_cast<int>(ReminderType::OneTime);
    reminderData["nextTrigger"] = now.toString(Qt::ISODate);
    
    onTypeChanged(0);
    LOG_INFO("重置提醒编辑对话框");
}

void ReminderEdit::setupConnections()
{
    connect(ui->typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReminderEdit::onTypeChanged);
    connect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged,
            this, &ReminderEdit::onDateTimeChanged);
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ReminderEdit::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ReminderEdit::onCancelClicked);
}

void ReminderEdit::loadReminderData(const QJsonObject &reminder)
{
    reminderData = reminder;
    QString id = reminder["id"].toString();
    QString name = reminder["name"].toString();
    LOG_INFO(QString("加载提醒数据: ID='%1', 名称='%2'").arg(id).arg(name));
    
    ui->nameEdit->setText(name);
    // 设置类型
    int type = reminder["type"].toInt();
    ui->typeCombo->setCurrentIndex(type);
    // 设置时间
    QDateTime nextTrigger = QDateTime::fromString(reminder["nextTrigger"].toString(), Qt::ISODate);
    ui->dateTimeEdit->setDateTime(nextTrigger);
    // 设置星期
    if (type == static_cast<int>(ReminderType::Weekly)) {
        QJsonArray weekDaysArray = reminder["weekDays"].toArray();
        QList<int> weekDays;
        for (const QJsonValue &value : weekDaysArray) {
            weekDays.append(value.toInt());
        }
        for (int i = 0; i < ui->weekDaysList->count(); ++i) {
            ui->weekDaysList->item(i)->setCheckState(
                weekDays.contains(i + 1) ? Qt::Checked : Qt::Unchecked
            );
        }
        QStringList weekDaysStr;
        for (int day : weekDays) {
            weekDaysStr.append(QString::number(day));
        }
        LOG_INFO(QString("设置每周提醒的星期: %1").arg(weekDaysStr.join(",")));
    }
    // 设置日期
    if (type == static_cast<int>(ReminderType::Monthly)) {
        QJsonArray monthDaysArray = reminder["monthDays"].toArray();
        QList<int> monthDays;
        for (const QJsonValue &value : monthDaysArray) {
            monthDays.append(value.toInt());
        }
        for (int i = 0; i < ui->monthDaysList->count(); ++i) {
            ui->monthDaysList->item(i)->setCheckState(
                monthDays.contains(i + 1) ? Qt::Checked : Qt::Unchecked
            );
        }
        QStringList monthDaysStr;
        for (int day : monthDays) {
            monthDaysStr.append(QString::number(day));
        }
        LOG_INFO(QString("设置每月提醒的日期: %1").arg(monthDaysStr.join(",")));
    }
    LOG_INFO("提醒数据加载完成");
}

QJsonObject ReminderEdit::getReminderData() const
{
    return reminderData;
}

void ReminderEdit::onTypeChanged(int index)
{
    LOG_INFO(QString("提醒类型变更为: %1").arg(index));
    // 先全部隐藏
    ui->dateTimeEdit->hide();
    ui->timeEdit->hide();
    ui->weekDaysList->hide();
    ui->monthDaysList->hide();
    ui->weekDaysLabel->hide();
    ui->monthDaysLabel->hide();
    ui->dateTimeLabel->hide();
    ui->timeLabel->hide();

    switch (index) {
        case 0: // 一次性
            ui->dateTimeEdit->show();
            ui->dateTimeLabel->show();
            reminderData["type"] = static_cast<int>(ReminderType::OneTime);
            break;
        case 1: // 每天
            ui->timeEdit->show();
            ui->timeLabel->show();
            reminderData["type"] = static_cast<int>(ReminderType::Daily);
            break;
        case 2: // 每周
            ui->timeEdit->show();
            ui->timeLabel->show();
            ui->weekDaysList->show();
            ui->weekDaysLabel->show();
            reminderData["type"] = static_cast<int>(ReminderType::Weekly);
            break;
        case 3: // 每月
            ui->timeEdit->show();
            ui->timeLabel->show();
            ui->monthDaysList->show();
            ui->monthDaysLabel->show();
            reminderData["type"] = static_cast<int>(ReminderType::Monthly);
            break;
    }
    updateNextTriggerTime();
}

void ReminderEdit::onDateTimeChanged(const QDateTime &dateTime)
{
    LOG_INFO(QString("日期时间变更为: %1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
    nextTriggerTime = dateTime;
    updateNextTriggerTime();
}

void ReminderEdit::onOkClicked()
{
    LOG_INFO("点击确定按钮");
    if (validateInput()) {
        QString name = ui->nameEdit->text().trimmed();
        reminderData["name"] = name;
        
        // 如果是新建提醒（没有ID），则生成新的ID
        if (!reminderData.contains("id") || reminderData["id"].toString().isEmpty()) {
            QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
            reminderData["id"] = newId;
            LOG_INFO(QString("新建提醒，生成ID: %1").arg(newId));
        }
        
        LOG_INFO(QString("保存提醒: ID='%1', 名称='%2', 类型='%3'")
                 .arg(reminderData["id"].toString())
                 .arg(name)
                 .arg(reminderData["type"].toString()));
        accept();
    } else {
        LOG_WARNING("输入验证失败，无法保存提醒");
    }
}

void ReminderEdit::onCancelClicked()
{
    LOG_INFO("点击取消按钮，关闭对话框");
    reject();
}

void ReminderEdit::onDaysChanged()
{
    LOG_INFO("星期/日期选择发生变化");
    updateNextTriggerTime();
}

QDateTime ReminderEdit::calculateNextTrigger() const
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextTrigger;
    ReminderType type = static_cast<ReminderType>(ui->typeCombo->currentIndex());

    switch (type) {
        case ReminderType::OneTime: {
            nextTrigger = ui->dateTimeEdit->dateTime();
            LOG_INFO(QString("计算一次性提醒时间: %1").arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
            break;
        }
        case ReminderType::Daily: {
            QTime time = ui->timeEdit->time();
            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }
            LOG_INFO(QString("计算每日提醒时间: %1").arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
            break;
        }
        case ReminderType::Weekly: {
            QTime time = ui->timeEdit->time();
            QList<int> weekDays;
            for (int i = 0; i < ui->weekDaysList->count(); ++i) {
                if (ui->weekDaysList->item(i)->checkState() == Qt::Checked) {
                    weekDays << (i + 1);
                }
            }
            
            if (weekDays.isEmpty()) {
                LOG_WARNING("未选择任何星期，返回当前时间");
                return now;
            }

            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }

            while (!weekDays.contains(nextTrigger.date().dayOfWeek())) {
                nextTrigger = nextTrigger.addDays(1);
            }
            QStringList weekDaysStr;
            for (int day : weekDays) {
                weekDaysStr.append(QString::number(day));
            }
            LOG_INFO(QString("计算每周提醒时间: %1, 选中的星期: %2")
                    .arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss"))
                    .arg(weekDaysStr.join(",")));
            break;
        }
        case ReminderType::Monthly: {
            QTime time = ui->timeEdit->time();
            QList<int> monthDays;
            for (int i = 0; i < ui->monthDaysList->count(); ++i) {
                if (ui->monthDaysList->item(i)->checkState() == Qt::Checked) {
                    monthDays << (i + 1);
                }
            }

            if (monthDays.isEmpty()) {
                LOG_WARNING("未选择任何日期，返回当前时间");
                return now;
            }

            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }

            while (!monthDays.contains(nextTrigger.date().day())) {
                nextTrigger = nextTrigger.addDays(1);
                if (nextTrigger.date().day() == 1) {
                    nextTrigger = QDateTime(QDate(nextTrigger.date().year(), 
                                                nextTrigger.date().month(), 
                                                monthDays.first()), 
                                          time);
                }
            }
            QStringList monthDaysStr;
            for (int day : monthDays) {
                monthDaysStr.append(QString::number(day));
            }
            LOG_INFO(QString("计算每月提醒时间: %1, 选中的日期: %2")
                    .arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss"))
                    .arg(monthDaysStr.join(",")));
            break;
        }
    }
    return nextTrigger;
}

bool ReminderEdit::validateInput() const
{
    bool isValid = !ui->nameEdit->text().trimmed().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
    if (!isValid) {
        LOG_WARNING("提醒名称不能为空");
    }
    return isValid;
}

void ReminderEdit::updateNextTriggerTime()
{
    QDateTime nextTime = calculateNextTrigger();
    ui->nextTriggerLabel->setText(nextTime.toString("yyyy-MM-dd HH:mm:ss"));
    reminderData["nextTrigger"] = nextTime.toString(Qt::ISODate);
    LOG_INFO(QString("更新下次触发时间: %1").arg(nextTime.toString("yyyy-MM-dd HH:mm:ss")));
}

bool ReminderEdit::isDaySelected(int day) const
{
    bool selected = false;
    if (ui->typeCombo->currentIndex() == 2) { // 每周
        selected = ui->weekDaysList->item(day - 1)->checkState() == Qt::Checked;
    } else if (ui->typeCombo->currentIndex() == 3) { // 每月
        selected = ui->monthDaysList->item(day - 1)->checkState() == Qt::Checked;
    }
    LOG_INFO(QString("检查日期 %1 是否被选中: %2").arg(day).arg(selected));
    return selected;
} 
