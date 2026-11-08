#include "clockwidget.h"

#include <algorithm>
#include <cmath>

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPen>
#include <QRadialGradient>

namespace {

constexpr qreal PI = 3.14159265358979323846;

QPointF offsetFrom(const QPointF& center, qreal angleDeg, qreal length) {
    const qreal angle = (angleDeg - 90.0) * PI / 180.0;
    return QPointF(center.x() + std::cos(angle) * length,
                   center.y() + std::sin(angle) * length);
}

}  // namespace

ClockWidget::ClockWidget(QWidget* parent)
    : QWidget(parent), minutes_(20 * 60) {
    setMinimumSize(48, 48);
}

void ClockWidget::setMinutes(int minutes) {
    minutes_ = minutes;
    update();
}

QSize ClockWidget::sizeHint() const {
    return QSize(96, 96);
}

void ClockWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QPointF center(width() / 2.0, height() / 2.0);
    const qreal radius = std::min(width(), height()) / 2.0 - 3.0;

    QRadialGradient face(center, radius);
    face.setColorAt(0.0, QColor(30, 32, 30));
    face.setColorAt(0.8, QColor(17, 19, 18));
    face.setColorAt(1.0, QColor(9, 10, 10));

    painter.setBrush(face);
    painter.setPen(QPen(QColor(96, 34, 30), 3));
    painter.drawEllipse(center, radius, radius);

    for (int i = 0; i < 12; ++i) {
        const qreal angle = i * 30.0;
        const bool major = (i % 3 == 0);
        const qreal inner = radius - (major ? 18.0 : 10.0);
        painter.setPen(QPen(major ? QColor(172, 62, 52) : QColor(116, 44, 38),
                            major ? 3.0 : 1.5));
        painter.drawLine(offsetFrom(center, angle, inner),
                         offsetFrom(center, angle, radius - 5.0));
    }

    const int hour = (minutes_ / 60) % 12;
    const int minute = minutes_ % 60;
    const qreal minuteAngle = minute * 6.0;
    const qreal hourAngle = hour * 30.0 + minute * 0.5;

    painter.setPen(QPen(QColor(160, 48, 40),
                        std::max(3.0, radius * 0.075), Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(center, offsetFrom(center, hourAngle, radius * 0.46));

    painter.setPen(QPen(QColor(186, 62, 50),
                        std::max(2.0, radius * 0.042), Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(center, offsetFrom(center, minuteAngle, radius * 0.72));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(150, 44, 38));
    painter.drawEllipse(center, std::max(3.0, radius * 0.08),
                        std::max(3.0, radius * 0.08));
}
