#include "priorityiconprovider.h"

#include <QPainter>
#include <QPixmap>

namespace {
QColor baseColor(Reminder::Priority priority)
{
    switch (priority) {
    case Reminder::Priority::Low:
        return QColor("#16a34a"); // green
    case Reminder::Priority::High:
        return QColor("#dc2626"); // red
    case Reminder::Priority::Medium:
    default:
        return QColor("#f97316"); // orange
    }
}

QIcon buildIcon(const QColor &color)
{
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(QRectF(2, 2, 14, 14));

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(color.darker(130), 1));
    painter.drawEllipse(QRectF(2, 2, 14, 14));
    painter.end();

    return QIcon(pixmap);
}
} // namespace

QIcon PriorityIconProvider::icon(Reminder::Priority priority)
{
    return buildIcon(baseColor(priority));
}

QColor PriorityIconProvider::color(Reminder::Priority priority)
{
    return baseColor(priority);
}
