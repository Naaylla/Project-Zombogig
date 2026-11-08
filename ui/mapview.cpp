#include "mapview.h"

#include <algorithm>
#include <cmath>

#include <QColor>
#include <QDateTime>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QPolygon>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QResizeEvent>
#include <QStringList>
#include <QTimer>

#include "entities/entity.h"
#include "entities/human.h"
#include "entities/zombie.h"
#include "prop.h"

namespace {

constexpr int TILE_WIDTH = 144;
constexpr int TILE_HEIGHT = 72;
constexpr int TILE_THICKNESS = 44;
constexpr int TOP_HALF_WIDTH = 72;
constexpr int TOP_HALF_HEIGHT = 36;
constexpr int CACHE_PAD = 2;

constexpr int MAP_MARGIN = 36;
constexpr qreal MAX_MAP_SCALE = 2.6;

QColor darker(const QColor& color, int amount) {
    return QColor(std::max(0, color.red() - amount),
                  std::max(0, color.green() - amount),
                  std::max(0, color.blue() - amount));
}

QColor mixColor(const QColor& a, const QColor& b, qreal t) {
    return QColor(qRound(a.red() + (b.red() - a.red()) * t),
                  qRound(a.green() + (b.green() - a.green()) * t),
                  qRound(a.blue() + (b.blue() - a.blue()) * t));
}

void drawBlock(QPainter& painter, int cx, int baseCenterY, const QColor& base,
               int halfW = TOP_HALF_WIDTH, int halfH = TOP_HALF_HEIGHT,
               int thickness = TILE_THICKNESS) {
    const int topY = baseCenterY - thickness;

    QPolygon left;
    left << QPoint(cx - halfW, topY)
         << QPoint(cx, topY + halfH)
         << QPoint(cx, baseCenterY + halfH)
         << QPoint(cx - halfW, baseCenterY);

    QPolygon right;
    right << QPoint(cx, topY + halfH)
          << QPoint(cx + halfW, topY)
          << QPoint(cx + halfW, baseCenterY)
          << QPoint(cx, baseCenterY + halfH);

    painter.setPen(Qt::NoPen);
    painter.setBrush(darker(base, 48));
    painter.drawPolygon(left);
    painter.setBrush(darker(base, 28));
    painter.drawPolygon(right);

    QPolygon upper;
    upper << QPoint(cx, topY - halfH)
          << QPoint(cx + halfW, topY)
          << QPoint(cx - halfW, topY);

    QPolygon lower;
    lower << QPoint(cx - halfW, topY)
          << QPoint(cx + halfW, topY)
          << QPoint(cx, topY + halfH);

    painter.setBrush(base.lighter(106));
    painter.drawPolygon(upper);
    painter.setBrush(base);
    painter.drawPolygon(lower);

    QPolygon top;
    top << QPoint(cx, topY - halfH)
        << QPoint(cx + halfW, topY)
        << QPoint(cx, topY + halfH)
        << QPoint(cx - halfW, topY);

    painter.setPen(QPen(darker(base, 55), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPolygon(top);
}

void drawProp(QPainter& painter, int cx, int topY, const Prop& prop) {
    switch (prop.kind()) {
    case PropKind::Wall: {
        const int wallHeight = 22;
        const QPoint top(cx, topY - TOP_HALF_HEIGHT);
        const QPoint left(cx - TOP_HALF_WIDTH, topY);
        const QPoint right(cx + TOP_HALF_WIDTH, topY);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(66, 58, 48));
        painter.drawPolygon(QPolygon()
                            << left << top
                            << QPoint(top.x(), top.y() - wallHeight)
                            << QPoint(left.x(), left.y() - wallHeight));
        painter.setBrush(QColor(78, 68, 56));
        painter.drawPolygon(QPolygon()
                            << top << right
                            << QPoint(right.x(), right.y() - wallHeight)
                            << QPoint(top.x(), top.y() - wallHeight));

        painter.setPen(QPen(QColor(46, 40, 33), 2));
        painter.drawLine(left.x(), left.y() - wallHeight,
                         top.x(), top.y() - wallHeight);
        painter.drawLine(top.x(), top.y() - wallHeight,
                         right.x(), right.y() - wallHeight);

        painter.setPen(QPen(QColor(50, 43, 35, 160), 1));
        for (int i = 1; i <= 2; ++i) {
            const int drop = wallHeight * i / 3;
            painter.drawLine(left.x(), left.y() - drop,
                             top.x(), top.y() - drop);
            painter.drawLine(top.x(), top.y() - drop,
                             right.x(), right.y() - drop);
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(54, 47, 38));
        const QPoint posts[] = {left, top, right};
        for (const QPoint& post : posts) {
            painter.drawRect(QRectF(post.x() - 2, post.y() - wallHeight,
                                    4, wallHeight + 3));
        }
        break;
    }
    case PropKind::Bush:
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 70));
        painter.drawEllipse(QPointF(cx, topY + 2), 24, 9);
        painter.setBrush(QColor(42, 56, 40));
        painter.drawEllipse(QPointF(cx - 9, topY - 6), 13, 11);
        painter.drawEllipse(QPointF(cx + 8, topY - 8), 12, 10);
        painter.setBrush(QColor(56, 72, 50));
        painter.drawEllipse(QPointF(cx, topY - 14), 13, 11);
        break;
    case PropKind::Tree:
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 80));
        painter.drawEllipse(QPointF(cx, topY + 2), 16, 6);
        painter.setBrush(QColor(54, 46, 38));
        painter.drawPolygon(QPolygon()
                            << QPoint(cx - 4, topY) << QPoint(cx + 4, topY)
                            << QPoint(cx + 3, topY - 36) << QPoint(cx - 3, topY - 36));
        painter.setPen(QPen(QColor(54, 46, 38), 2));
        painter.drawLine(cx - 2, topY - 26, cx - 14, topY - 38);
        painter.drawLine(cx + 2, topY - 30, cx + 13, topY - 42);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(38, 50, 38));
        painter.drawEllipse(QPointF(cx - 12, topY - 42), 14, 10);
        painter.drawEllipse(QPointF(cx + 12, topY - 45), 15, 10);
        painter.setBrush(QColor(48, 62, 46));
        painter.drawEllipse(QPointF(cx, topY - 52), 16, 11);
        break;
    case PropKind::Plant: {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 55));
        painter.drawEllipse(QPointF(cx, topY + 1), 13, 4);

        painter.setBrush(QColor(56, 46, 34));
        painter.drawEllipse(QPointF(cx - 9, topY), 8, 3);
        painter.drawEllipse(QPointF(cx + 9, topY), 8, 3);

        const int leaves = 3 + prop.growth();
        for (int i = 0; i < leaves; ++i) {
            const qreal dx = (i - (leaves - 1) / 2.0) * 7.0;
            const qreal tipX = cx + dx * 1.4;
            const qreal tipY = topY - 9 - prop.growth() * 3;
            painter.setPen(QPen(QColor(66, 102, 54), 2));
            painter.drawLine(QPointF(cx + dx, topY), QPointF(tipX, tipY));
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(78, 118, 62));
            painter.drawEllipse(QPointF(tipX, tipY), 3.5, 2.5);
        }

        if (prop.growth() >= 2) {
            painter.setBrush(QColor(176, 74, 52));
            painter.drawEllipse(QPointF(cx - 5, topY - 14), 2, 2);
            painter.drawEllipse(QPointF(cx + 6, topY - 16), 2, 2);
        }
        break;
    }
    case PropKind::Corpse:
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 70));
        painter.drawEllipse(QPointF(cx, topY + 2), 19, 7);
        painter.setBrush(QColor(70, 72, 60));
        painter.drawEllipse(QPointF(cx - 3, topY - 3), 11, 5);
        painter.setBrush(QColor(96, 92, 80));
        painter.drawEllipse(QPointF(cx + 8, topY - 5), 6, 5);
        break;
    case PropKind::Campfire:
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 80));
        painter.drawEllipse(QPointF(cx, topY + 2), 22, 8);
        for (int i = 0; i < 8; ++i) {
            const qreal angle = i * 3.14159265358979323846 / 4.0;
            painter.setBrush(QColor(84, 82, 76));
            painter.drawEllipse(QPointF(cx + std::cos(angle) * 17.0,
                                        topY + std::sin(angle) * 8.0),
                                4.5, 3.5);
        }
        painter.setBrush(QColor(64, 44, 30));
        painter.drawPolygon(QPolygon()
                            << QPoint(cx - 16, topY - 2) << QPoint(cx + 16, topY - 9)
                            << QPoint(cx + 18, topY - 4) << QPoint(cx - 14, topY + 3));
        painter.drawPolygon(QPolygon()
                            << QPoint(cx - 16, topY - 9) << QPoint(cx + 16, topY - 2)
                            << QPoint(cx + 14, topY + 3) << QPoint(cx - 18, topY - 4));
        painter.setBrush(QColor(196, 92, 38));
        painter.drawEllipse(QPointF(cx - 5, topY - 6), 4, 2.2);
        painter.drawEllipse(QPointF(cx + 6, topY - 8), 3.5, 2.0);
        painter.setBrush(QColor(232, 150, 54));
        painter.drawEllipse(QPointF(cx + 1, topY - 10), 5, 3);
        break;
    }
}

