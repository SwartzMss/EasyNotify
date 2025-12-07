#ifndef REMINDERLISTSTYLER_H
#define REMINDERLISTSTYLER_H

#include <QLineEdit>
#include <QPushButton>
#include <QTableView>

namespace ReminderListStyler {

void apply(QLineEdit *searchEdit,
           QPushButton *primaryButton,
           QPushButton *secondaryButton,
           QTableView *tableView);

} // namespace ReminderListStyler

#endif // REMINDERLISTSTYLER_H
