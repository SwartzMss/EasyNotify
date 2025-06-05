#include "reminderedit.h"
#include "./ui_reminderedit.h"
#include <QMessageBox>
#include <QUuid>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QPushButton>

QStringList ReminderEdit::categories;
const QString ReminderEdit::CATEGORIES_FILE = "categories.json";

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
    reminderData["Id"] = generateId();
    reminderData["IsEnabled"] = true;
    reminderData["Type"] = "OneTime";
    reminderData["NextTrigger"] = now.toString(Qt::ISODate);
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
    ui->nameEdit->setText(reminder["Name"].toString());
    // 设置类型
    QString type = reminder["Type"].toString();
    int typeIndex = 0;
    if (type == "Daily") typeIndex = 1;
    else if (type == "Weekly") typeIndex = 2;
    else if (type == "Monthly") typeIndex = 3;
    ui->typeCombo->setCurrentIndex(typeIndex);
    // 设置时间
    QDateTime nextTrigger = QDateTime::fromString(reminder["NextTrigger"].toString(), Qt::ISODate);
    ui->dateTimeEdit->setDateTime(nextTrigger);
    // 设置星期
    if (type == "Weekly") {
        QJsonArray weekDaysArray = reminder["WeekDays"].toArray();
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
        QJsonArray monthDaysArray = reminder["MonthDays"].toArray();
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
            reminderData["Type"] = "OneTime";
            break;
        case 1: // 每天
            ui->timeEdit->show();
            ui->timeLabel->show();
            reminderData["Type"] = "Daily";
            break;
        case 2: // 工作日
            ui->timeEdit->show();
            ui->timeLabel->show();
            reminderData["Type"] = "Workday";
            break;
        case 3: // 每周
            ui->timeEdit->show();
            ui->timeLabel->show();
            ui->weekDaysList->show();
            ui->weekDaysLabel->show();
            reminderData["Type"] = "Weekly";
            break;
        case 4: // 每月
            ui->timeEdit->show();
            ui->timeLabel->show();
            ui->monthDaysList->show();
            ui->monthDaysLabel->show();
            reminderData["Type"] = "Monthly";
            break;
    }
    updateNextTriggerTime();
}

void ReminderEdit::onDateTimeChanged(const QDateTime &dateTime)
{
    nextTriggerTime = dateTime;
    updateNextTriggerTime();
}

void ReminderEdit::onTimeChanged(const QTime &time)
{
    QDateTime currentDateTime = nextTriggerTime;
    currentDateTime.setTime(time);
    nextTriggerTime = currentDateTime;
    updateNextTriggerTime();
}

void ReminderEdit::onDateChanged(const QDate &date)
{
    QDateTime currentDateTime = nextTriggerTime;
    currentDateTime.setDate(date);
    nextTriggerTime = currentDateTime;
    updateNextTriggerTime();
}

void ReminderEdit::onOkClicked()
{
    if (validateInput()) {
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

QString ReminderEdit::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QDateTime ReminderEdit::calculateNextTrigger() const
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextTrigger;
    switch (ui->typeCombo->currentIndex()) {
        case 0: // 一次性提醒
            nextTrigger = ui->dateTimeEdit->dateTime();
            break;
        case 1: { // 每日提醒
            QTime time = ui->dateTimeEdit->time();
            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }
            break;
        }
        case 2: { // 每周提醒
            QTime time = ui->dateTimeEdit->time();
            QList<int> weekDays;
            for (int i = 0; i < ui->weekDaysList->count(); ++i) {
                if (ui->weekDaysList->item(i)->checkState() == Qt::Checked) {
                    weekDays << (i + 1);
                }
            }
            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }
            while (!weekDays.contains(nextTrigger.date().dayOfWeek())) {
                nextTrigger = nextTrigger.addDays(1);
            }
            break;
        }
        case 3: { // 每月提醒
            QTime time = ui->dateTimeEdit->time();
            QList<int> monthDays;
            for (int i = 0; i < ui->monthDaysList->count(); ++i) {
                if (ui->monthDaysList->item(i)->checkState() == Qt::Checked) {
                    monthDays << (i + 1);
                }
            }
            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }
            while (!monthDays.contains(nextTrigger.date().day())) {
                nextTrigger = nextTrigger.addDays(1);
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
    QString type = ui->typeCombo->currentText();
    QDateTime nextTime = nextTriggerTime;
    if (type == tr("每天")) {
        nextTime = nextTime.addDays(1);
    } else if (type == tr("每周")) {
        nextTime = nextTime.addDays(7);
    } else if (type == tr("每月")) {
        nextTime = nextTime.addMonths(1);
    }
    ui->nextTriggerLabel->setText(nextTime.toString("yyyy-MM-dd HH:mm:ss"));
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