void drawBloodSplat(QPainter& painter, int cx, int groundY, quint32 seed,
                    bool large) {
    QRandomGenerator rng(seed);
    painter.save();
    painter.setPen(Qt::NoPen);

    const int blobs = large ? 8 : 4;
    for (int i = 0; i < blobs; ++i) {
        const qreal ox = rng.bounded(-44, 45);
        const qreal oy = rng.bounded(-11, 18);
        const qreal rx = rng.bounded(5, large ? 17 : 12);
        const qreal ry = rx * (0.28 + rng.generateDouble() * 0.24);
        const int alpha = rng.bounded(75, 140);
        painter.setBrush(QColor(104, 24, 20, alpha));
        painter.drawEllipse(QPointF(cx + ox, groundY + oy), rx, ry);
    }

    painter.setPen(QPen(QColor(88, 18, 16, 110), 2));
    const int streaks = rng.bounded(2, 5);
    for (int i = 0; i < streaks; ++i) {
        const qreal angle = rng.generateDouble() * 6.283185307179586;
        const qreal length = rng.bounded(8, large ? 30 : 20);
        const qreal sx = cx + rng.bounded(-9, 10);
        const qreal sy = groundY + rng.bounded(-4, 8);
        painter.drawLine(QPointF(sx, sy),
                         QPointF(sx + std::cos(angle) * length,
                                 sy + std::sin(angle) * length * 0.35));
    }
    painter.restore();
}

