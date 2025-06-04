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
    setupUI();
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

void ReminderEdit::setupUI()
{
    setWindowTitle(tr("编辑提醒"));
    resize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建表单布局
    QFormLayout *formLayout = new QFormLayout();
    
    nameEdit = new QLineEdit(this);
    formLayout->addRow(tr("名称:"), nameEdit);

    typeCombo = new QComboBox(this);
    typeCombo->addItems({
        tr("一次性"),
        tr("每天"),
        tr("每周"),
        tr("每月")
    });
    formLayout->addRow(tr("类型:"), typeCombo);

    dateTimeEdit = new QDateTimeEdit(this);
    dateTimeEdit->setCalendarPopup(true);
    dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    formLayout->addRow(tr("时间:"), dateTimeEdit);

    weekDaysList = new QListWidget(this);
    weekDaysList->addItems({
        tr("星期一"),
        tr("星期二"),
        tr("星期三"),
        tr("星期四"),
        tr("星期五"),
        tr("星期六"),
        tr("星期日")
    });
    for (int i = 0; i < weekDaysList->count(); ++i) {
        weekDaysList->item(i)->setFlags(weekDaysList->item(i)->flags() | Qt::ItemIsUserCheckable);
        weekDaysList->item(i)->setCheckState(Qt::Unchecked);
    }
    formLayout->addRow(tr("星期:"), weekDaysList);

    monthDaysList = new QListWidget(this);
    for (int i = 1; i <= 31; ++i) {
        QListWidgetItem *item = new QListWidgetItem(QString::number(i));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        monthDaysList->addItem(item);
    }
    formLayout->addRow(tr("日期:"), monthDaysList);

    // 添加下次触发时间标签
    nextTriggerLabel = new QLabel(this);
    nextTriggerLabel->setText(tr("下次触发时间："));
    formLayout->addRow(tr(""), nextTriggerLabel);

    mainLayout->addLayout(formLayout);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    okButton = new QPushButton(tr("确定"), this);
    cancelButton = new QPushButton(tr("取消"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    updateUI();
}

void ReminderEdit::setupConnections()
{
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReminderEdit::onTypeChanged);
    connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged,
            this, &ReminderEdit::onDateTimeChanged);
    connect(okButton, &QPushButton::clicked,
            this, &ReminderEdit::onOkClicked);
    connect(cancelButton, &QPushButton::clicked,
            this, &ReminderEdit::onCancelClicked);
}

void ReminderEdit::updateUI()
{
    bool isWeekly = typeCombo->currentText() == tr("每周");
    bool isMonthly = typeCombo->currentText() == tr("每月");
    
    weekDaysList->setVisible(isWeekly);
    monthDaysList->setVisible(isMonthly);
    dateTimeEdit->setVisible(!isWeekly && !isMonthly);
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
            categoryCombo->addItems(categories);
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
    QString category = newCategoryEdit->text().trimmed();
    if (!category.isEmpty()) {
        addCategory(category);
        categoryCombo->addItem(category);
        newCategoryEdit->clear();
    }
}

void ReminderEdit::onRemoveCategory()
{
    QString category = categoryCombo->currentText();
    if (!category.isEmpty()) {
        removeCategory(category);
        categoryCombo->removeItem(categoryCombo->currentIndex());
    }
}

void ReminderEdit::onAddTag()
{
    QString tag = newTagEdit->text().trimmed();
    if (!tag.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(tag);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        tagList->addItem(item);
        newTagEdit->clear();
        
        QJsonArray tags = reminderData["Tags"].toArray();
        tags.append(tag);
        reminderData["Tags"] = tags;
    }
}

