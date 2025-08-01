#include "active_reminderedit.h"
#include "ui_reminderedit.h"
#include <QMessageBox>
#include <QUuid>
#include <QDateTime>
#include <QFile>
#include <QPushButton>
#include "logger.h"


ActiveReminderEdit::ActiveReminderEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReminderEdit)
{
    LOG_INFO("创建提醒编辑对话框");
    ui->setupUi(this);
    setupConnections();
    LOG_INFO("提醒编辑对话框初始化完成");
}

ActiveReminderEdit::~ActiveReminderEdit()
{
    LOG_INFO("销毁提醒编辑对话框");
    delete ui;
}

void ActiveReminderEdit::prepareNewReminder()
{
    LOG_INFO("初始化新建提醒");
    setWindowTitle(tr("新增提醒"));

    ui->nameEdit->clear();
    QDateTime now = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(now);
    ui->timeEdit->setTime(now.time());
    ui->typeCombo->setCurrentIndex(0);
    ui->priorityCombo->setCurrentIndex(1);

    m_reminder = Reminder();
    m_reminder.setId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    m_reminder.setType(Reminder::Type::Once);
    m_reminder.setNextTrigger(now);
    m_reminder.setPriority(Reminder::Priority::Medium);

    onTypeChanged(0);
    LOG_INFO("新建提醒准备完成");
}

void ActiveReminderEdit::setupConnections()
{
    connect(ui->typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ActiveReminderEdit::onTypeChanged);
    connect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged,
            this, &ActiveReminderEdit::onDateTimeChanged);
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ActiveReminderEdit::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &ActiveReminderEdit::onCancelClicked);
}

void ActiveReminderEdit::prepareEditReminder(const Reminder &reminder)
{
    LOG_INFO("加载待编辑的提醒数据");
    setWindowTitle(tr("编辑提醒"));

    m_reminder = reminder;
    ui->nameEdit->setText(reminder.name());

    int type = static_cast<int>(reminder.type());
    ui->typeCombo->setCurrentIndex(type);
    ui->priorityCombo->setCurrentIndex(static_cast<int>(reminder.priority()));

    QDateTime nextTrigger = reminder.nextTrigger();
    ui->dateTimeEdit->setDateTime(nextTrigger);
    ui->timeEdit->setTime(nextTrigger.time());

    onTypeChanged(type);
    LOG_INFO("编辑提醒准备完成");
}


Reminder ActiveReminderEdit::getReminder() const
{
    // 如果提醒名称为空，返回一个空的提醒对象
    if (m_reminder.name().isEmpty()) {
        return Reminder();
    }
    return m_reminder;
}

void ActiveReminderEdit::onTypeChanged(int index)
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
            m_reminder.setType(Reminder::Type::Once);
            break;
        case 1: // 每天
            ui->timeEdit->show();
            ui->timeLabel->show();
            m_reminder.setType(Reminder::Type::Daily);
            break;
    }
    updateNextTriggerTime();
}

void ActiveReminderEdit::onDateTimeChanged(const QDateTime &dateTime)
{
    LOG_INFO(QString("日期时间变更为: %1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
    updateNextTriggerTime();
}

void ActiveReminderEdit::onOkClicked()
{
    LOG_INFO("点击确定按钮");
    if (!validateInput()) {
        LOG_WARNING("输入验证失败，无法保存提醒");
        return;
    }

    QString name = ui->nameEdit->text().trimmed();
    m_reminder.setName(name);
    m_reminder.setPriority(static_cast<Reminder::Priority>(ui->priorityCombo->currentIndex()));
    updateNextTriggerTime();

    LOG_INFO(QString("保存提醒: ID='%1', 名称='%2', 类型='%3'")
             .arg(m_reminder.id())
             .arg(name)
             .arg(static_cast<int>(m_reminder.type())));
    accept();
}

void ActiveReminderEdit::onCancelClicked()
{
    LOG_INFO("点击取消按钮，关闭对话框");
    reject();
}

void ActiveReminderEdit::onDaysChanged()
{
    LOG_INFO("星期/日期选择发生变化");
    updateNextTriggerTime();
}

QDateTime ActiveReminderEdit::calculateNextTrigger() const
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextTrigger;
    Reminder::Type type = static_cast<Reminder::Type>(ui->typeCombo->currentIndex());

    switch (type) {
        case Reminder::Type::Once: {
            nextTrigger = ui->dateTimeEdit->dateTime();
            LOG_INFO(QString("计算一次性提醒时间: %1").arg(nextTrigger.toString("yyyy-MM-dd HH:mm:ss")));
            break;
        }
        case Reminder::Type::Daily: {
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

bool ActiveReminderEdit::validateInput() const
{
    bool isValid = !ui->nameEdit->text().trimmed().isEmpty();
    if (!isValid) {
        QMessageBox::warning(const_cast<ActiveReminderEdit*>(this),
            tr("输入错误"),
            tr("提醒名称不能为空！"));
        LOG_WARNING("提醒名称不能为空");
        return false;
    }

    // 检查一次性提醒的时间是否有效
    if (ui->typeCombo->currentIndex() == 0) { // 一次性提醒
        QDateTime selectedTime = ui->dateTimeEdit->dateTime();
        QDateTime currentTime = QDateTime::currentDateTime();
        
        if (selectedTime <= currentTime) {
            QMessageBox::warning(const_cast<ActiveReminderEdit*>(this),
                tr("时间设置错误"),
                tr("一次性提醒的时间必须晚于当前时间！\n\n当前时间：%1\n设置时间：%2")
                    .arg(currentTime.toString("yyyy-MM-dd HH:mm:ss"))
                    .arg(selectedTime.toString("yyyy-MM-dd HH:mm:ss")));
            LOG_WARNING(QString("一次性提醒时间无效: %1 <= %2")
                .arg(selectedTime.toString("yyyy-MM-dd HH:mm:ss"))
                .arg(currentTime.toString("yyyy-MM-dd HH:mm:ss")));
            return false;
        }
    }

    return true;
}

void ActiveReminderEdit::updateNextTriggerTime()
{
    QDateTime nextTime = calculateNextTrigger();
    ui->nextTriggerLabel->setText(nextTime.toString("yyyy-MM-dd HH:mm:ss"));
    m_reminder.setNextTrigger(nextTime);
    LOG_INFO(QString("更新下次触发时间: %1").arg(nextTime.toString("yyyy-MM-dd HH:mm:ss")));
}