void drawEntity(QPainter& painter, int cx, int topY, const Entity& entity) {
    const bool human = entity.faction() == Faction::Human;
    const QColor body = entity.color();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 70));
    painter.drawEllipse(QPointF(cx, topY + 1), human ? 16 : 17, 6);

    int headY = topY - 50;
    int shoulderY = topY - 43;
    int waistY = topY - 18;
    int halfBody = human ? 10 : 11;
    bool lean = false;

    if (!human) {
        if (const auto* zombie = dynamic_cast<const Zombie*>(&entity)) {
            switch (zombie->type()) {
            case ZombieType::Tank:
                halfBody = 15;
                shoulderY = topY - 52;
                headY = topY - 60;
                break;
            case ZombieType::Runner:
                halfBody = 9;
                lean = true;
                break;
            case ZombieType::Exploder:
                halfBody = 14;
                shoulderY = topY - 46;
                headY = topY - 54;
                break;
            }
        }
    }

    painter.setPen(QPen(body.darker(140), 4));
    painter.drawLine(cx - 5, waistY, cx - 6, topY);
    painter.drawLine(cx + 5, waistY, cx + 6, topY);
    painter.setPen(Qt::NoPen);

    QPolygon torso;
    torso << QPoint(cx - halfBody, waistY) << QPoint(cx + halfBody, waistY)
          << QPoint(cx + halfBody - 1, shoulderY) << QPoint(cx - halfBody + 1, shoulderY);
    painter.setBrush(body);
    painter.drawPolygon(torso);

    painter.setPen(QPen(body.darker(120), 4));
    painter.drawLine(cx - halfBody, shoulderY + 5,
                     cx - halfBody - 6, waistY + 5);
    painter.drawLine(cx + halfBody, shoulderY + 5,
                     cx + halfBody + 6, waistY + 5);
    painter.setPen(Qt::NoPen);

    const QColor headColor = human ? QColor(158, 138, 116) : body.lighter(150);
    const int headX = cx + (lean ? 4 : 0);
    painter.setBrush(headColor);
    painter.drawEllipse(QPointF(headX, headY), 7.5, 7.5);

    if (!human) {
        painter.setBrush(QColor(178, 46, 34));
        painter.drawEllipse(QPointF(headX - 2.8, headY - 0.5), 1.8, 1.8);
        painter.drawEllipse(QPointF(headX + 2.8, headY - 0.5), 1.8, 1.8);

        QRandomGenerator rng(static_cast<quint32>(cx * 73 + topY * 131 + 911));
        painter.setPen(Qt::NoPen);
        for (int i = 0; i < 4; ++i) {
            const qreal bx = cx + rng.bounded(-halfBody + 3, halfBody - 2);
            const qreal by = rng.bounded(shoulderY + 5, waistY - 1);
            const qreal radius = rng.bounded(2, 5);
            painter.setBrush(QColor(112, 26, 22, 175));
            painter.drawEllipse(QPointF(bx, by), radius, radius * 0.7);
        }

        painter.setBrush(body.darker(160));
        painter.drawPolygon(QPolygon()
                            << QPoint(cx - halfBody, waistY)
                            << QPoint(cx - halfBody + 5, waistY - 9)
                            << QPoint(cx - halfBody + 10, waistY)
                            << QPoint(cx, waistY - 6)
                            << QPoint(cx + halfBody - 4, waistY));

        painter.setPen(QPen(QColor(178, 168, 138, 190), 1));
        painter.drawLine(cx - halfBody + 3, shoulderY + 10,
                         cx + halfBody - 4, shoulderY + 13);
        painter.drawLine(cx - halfBody + 4, shoulderY + 15,
                         cx + halfBody - 6, shoulderY + 18);

        painter.setPen(QPen(QColor(126, 28, 22, 220), 2));
        painter.drawLine(QPointF(headX, headY + 5),
                         QPointF(headX - 1, headY + 11));
        painter.setPen(Qt::NoPen);

        if (const auto* zombie = dynamic_cast<const Zombie*>(&entity)) {
            if (zombie->type() == ZombieType::Exploder) {
                for (int i = 0; i < 5; ++i) {
                    const qreal bx = cx + rng.bounded(-halfBody + 3, halfBody - 2);
                    const qreal by = rng.bounded(shoulderY + 8, waistY - 2);
                    painter.setBrush(QColor(150, 58, 38, 200));
                    painter.drawEllipse(QPointF(bx, by), 3, 3);
                }
            }
        }
    } else if (const auto* h = dynamic_cast<const Human*>(&entity)) {
        switch (h->type()) {
        case HumanType::Military:
            painter.setBrush(QColor(58, 64, 52));
            painter.drawEllipse(QPointF(headX, headY - 3), 9, 5);
            break;
        case HumanType::Doctor:
            painter.setBrush(QColor(150, 46, 40));
            painter.drawRect(QRectF(cx - 2, shoulderY + 7, 4, 10));
            painter.drawRect(QRectF(cx - 5, shoulderY + 10, 10, 4));
            break;
        case HumanType::Builder:
            painter.setBrush(QColor(196, 158, 58));
            painter.drawEllipse(QPointF(headX, headY - 4), 10, 4);
            painter.drawRect(QRectF(headX - 6, headY - 9, 12, 5));
            break;
        case HumanType::Farmer:
            painter.setBrush(QColor(178, 158, 96));
            painter.drawEllipse(QPointF(headX, headY - 4), 13, 4);
            painter.drawEllipse(QPointF(headX, headY - 7), 6, 4);
            break;
        }
    }
}

}  // namespace

