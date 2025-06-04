#ifndef REMINDEREDIT_H
#define REMINDEREDIT_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QDateTime>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QList>

namespace Ui {
class ReminderEdit;
}

class QLineEdit;
class QComboBox;
class QDateTimeEdit;
class QListWidget;
class QPushButton;
class QLabel;
class QTimeEdit;
class QDateEdit;
class QCheckBox;

class ReminderEdit : public QDialog
{
    Q_OBJECT

public:
    explicit ReminderEdit(QWidget *parent = nullptr);
    ~ReminderEdit();

    void loadReminderData(const QJsonObject &reminder);
    QJsonObject getReminderData() const;
    static QStringList getCategories();
    static void addCategory(const QString &category);
    static void removeCategory(const QString &category);
    static void saveCategories();

private slots:
    void onTypeChanged(int index);
    void onOkClicked();
    void onCancelClicked();
    void onDateTimeChanged(const QDateTime &dateTime);
    void onTimeChanged(const QTime &time);
    void onDaysChanged();
    void onDateChanged(const QDate &date);
    void onAddCategory();
    void onRemoveCategory();
    void onAddTag();
    void onRemoveTag();

private:
    void setupUI();
    void setupConnections();
    void updateUI();
    void loadCategories();
    void updateNextTriggerTime();
    QString generateId() const;
    QDateTime calculateNextTrigger() const;
    bool validateInput() const;
    bool isDaySelected(int day) const;

    Ui::ReminderEdit *ui;
    QJsonObject reminderData;
    QLineEdit *nameEdit;
    QComboBox *typeCombo;
    QDateTimeEdit *dateTimeEdit;
    QTimeEdit *timeEdit;
    QDateEdit *dateEdit;
    QListWidget *weekDaysList;
    QListWidget *monthDaysList;
    QListWidget *tagList;
    QComboBox *categoryCombo;
    QLineEdit *newCategoryEdit;
    QLineEdit *newTagEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *nextTriggerLabel;
    QDateTime nextTriggerTime;
    QCheckBox *enabledCheck;
    QList<QCheckBox*> weekDaysCheckBoxes;
    QList<QCheckBox*> monthDaysCheckBoxes;

    static QStringList categories;
    static const QString CATEGORIES_FILE;
};

#endif // REMINDEREDIT_H 