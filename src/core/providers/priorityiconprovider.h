#ifndef PRIORITYICONPROVIDER_H
#define PRIORITYICONPROVIDER_H

#include <QColor>
#include <QIcon>
#include "core/reminders/reminder.h"

class PriorityIconProvider
{
public:
    static QIcon icon(Reminder::Priority priority);
    static QColor color(Reminder::Priority priority);
};

#endif // PRIORITYICONPROVIDER_H