MapView::MapView(Grid& grid, QWidget* parent)
    : QWidget(parent),
      grid_(grid),
      rainTimer_(nullptr),
      hoverRow_(-1),
      hoverCol_(-1),
      minutes_(20 * 60),
      mapScale_(1.0) {
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    buildMapCache();
    initRain();
    buildVignette();
    rebuildScene();

    rainTimer_ = new QTimer(this);
    rainTimer_->setInterval(40);
    connect(rainTimer_, &QTimer::timeout, this, &MapView::stepRain);
    rainTimer_->start();
}

QSize MapView::sizeHint() const {
    return QSize(1100, 780);
}

void MapView::setMinutes(int minutes) {
    minutes_ = minutes;
    rebuildScene();
    update();
}

void MapView::refreshWorld() {
    hoverRow_ = -1;
    hoverCol_ = -1;
    buildMapCache();
    rebuildScene();
    update();
}

void MapView::addBloodAt(int row, int col) {
    if (row < 0 || col < 0 || row >= grid_.rows() || col >= grid_.cols()) {
        return;
    }
    for (const BloodStain& stain : bloodStains_) {
        if (stain.row == row && stain.col == col) {
            return;
        }
    }
    BloodStain stain;
    stain.row = row;
    stain.col = col;
    stain.seed = static_cast<quint32>(row * 131 + col * 977 +
                                      bloodStains_.size() * 37 + 17);
    bloodStains_.push_back(stain);
}

void MapView::clearBlood() {
    bloodStains_.clear();
}

qreal MapView::daylight() const {
    const qreal hour = (minutes_ % (24 * 60)) / 60.0;
    if (hour < 5.0) {
        return 0.0;
    }
    if (hour < 8.0) {
        return (hour - 5.0) / 3.0;
    }
    if (hour < 17.0) {
        return 1.0;
    }
    if (hour < 20.0) {
        return 1.0 - (hour - 17.0) / 3.0;
    }
    return 0.0;
}

bool MapView::isNight() const {
    return daylight() < 0.35;
}