void ReminderEdit::onRemoveTag()
{
    QListWidgetItem *item = tagList->currentItem();
    if (item) {
        QString tag = item->text();
        tagList->takeItem(tagList->row(item));
        
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
    reminderData = reminder;
    
    // 设置名称
    nameEdit->setText(reminder["Name"].toString());
    
    // 设置分类
    QString category = reminder["Category"].toString();
    int categoryIndex = categoryCombo->findText(category);
    if (categoryIndex >= 0) {
        categoryCombo->setCurrentIndex(categoryIndex);
    }
    
    // 设置标签
    tagList->clear();
    QJsonArray tags = reminder["Tags"].toArray();
    for (const QJsonValue &tag : tags) {
        QListWidgetItem *item = new QListWidgetItem(tag.toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        tagList->addItem(item);
    }
    
    // 设置类型
    QString type = reminder["Type"].toString();
    int typeIndex = 0;
    if (type == "Daily") typeIndex = 1;
    else if (type == "Weekly") typeIndex = 2;
    else if (type == "Monthly") typeIndex = 3;
    typeCombo->setCurrentIndex(typeIndex);
    
    // 设置时间
    QDateTime nextTrigger = QDateTime::fromString(reminder["NextTrigger"].toString(), Qt::ISODate);
    dateTimeEdit->setDateTime(nextTrigger);
    
    // 设置星期
    if (type == "Weekly") {
        QJsonArray weekDaysArray = reminder["WeekDays"].toArray();
        QList<int> weekDays;
        for (const QJsonValue &value : weekDaysArray) {
            weekDays.append(value.toInt());
        }
        for (int i = 0; i < weekDaysList->count(); ++i) {
            weekDaysList->item(i)->setCheckState(
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
        for (int i = 0; i < monthDaysList->count(); ++i) {
            monthDaysList->item(i)->setCheckState(
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
    dateTimeEdit->hide();
    weekDaysList->hide();
    monthDaysList->hide();
    
    // 根据类型显示相应控件
    switch (index) {
        case 0: // 一次性提醒
            dateTimeEdit->show();
            reminderData["Type"] = "OneTime";
            break;
        case 1: // 每日提醒
            reminderData["Type"] = "Daily";
            break;
        case 2: // 每周提醒
            weekDaysList->show();
            reminderData["Type"] = "Weekly";
            break;
        case 3: // 每月提醒
            monthDaysList->show();
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
    QString type = typeCombo->currentText();
    QDateTime nextTime = nextTriggerTime;

    if (type == tr("每天")) {
        nextTime = nextTime.addDays(1);
    } else if (type == tr("每周")) {
        nextTime = nextTime.addDays(7);
    } else if (type == tr("每月")) {
        nextTime = nextTime.addMonths(1);
    }

    nextTriggerLabel->setText(tr("下次触发时间：%1")
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
    
    switch (typeCombo->currentIndex()) {
        case 0: // 一次性提醒
            nextTrigger = dateTimeEdit->dateTime();
            break;
        case 1: { // 每日提醒
            QTime time = dateTimeEdit->time();
            nextTrigger = QDateTime(now.date(), time);
            if (nextTrigger <= now) {
                nextTrigger = nextTrigger.addDays(1);
            }
            break;
        }
        case 2: { // 每周提醒
            QTime time = dateTimeEdit->time();
            QList<int> weekDays;
            for (int i = 0; i < weekDaysList->count(); ++i) {
                if (weekDaysList->item(i)->checkState() == Qt::Checked) {
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
            QTime time = dateTimeEdit->time();
            QList<int> monthDays;
            for (int i = 0; i < monthDaysList->count(); ++i) {
                if (monthDaysList->item(i)->checkState() == Qt::Checked) {
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
    bool isValid = !nameEdit->text().trimmed().isEmpty();
    okButton->setEnabled(isValid);
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
    if (typeCombo->currentIndex() == 2) { // 每周
        return weekDaysList->item(day - 1)->checkState() == Qt::Checked;
    } else if (typeCombo->currentIndex() == 3) { // 每月
        return monthDaysList->item(day - 1)->checkState() == Qt::Checked;
    }
    return false;
} 