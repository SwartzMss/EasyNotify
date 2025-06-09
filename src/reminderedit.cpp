#include "reminderedit.h"
#include "./ui_reminderedit.h"
#include <QMessageBox>
#include <QUuid>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QPushButton>

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
    ui->setupUi(this);
    setWindowTitle(tr("新增提醒"));
    setupConnections();
    // 设置默认值
    QDateTime now = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(now);
    ui->timeEdit->setTime(now.time());
    reminderData["isEnabled"] = true;
    reminderData["type"] = "OneTime";
    reminderData["nextTrigger"] = now.toString(Qt::ISODate);
    // 初始化控件显示
    onTypeChanged(ui->typeCombo->currentIndex());
}

ReminderEdit::~ReminderEdit()
{
    delete ui;
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
    setWindowTitle(tr("编辑提醒"));
    reminderData = reminder;
    ui->nameEdit->setText(reminder["name"].toString());
    // 设置类型
    QString type = reminder["type"].toString();
    ReminderType typeIndex = ReminderType::OneTime;
    if (type == "Daily") typeIndex = ReminderType::Daily;
    else if (type == "Weekly") typeIndex = ReminderType::Weekly;
    else if (type == "Monthly") typeIndex = ReminderType::Monthly;
    ui->typeCombo->setCurrentIndex(static_cast<int>(typeIndex));
    // 设置时间
    QDateTime nextTrigger = QDateTime::fromString(reminder["nextTrigger"].toString(), Qt::ISODate);
    ui->dateTimeEdit->setDateTime(nextTrigger);
    // 设置星期
    if (type == "Weekly") {
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
    }
    // 设置日期
    if (type == "Monthly") {
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
    }
}

QJsonObject ReminderEdit::getReminderData() const
{
    return reminderData;
}

void ReminderEdit::onTypeChanged(int index)
{
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
            reminderData["type"] = "OneTime";
            break;
        case 1: // 每天
            ui->timeEdit->show();
            ui->timeLabel->show();
            reminderData["type"] = "Daily";
            break;
        case 2: // 每周
            ui->timeEdit->show();
            ui->timeLabel->show();
            ui->weekDaysList->show();
            ui->weekDaysLabel->show();
            reminderData["type"] = "Weekly";
            break;
        case 3: // 每月
            ui->timeEdit->show();
            ui->timeLabel->show();
            ui->monthDaysList->show();
            ui->monthDaysLabel->show();
            reminderData["type"] = "Monthly";
            break;
    }
    updateNextTriggerTime();
}

void ReminderEdit::onDateTimeChanged(const QDateTime &dateTime)
{
    nextTriggerTime = dateTime;
    updateNextTriggerTime();
}


void ReminderEdit::onOkClicked()
{
    if (validateInput()) {
        reminderData["name"] = ui->nameEdit->text().trimmed();
        accept();
    }
}

void ReminderEdit::onCancelClicked()
{
    reject();
}

void ReminderEdit::onDaysChanged()
{
    updateNextTriggerTime();
}

QDateTime ReminderEdit::calculateNextTrigger() const
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextTrigger;
    ReminderType type = static_cast<ReminderType>(ui->typeCombo->currentIndex());

    switch (type) {
        case ReminderType::OneTime: {
            // 一次性提醒直接使用日期时间选择器的值
            nextTrigger = ui->dateTimeEdit->dateTime();
            break;
        }
        case ReminderType::Daily: {
            // 每日提醒，使用时间选择器的值
            QTime time = ui->timeEdit->time();
            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }
            break;
        }
        case ReminderType::Weekly: {
            // 每周提醒，需要考虑选中的星期
            QTime time = ui->timeEdit->time();
            QList<int> weekDays;
            for (int i = 0; i < ui->weekDaysList->count(); ++i) {
                if (ui->weekDaysList->item(i)->checkState() == Qt::Checked) {
                    weekDays << (i + 1);
                }
            }
            
            if (weekDays.isEmpty()) {
                // 如果没有选择任何星期，返回当前时间
                return now;
            }

            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }

            // 找到下一个符合条件的日期
            while (!weekDays.contains(nextTrigger.date().dayOfWeek())) {
                nextTrigger = nextTrigger.addDays(1);
            }
            break;
        }
        case ReminderType::Monthly: {
            // 每月提醒，需要考虑选中的日期
            QTime time = ui->timeEdit->time();
            QList<int> monthDays;
            for (int i = 0; i < ui->monthDaysList->count(); ++i) {
                if (ui->monthDaysList->item(i)->checkState() == Qt::Checked) {
                    monthDays << (i + 1);
                }
            }

            if (monthDays.isEmpty()) {
                // 如果没有选择任何日期，返回当前时间
                return now;
            }

            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }

            // 找到下一个符合条件的日期
            while (!monthDays.contains(nextTrigger.date().day())) {
                nextTrigger = nextTrigger.addDays(1);
                // 如果已经到下个月，重置到月初
                if (nextTrigger.date().day() == 1) {
                    nextTrigger = QDateTime(QDate(nextTrigger.date().year(), 
                                                nextTrigger.date().month(), 
                                                monthDays.first()), 
                                          time);
                }
            }
            break;
        }
    }
    return nextTrigger;
}

bool ReminderEdit::validateInput() const
{
    bool isValid = !ui->nameEdit->text().trimmed().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
    return isValid;
}

void ReminderEdit::updateNextTriggerTime()
{
    // 直接使用calculateNextTrigger计算下一个触发时间
    QDateTime nextTime = calculateNextTrigger();
    
    // 更新显示
    ui->nextTriggerLabel->setText(nextTime.toString("yyyy-MM-dd HH:mm:ss"));
    
    // 更新数据
    reminderData["nextTrigger"] = nextTime.toString(Qt::ISODate);
}

bool ReminderEdit::isDaySelected(int day) const
{
    if (ui->typeCombo->currentIndex() == 2) { // 每周
        return ui->weekDaysList->item(day - 1)->checkState() == Qt::Checked;
    } else if (ui->typeCombo->currentIndex() == 3) { // 每月
        return ui->monthDaysList->item(day - 1)->checkState() == Qt::Checked;
    }
    return false;
} 