void MapView::buildMapCache() {
    const int width = (grid_.rows() + grid_.cols()) * TILE_WIDTH / 2 + 2 * CACHE_PAD;
    const int height = (grid_.rows() + grid_.cols()) * TILE_HEIGHT / 2 +
                       TILE_THICKNESS + TOP_HALF_HEIGHT + 1 + 2 * CACHE_PAD;

    mapCache_ = QPixmap(width, height);
    mapCache_.fill(Qt::transparent);

    QPainter painter(&mapCache_);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int originX = width / 2;
    const int originY = TILE_THICKNESS + TOP_HALF_HEIGHT + CACHE_PAD;

    for (int sum = 0; sum <= grid_.rows() + grid_.cols() - 2; ++sum) {
        for (int row = 0; row < grid_.rows(); ++row) {
            const int col = sum - row;
            if (col < 0 || col >= grid_.cols()) {
                continue;
            }

            const Cell* cell = grid_.at(row, col);
            const Color color = cell->color();
            const QColor base(color.r, color.g, color.b);

            const int cx = originX + (col - row) * TILE_WIDTH / 2;
            const int cy = originY + (col + row) * TILE_HEIGHT / 2;

            drawBlock(painter, cx, cy, base);

            const int topY = cy - TILE_THICKNESS;
            for (const BloodStain& stain : bloodStains_) {
                if (stain.row == row && stain.col == col) {
                    drawBloodSplat(painter, cx, topY, stain.seed, false);
                }
            }
            if (cell->prop()) {
                drawProp(painter, cx, topY, *cell->prop());
            }
            if (cell->entity()) {
                drawEntity(painter, cx, topY, *cell->entity());
            }
        }
    }
}

void MapView::updateMapPlacement() {
    if (mapCache_.isNull() || width() <= 0 || height() <= 0) {
        mapScale_ = 1.0;
        mapTopLeft_ = QPoint(0, 0);
        return;
    }

    const QRect area = rect().adjusted(MAP_MARGIN, MAP_MARGIN,
                                       -MAP_MARGIN, -MAP_MARGIN);
    if (area.width() <= 0 || area.height() <= 0) {
        mapScale_ = 1.0;
        mapTopLeft_ = QPoint(0, 0);
        return;
    }

    mapScale_ = std::min(
        {static_cast<qreal>(area.width()) / mapCache_.width(),
         static_cast<qreal>(area.height()) / mapCache_.height(),
         MAX_MAP_SCALE});

    const QSize targetSize(
        qRound(mapCache_.width() * mapScale_),
        qRound(mapCache_.height() * mapScale_));
    mapTopLeft_ = QPoint(area.center().x() - targetSize.width() / 2,
                         area.center().y() - targetSize.height() / 2);
}

void MapView::drawBackground(QPainter& painter) {
    const qreal day = daylight();

    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, mixColor(QColor(15, 19, 24), QColor(66, 70, 70), day));
    sky.setColorAt(0.45, mixColor(QColor(10, 13, 16), QColor(44, 48, 48), day));
    sky.setColorAt(1.0, mixColor(QColor(5, 7, 9), QColor(26, 29, 29), day));
    painter.fillRect(rect(), sky);

    QRandomGenerator rng(0x5EED1234u);
    painter.setPen(Qt::NoPen);

    const qreal cloudFactor = 0.7 + 0.3 * day;
    for (int i = 0; i < 32; ++i) {
        const int cx = rng.bounded(-width() / 4, width() + width() / 4);
        const int cy = rng.bounded(height() / 16, height() * 4 / 5);
        const int radius =
            rng.bounded(height() / 8, static_cast<int>(height() / 2.4));
        const int alpha = static_cast<int>(rng.bounded(18, 44) * cloudFactor);

        QRadialGradient cloud(QPointF(cx, cy), radius);
        cloud.setColorAt(0.0, QColor(126, 136, 138, alpha));
        cloud.setColorAt(0.5, QColor(86, 98, 102, alpha / 2));
        cloud.setColorAt(1.0, QColor(52, 60, 64, 0));

        painter.setBrush(cloud);
        painter.drawEllipse(QPointF(cx, cy), radius, radius * 0.5);
        painter.drawEllipse(
            QPointF(cx - radius * 0.45, cy + radius * 0.1),
            radius * 0.62, radius * 0.32);
        painter.drawEllipse(
            QPointF(cx + radius * 0.5, cy + radius * 0.06),
            radius * 0.55, radius * 0.28);
        painter.drawEllipse(
            QPointF(cx + radius * 0.1, cy - radius * 0.22),
            radius * 0.55, radius * 0.3);
    }

    const int stormAlpha = static_cast<int>(70 + 35 * (1.0 - day));
    QLinearGradient storm(0, 0, 0, height() * 0.55);
    storm.setColorAt(0.0, QColor(8, 12, 15, stormAlpha));
    storm.setColorAt(1.0, QColor(8, 12, 15, 0));
    painter.fillRect(QRect(0, 0, width(), qRound(height() * 0.55)), storm);

    const int fogAlpha = static_cast<int>(34 + 26 * day);
    QLinearGradient fog(0, height() * 0.45, 0, height());
    fog.setColorAt(0.0, QColor(100, 110, 105, 0));
    fog.setColorAt(1.0, QColor(100, 110, 105, fogAlpha));
    painter.fillRect(rect(), fog);
}

