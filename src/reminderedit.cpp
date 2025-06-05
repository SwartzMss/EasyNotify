#include "reminderedit.h"
#include "./ui_reminderedit.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QUuid>
#include <QDateTime>
#include <QTime>
#include <QDate>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

QStringList ReminderEdit::categories;
const QString ReminderEdit::CATEGORIES_FILE = "categories.json";

ReminderEdit::ReminderEdit(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReminderEdit)
{
    ui->setupUi(this);
    setWindowTitle(tr("新增提醒"));
    setupConnections();
    loadCategories();
    // 设置默认值
    reminderData["Id"] = generateId();
    reminderData["IsEnabled"] = true;
    reminderData["Type"] = "OneTime";
    reminderData["NextTrigger"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    reminderData["Tags"] = QJsonArray();
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

void ReminderEdit::loadCategories()
{
    QFile file(CATEGORIES_FILE);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        if (doc.isArray()) {
            categories.clear();
            QJsonArray array = doc.array();
            for (const QJsonValue &value : array) {
                categories.append(value.toString());
            }
            ui->categoryCombo->addItems(categories);
        }
    }
}

void ReminderEdit::saveCategories()
{
    QFile file(CATEGORIES_FILE);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonArray array;
        for (const QString &category : categories) {
            array.append(category);
        }
        QJsonDocument doc(array);
        file.write(doc.toJson());
        file.close();
    }
}

QStringList ReminderEdit::getCategories()
{
    return categories;
}

void ReminderEdit::addCategory(const QString &category)
{
    if (!categories.contains(category)) {
        categories.append(category);
        saveCategories();
    }
}

void ReminderEdit::removeCategory(const QString &category)
{
    categories.removeOne(category);
    saveCategories();
}

void ReminderEdit::onAddCategory()
{
    QString category = ui->newCategoryEdit->text().trimmed();
    if (!category.isEmpty()) {
        addCategory(category);
        ui->categoryCombo->addItem(category);
        ui->newCategoryEdit->clear();
    }
}

void ReminderEdit::onRemoveCategory()
{
    QString category = ui->categoryCombo->currentText();
    if (!category.isEmpty()) {
        removeCategory(category);
        ui->categoryCombo->removeItem(ui->categoryCombo->currentIndex());
    }
}

void ReminderEdit::onAddTag()
{
    QString tag = ui->newTagEdit->text().trimmed();
    if (!tag.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(tag);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->tagList->addItem(item);
        ui->newTagEdit->clear();
        
        QJsonArray tags = reminderData["Tags"].toArray();
        tags.append(tag);
        reminderData["Tags"] = tags;
    }
}

void ReminderEdit::onRemoveTag()
{
    QListWidgetItem *item = ui->tagList->currentItem();
    if (item) {
        QString tag = item->text();
        ui->tagList->takeItem(ui->tagList->row(item));
        
        QJsonArray tags = reminderData["Tags"].toArray();
        for (int i = 0; i < tags.size(); ++i) {
            if (tags[i].toString() == tag) {
                tags.removeAt(i);
                break;
            }
        }
        reminderData["Tags"] = tags;
    }
}

void ReminderEdit::loadReminderData(const QJsonObject &reminder)
{
    setWindowTitle(tr("编辑提醒"));
    reminderData = reminder;
    ui->nameEdit->setText(reminder["Name"].toString());
    
    // 设置分类
    QString category = reminder["Category"].toString();
    int categoryIndex = ui->categoryCombo->findText(category);
    if (categoryIndex >= 0) {
        ui->categoryCombo->setCurrentIndex(categoryIndex);
    }
    
    // 设置标签
    ui->tagList->clear();
    QJsonArray tags = reminder["Tags"].toArray();
    for (const QJsonValue &tag : tags) {
        QListWidgetItem *item = new QListWidgetItem(tag.toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->tagList->addItem(item);
    }
    
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
    // 隐藏所有时间相关控件
    ui->dateTimeEdit->hide();
    ui->weekDaysList->hide();
    ui->monthDaysList->hide();
    
    // 根据类型显示相应控件
    switch (index) {
        case 0: // 一次性提醒
            ui->dateTimeEdit->show();
            reminderData["Type"] = "OneTime";
            break;
        case 1: // 每日提醒
            reminderData["Type"] = "Daily";
            break;
        case 2: // 每周提醒
            ui->weekDaysList->show();
            reminderData["Type"] = "Weekly";
            break;
        case 3: // 每月提醒
            ui->monthDaysList->show();
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

    ui->nextTriggerLabel->setText(tr("下次触发时间：%1")
        .arg(nextTime.toString("yyyy-MM-dd HH:mm:ss")));
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

void ReminderEdit::onCancelClicked()
{
    reject();
}

void ReminderEdit::onDaysChanged()
{
    // 更新下次触发时间
    updateNextTriggerTime();
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