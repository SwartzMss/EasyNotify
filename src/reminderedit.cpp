#include "reminderedit.h"
#include "ui_reminderedit.h"
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
    Daily = 1
};

ReminderEdit::ReminderEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReminderEdit)
{
    LOG_INFO("创建提醒编辑对话框");
    ui->setupUi(this);
    setupConnections();
    LOG_INFO("提醒编辑对话框初始化完成");
}

ReminderEdit::~ReminderEdit()
{
    LOG_INFO("销毁提醒编辑对话框");
    delete ui;
}

void ReminderEdit::prepareNewReminder()
{
    LOG_INFO("初始化新建提醒");
    setWindowTitle(tr("新增提醒"));

    ui->nameEdit->clear();
    QDateTime now = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(now);
    ui->timeEdit->setTime(now.time());
    ui->typeCombo->setCurrentIndex(0);

    // QJsonObject does not have a clear() method, so assign a new empty object
    reminderData = QJsonObject();
    reminderData["id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    reminderData["type"] = static_cast<int>(ReminderType::OneTime);
    reminderData["nextTrigger"] = now.toString(Qt::ISODate);

    onTypeChanged(0);
    LOG_INFO("新建提醒准备完成");
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

void ReminderEdit::prepareEditReminder(const QJsonObject &reminder)
{
    LOG_INFO("加载待编辑的提醒数据");
    setWindowTitle(tr("编辑提醒"));

    reminderData = reminder;
    ui->nameEdit->setText(reminder["name"].toString());

    int type = reminder["type"].toInt();
    ui->typeCombo->setCurrentIndex(type);

    QDateTime nextTrigger = QDateTime::fromString(reminder["nextTrigger"].toString(), Qt::ISODate);
    if (type == static_cast<int>(ReminderType::OneTime)) {
        ui->dateTimeEdit->setDateTime(nextTrigger);
    } else {
        ui->timeEdit->setTime(nextTrigger.time());
    }

    onTypeChanged(type);
    LOG_INFO("编辑提醒准备完成");
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
        
        LOG_INFO(QString("保存提醒: ID='%1', 名称='%2', 类型='%3'")
                 .arg(reminderData["id"].toString())
                 .arg(name)
                 .arg(reminderData["type"].toInt()));
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