void MapView::drawMap(QPainter& painter) {
    if (mapCache_.isNull()) {
        return;
    }

    const QSize targetSize(
        qRound(mapCache_.width() * mapScale_),
        qRound(mapCache_.height() * mapScale_));

    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(QRect(mapTopLeft_, targetSize), mapCache_);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
}

void MapView::drawAmbient(QPainter& painter) {
    const int alpha = static_cast<int>(std::round(145.0 - 120.0 * daylight()));
    painter.fillRect(rect(), QColor(5, 9, 16, alpha));
}

void MapView::rebuildScene() {
    if (width() < 32 || height() < 32) {
        return;
    }

    updateMapPlacement();

    scene_ = QPixmap(size());
    QPainter painter(&scene_);
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawBackground(painter);
    drawMap(painter);
    drawAmbient(painter);
}

void MapView::buildVignette() {
    if (width() < 32 || height() < 32) {
        return;
    }

    vignette_ = QPixmap(size());
    vignette_.fill(Qt::transparent);

    QPainter painter(&vignette_);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QPointF center(width() / 2.0, height() / 2.0);
    const qreal radius = std::hypot(static_cast<qreal>(width()),
                                    static_cast<qreal>(height())) * 0.62;

    QRadialGradient vignette(center, radius);
    vignette.setColorAt(0.0, QColor(0, 0, 0, 0));
    vignette.setColorAt(0.55, QColor(0, 0, 0, 0));
    vignette.setColorAt(0.82, QColor(0, 0, 0, 70));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 185));
    painter.fillRect(rect(), vignette);
}

QPointF MapView::cacheToWidget(qreal x, qreal y) const {
    return QPointF(mapTopLeft_.x() + x * mapScale_,
                   mapTopLeft_.y() + y * mapScale_);
}

bool MapView::cellAt(const QPointF& point, int& row, int& col) const {
    if (mapCache_.isNull() || mapScale_ <= 0.0) {
        return false;
    }

    const qreal cxCache = (point.x() - mapTopLeft_.x()) / mapScale_;
    const qreal cyCache = (point.y() - mapTopLeft_.y()) / mapScale_;
    const qreal originX = mapCache_.width() / 2.0;
    const qreal planeY = TOP_HALF_HEIGHT + CACHE_PAD;

    const int offsets[] = {0, 16, 28, 40, 52, 64, 76};
    for (int offset : offsets) {
        const qreal dx = cxCache - originX;
        const qreal dy = cyCache + offset - planeY;
        const qreal u = dx / TOP_HALF_WIDTH;
        const qreal v = dy / TOP_HALF_HEIGHT;

        const int c = qRound((u + v) / 2.0);
        const int r = qRound((v - u) / 2.0);

        if (r < 0 || c < 0 || r >= grid_.rows() || c >= grid_.cols()) {
            continue;
        }

        const qreal du = u - (c - r);
        const qreal dv = v - (c + r);
        if (std::abs(du) + std::abs(dv) > 1.0) {
            continue;
        }

        if (offset > 0 && !grid_.at(r, c)->prop() && !grid_.at(r, c)->entity()) {
            continue;
        }

        row = r;
        col = c;
        return true;
    }

    return false;
}

