#include "core/providers/priorityiconprovider.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QLinearGradient>
#include <QtMath>
#include <cmath>

namespace {
QColor baseColor(Reminder::Priority priority)
{
    switch (priority) {
    case Reminder::Priority::Low:
        return QColor("#22c55e"); // friendly green
    case Reminder::Priority::High:
        return QColor("#ef4444"); // bright red
    case Reminder::Priority::Medium:
    default:
        return QColor("#facc15"); // sunny yellow
    }
}

QPainterPath starShape(qreal cx, qreal cy, qreal rOuter, qreal rInner)
{
    QPainterPath path;
    constexpr int points = 5;
    for (int i = 0; i <= points; ++i) {
        const qreal angle = i * 72.0 - 90.0;
        const qreal radOuter = qDegreesToRadians(angle);
        const qreal radInner = qDegreesToRadians(angle + 36.0);
        QPointF pOuter(cx + rOuter * std::cos(radOuter), cy + rOuter * std::sin(radOuter));
        QPointF pInner(cx + rInner * std::cos(radInner), cy + rInner * std::sin(radInner));
        if (i == 0) {
            path.moveTo(pOuter);
        } else {
            path.lineTo(pOuter);
        }
        path.lineTo(pInner);
    }
    path.closeSubpath();
    return path;
}

QIcon buildIcon(Reminder::Priority priority, const QColor &color)
{
    QPixmap pixmap(26, 26);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const qreal w = pixmap.width();
    const qreal h = pixmap.height();

    // soft halo
    QColor shadow = color;
    shadow.setAlpha(70);
    painter.setPen(Qt::NoPen);
    painter.setBrush(shadow);
    painter.drawEllipse(QRectF(w * 0.18, h * 0.24, w * 0.64, h * 0.64));

    // sticker base
    QLinearGradient gradient(0, 0, 0, h);
    gradient.setColorAt(0.0, color.lighter(165));
    gradient.setColorAt(1.0, color.darker(110));
    painter.setBrush(gradient);
    painter.setPen(QPen(color.darker(145), 1.1));
    painter.drawRoundedRect(QRectF(w * 0.12, h * 0.12, w * 0.76, h * 0.76), 7, 7);

    // pick a cute shape per priority
    painter.setPen(Qt::NoPen);
    QColor lineColor(40, 40, 40, 200);
    switch (priority) {
    case Reminder::Priority::Low: {
        // fluffy cloud with sleepy face
        QPainterPath cloud;
        cloud.addEllipse(QRectF(w * 0.30, h * 0.52, w * 0.30, h * 0.20));
        cloud.addEllipse(QRectF(w * 0.44, h * 0.45, w * 0.30, h * 0.24));
        cloud.addEllipse(QRectF(w * 0.28, h * 0.56, w * 0.44, h * 0.20));
        painter.setBrush(QColor(255, 255, 255, 240));
        painter.setPen(QPen(lineColor, 1.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(cloud);
        painter.fillPath(cloud, QColor(255, 255, 255, 240));

        // eyes (sleepy)
        painter.setPen(QPen(lineColor, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawArc(QRectF(w * 0.36, h * 0.58, w * 0.14, h * 0.10), 0, -180 * 16);
        painter.drawArc(QRectF(w * 0.54, h * 0.58, w * 0.14, h * 0.10), 0, -180 * 16);
        // small Z sparkle
        painter.drawLine(QPointF(w * 0.32, h * 0.38), QPointF(w * 0.42, h * 0.38));
        painter.drawLine(QPointF(w * 0.42, h * 0.38), QPointF(w * 0.32, h * 0.32));
        painter.drawLine(QPointF(w * 0.32, h * 0.32), QPointF(w * 0.42, h * 0.32));
        break;
    }
    case Reminder::Priority::Medium: {
        // star with smile
        QPainterPath star = starShape(w / 2.0, h / 2.0 + 1, 7.0, 3.5);
        painter.setBrush(QColor(255, 249, 230, 255));
        painter.setPen(QPen(lineColor, 1.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(star);
        painter.fillPath(star, QColor(255, 249, 230, 255));

        painter.setPen(QPen(lineColor, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawEllipse(QRectF(w * 0.38, h * 0.44, 2.4, 2.4));
        painter.drawEllipse(QRectF(w * 0.53, h * 0.44, 2.4, 2.4));
        QPainterPath smile;
        smile.moveTo(w * 0.37, h * 0.62);
        smile.quadTo(w * 0.50, h * 0.70, w * 0.63, h * 0.62);
        painter.drawPath(smile);
        // sparkle
        painter.setPen(QPen(QColor(255, 255, 255, 200), 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(w * 0.28, h * 0.28), QPointF(w * 0.42, h * 0.28));
        painter.drawLine(QPointF(w * 0.35, h * 0.22), QPointF(w * 0.35, h * 0.36));
        break;
    }
    case Reminder::Priority::High:
    default: {
        // heart with wink
        QPainterPath heart;
        heart.moveTo(w / 2.0, h * 0.30);
        heart.cubicTo(w / 2.0 + w * 0.20, h * 0.08, w - w * 0.22, h * 0.32, w / 2.0, h * 0.74);
        heart.cubicTo(w * 0.22, h * 0.32, w / 2.0 - w * 0.20, h * 0.08, w / 2.0, h * 0.30);
        painter.setBrush(QColor(255, 240, 245, 255));
        painter.setPen(QPen(lineColor, 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(heart);
        painter.fillPath(heart, QColor(255, 240, 245, 255));

        painter.setPen(QPen(lineColor, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawEllipse(QRectF(w * 0.38, h * 0.41, 2.4, 2.4));
        painter.drawLine(QPointF(w * 0.58, h * 0.45), QPointF(w * 0.69, h * 0.42));
        painter.drawLine(QPointF(w * 0.69, h * 0.42), QPointF(w * 0.74, h * 0.48));
        QPainterPath smile;
        smile.moveTo(w * 0.38, h * 0.62);
        smile.quadTo(w * 0.50, h * 0.74, w * 0.68, h * 0.64);
        painter.drawPath(smile);

        // tiny shine
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 180));
        painter.drawEllipse(QRectF(w * 0.32, h * 0.26, 3.2, 3.2));
        break;
    }
    }

    painter.end();

    return QIcon(pixmap);
}
} // namespace

QIcon PriorityIconProvider::icon(Reminder::Priority priority)
{
    return buildIcon(priority, baseColor(priority));
}

QColor PriorityIconProvider::color(Reminder::Priority priority)
{
    return baseColor(priority);
}
