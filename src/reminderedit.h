#ifndef REMINDEREDIT_H
#define REMINDEREDIT_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QDateTime>

namespace Ui {
class ReminderEdit;
}

class ReminderEdit : public QDialog
{
    Q_OBJECT

public:
    explicit ReminderEdit(QWidget *parent = nullptr);
    ~ReminderEdit();

    void loadReminderData(const QJsonObject &reminder);
    QJsonObject getReminderData() const;
    void reset();

private slots:
    void onTypeChanged(int index);
    void onOkClicked();
    void onCancelClicked();
    void onDateTimeChanged(const QDateTime &dateTime);
    void onDaysChanged();

private:
    void setupConnections();
    QDateTime calculateNextTrigger() const;
    bool validateInput() const;
    bool isDaySelected(int day) const;
    void updateNextTriggerTime();

    Ui::ReminderEdit *ui;
    QJsonObject reminderData;
    QDateTime nextTriggerTime;

    static QStringList categories;
    static const QString CATEGORIES_FILE;
};

#endif // REMINDEREDIT_H 