void MapView::drawCampfireGlow(QPainter& painter) {
    const qreal dark = 1.0 - daylight();
    if (dark <= 0.05 || mapCache_.isNull()) {
        return;
    }

    int fireRow = -1;
    int fireCol = -1;
    for (int row = 0; row < grid_.rows() && fireRow < 0; ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            const Prop* prop = grid_.at(row, col)->prop();
            if (prop && prop->kind() == PropKind::Campfire) {
                fireRow = row;
                fireCol = col;
                break;
            }
        }
    }
    if (fireRow < 0) {
        return;
    }

    const qreal originX = mapCache_.width() / 2.0;
    const qreal originY = TILE_THICKNESS + TOP_HALF_HEIGHT + CACHE_PAD;
    const qreal cxCache = originX + (fireCol - fireRow) * TILE_WIDTH / 2.0;
    const qreal cyCache = originY + (fireCol + fireRow) * TILE_HEIGHT / 2.0;
    const QPointF center = cacheToWidget(cxCache, cyCache - TILE_THICKNESS + 4);

    const qreal t = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    const qreal flicker = 0.88 + 0.12 * std::sin(t * 9.0) * std::sin(t * 3.1 + 1.7);
    const qreal radius = TILE_WIDTH * 1.35 * mapScale_ * (0.98 + 0.02 * flicker);
    const int alpha = qRound(95.0 * dark * flicker);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_Plus);
    painter.setPen(Qt::NoPen);

    QRadialGradient glow(center, radius);
    glow.setColorAt(0.0, QColor(255, 178, 92, alpha));
    glow.setColorAt(0.22, QColor(214, 118, 48, qRound(alpha * 0.62)));
    glow.setColorAt(0.58, QColor(120, 58, 24, qRound(alpha * 0.22)));
    glow.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setBrush(glow);
    painter.drawEllipse(center, radius, radius * 0.62);

    QRadialGradient flame(center - QPointF(0, 9 * mapScale_), 14 * mapScale_);
    flame.setColorAt(0.0, QColor(255, 226, 150, qRound(150 * dark)));
    flame.setColorAt(0.5, QColor(242, 142, 52, qRound(110 * dark)));
    flame.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setBrush(flame);
    painter.drawEllipse(center - QPointF(0, 8 * mapScale_),
                        11 * mapScale_, 18 * mapScale_);
    painter.restore();
}

void MapView::drawHover(QPainter& painter) {
    if (hoverRow_ < 0 || hoverCol_ < 0) {
        return;
    }

    const qreal originX = mapCache_.width() / 2.0;
    const qreal planeY = TOP_HALF_HEIGHT + CACHE_PAD;
    const QPointF center = cacheToWidget(
        originX + (hoverCol_ - hoverRow_) * TOP_HALF_WIDTH,
        planeY + (hoverCol_ + hoverRow_) * TOP_HALF_HEIGHT);

    const qreal halfW = TOP_HALF_WIDTH * mapScale_;
    const qreal halfH = TOP_HALF_HEIGHT * mapScale_;

    QPolygonF diamond;
    diamond << QPointF(center.x(), center.y() - halfH)
            << QPointF(center.x() + halfW, center.y())
            << QPointF(center.x(), center.y() + halfH)
            << QPointF(center.x() - halfW, center.y());

    painter.setPen(QPen(QColor(206, 176, 132, 210), 2));
    painter.setBrush(QColor(224, 192, 140, 30));
    painter.drawPolygon(diamond);
}

void MapView::initRain() {
    drops_.clear();
    if (width() <= 0 || height() <= 0) {
        return;
    }

    QRandomGenerator rng(0x4A1F9Cu);
    const int count = std::max(36, (width() * height()) / 28000);
    drops_.reserve(count);

    for (int i = 0; i < count; ++i) {
        Drop drop;
        drop.x = rng.bounded(0, width() + 80) - 40.0;
        drop.y = rng.bounded(-height(), height());
        drop.speed = rng.bounded(380, 660);
        drop.length = rng.bounded(9, 18);
        drops_.push_back(drop);
    }
}

void MapView::stepRain() {
    if (drops_.isEmpty()) {
        return;
    }

    QRandomGenerator& rng = *QRandomGenerator::global();
    const qreal dt = 0.04;
    for (Drop& drop : drops_) {
        drop.y += drop.speed * dt;
        drop.x += drop.speed * dt * 0.16;

        if (drop.y > height() + 24 || drop.x > width() + 40) {
            drop.y = -20.0 - rng.bounded(80);
            drop.x = rng.bounded(width() + 80) - 40.0;
        }
    }

    update();
}

void MapView::drawRain(QPainter& painter) {
    if (drops_.isEmpty()) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, false);
    QVector<QLineF> normal;
    QVector<QLineF> bright;
    normal.reserve(drops_.size());
    bright.reserve(drops_.size() / 8 + 1);

    for (int i = 0; i < drops_.size(); ++i) {
        const Drop& drop = drops_[i];
        const QLineF line(QPointF(drop.x, drop.y),
                          QPointF(drop.x - drop.length * 0.18,
                                  drop.y + drop.length));
        if (i % 9 == 0) {
            bright.push_back(line);
        } else {
            normal.push_back(line);
        }
    }

    painter.setPen(QPen(QColor(158, 178, 186, 52), 1));
    painter.drawLines(normal);
    painter.setPen(QPen(QColor(170, 190, 198, 100), 1));
    painter.drawLines(bright);
}

void MapView::drawTooltip(QPainter& painter) {
    if (hoverRow_ < 0 || hoverCol_ < 0) {
        return;
    }

    const Cell* cell = grid_.at(hoverRow_, hoverCol_);
    QString title;
    QStringList lines;

    if (const Entity* entity = cell->entity()) {
        title = entity->typeName();
        lines << QStringLiteral("HP      %1").arg(entity->maxHp())
              << QStringLiteral("ATTACK  %1").arg(entity->attack())
              << QStringLiteral("DEFENSE %1").arg(entity->defense())
              << QStringLiteral("SPEED   %1").arg(entity->speed());
    } else if (const Prop* prop = cell->prop()) {
        title = prop->name();
        if (prop->kind() == PropKind::Plant) {
            lines << QStringLiteral("GROWTH  %1/3").arg(prop->growth())
                  << QStringLiteral("FOOD    %1").arg(prop->food());
        } else if (prop->food() > 0) {
            lines << QStringLiteral("FOOD    %1").arg(prop->food());
        }
    } else {
        title = QStringLiteral("Empty Ground");
        lines << QStringLiteral("Nothing here");
    }

    const qreal ui = std::clamp(mapScale_, 1.0, 2.2);

    QFont titleFont = painter.font();
    titleFont.setBold(true);
    titleFont.setPixelSize(qRound(17 * ui));

    QFont bodyFont = painter.font();
    bodyFont.setBold(false);
    bodyFont.setPixelSize(qRound(14 * ui));

    const QFontMetrics titleMetrics(titleFont);
    const QFontMetrics bodyMetrics(bodyFont);

    int textWidth = titleMetrics.horizontalAdvance(title);
    for (const QString& line : lines) {
        textWidth = std::max(textWidth, bodyMetrics.horizontalAdvance(line));
    }

    const int lineHeight = bodyMetrics.height();
    const int boxWidth = textWidth + qRound(32 * ui);
    const int boxHeight = titleMetrics.height() +
                          lineHeight * lines.size() + qRound(24 * ui);

    const int gap = qRound(24 * ui);
    const int edge = qRound(12 * ui);
    QPoint pos = mousePos_ + QPoint(gap, gap);
    pos.setX(std::clamp(pos.x(), edge, width() - edge - boxWidth));
    pos.setY(std::clamp(pos.y(), edge, height() - edge - boxHeight));

    const QRect box(pos, QSize(boxWidth, boxHeight));
    painter.setPen(QPen(QColor(122, 52, 46), 1));
    painter.setBrush(QColor(10, 12, 11, 235));
    painter.drawRoundedRect(box, qRound(4 * ui), qRound(4 * ui));

    const int padX = qRound(16 * ui);
    const int padTop = qRound(10 * ui);
    painter.setFont(titleFont);
    painter.setPen(QColor(198, 92, 76));
    painter.drawText(QRect(box.left() + padX, box.top() + padTop,
                           boxWidth - padX * 2, titleMetrics.height()),
                     Qt::AlignLeft | Qt::AlignVCenter, title);

    painter.setFont(bodyFont);
    painter.setPen(QColor(184, 178, 162));
    int y = box.top() + padTop + titleMetrics.height() + qRound(2 * ui);
    for (const QString& line : lines) {
        painter.drawText(QRect(box.left() + padX, y, boxWidth - padX * 2,
                               lineHeight),
                         Qt::AlignLeft | Qt::AlignVCenter, line);
        y += lineHeight;
    }
}

void MapView::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    if (scene_.isNull()) {
        rebuildScene();
    }

    if (!scene_.isNull()) {
        painter.drawPixmap(0, 0, scene_);
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    drawCampfireGlow(painter);
    drawHover(painter);
    drawRain(painter);

    if (!vignette_.isNull()) {
        painter.drawPixmap(rect(), vignette_);
    }

    drawTooltip(painter);
}

void MapView::mouseMoveEvent(QMouseEvent* event) {
    mousePos_ = event->pos();

    int row = -1;
    int col = -1;
    if (cellAt(event->pos(), row, col)) {
        hoverRow_ = row;
        hoverCol_ = col;
    } else {
        hoverRow_ = -1;
        hoverCol_ = -1;
    }

    update();
}

void MapView::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);
    hoverRow_ = -1;
    hoverCol_ = -1;
    update();
}

void MapView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    buildVignette();
    initRain();
    rebuildScene();
}
