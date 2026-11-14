#include "ui/ui.h"

#include <algorithm>
#include <cmath>

#include <QAbstractSpinBox>
#include <QApplication>
#include <QButtonGroup>
#include <QColor>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineF>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QPolygon>
#include <QPolygonF>
#include <QPushButton>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QScreen>
#include <QSpinBox>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

namespace {

constexpr int TILE_WIDTH = 144;
constexpr int TILE_HEIGHT = 72;
constexpr int TILE_THICKNESS = 44;
constexpr int TOP_HALF_WIDTH = 72;
constexpr int TOP_HALF_HEIGHT = 36;
constexpr int CACHE_PAD = 2;
constexpr int MAP_MARGIN = 36;
constexpr qreal MAX_MAP_SCALE = 2.6;
constexpr qreal PI = 3.14159265358979323846;

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

bool isHumanSprite(Sprite sprite) {
    return sprite == Sprite::Farmer || sprite == Sprite::Doctor ||
           sprite == Sprite::Military || sprite == Sprite::Builder;
}

QString displayFontFallback() {
    const QStringList extraDirs = {
        QCoreApplication::applicationDirPath() + QStringLiteral("/fonts"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../fonts"),
        QDir::currentPath() + QStringLiteral("/fonts")};

    for (const QString& path : extraDirs) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }
        const QStringList files =
            dir.entryList({QStringLiteral("*.ttf"), QStringLiteral("*.otf")},
                          QDir::Files);
        for (const QString& file : files) {
            const int id = QFontDatabase::addApplicationFont(dir.filePath(file));
            if (id < 0) {
                continue;
            }
            const QStringList families =
                QFontDatabase::applicationFontFamilies(id);
            if (!families.isEmpty()) {
                return families.first();
            }
        }
    }

    const QStringList preferred = {
        QStringLiteral("Creepster"), QStringLiteral("Nosifer"),
        QStringLiteral("Chiller"), QStringLiteral("Jokerman"),
        QStringLiteral("Impact"), QStringLiteral("Stencil"),
        QStringLiteral("Bahnschrift SemiBold"), QStringLiteral("Arial Black"),
        QStringLiteral("DejaVu Sans Condensed")};

    const QStringList families = QFontDatabase::families();
    for (const QString& name : preferred) {
        if (families.contains(name, Qt::CaseInsensitive)) {
            return name;
        }
    }
    return QApplication::font().family();
}

void drawBlock(QPainter& painter, int cx, int baseCenterY, const QColor& base) {
    const int topY = baseCenterY - TILE_THICKNESS;

    QPolygon left;
    left << QPoint(cx - TOP_HALF_WIDTH, topY)
         << QPoint(cx, topY + TOP_HALF_HEIGHT)
         << QPoint(cx, baseCenterY + TOP_HALF_HEIGHT)
         << QPoint(cx - TOP_HALF_WIDTH, baseCenterY);

    QPolygon right;
    right << QPoint(cx, topY + TOP_HALF_HEIGHT)
          << QPoint(cx + TOP_HALF_WIDTH, topY)
          << QPoint(cx + TOP_HALF_WIDTH, baseCenterY)
          << QPoint(cx, baseCenterY + TOP_HALF_HEIGHT);

    painter.setPen(Qt::NoPen);
    painter.setBrush(darker(base, 48));
    painter.drawPolygon(left);
    painter.setBrush(darker(base, 28));
    painter.drawPolygon(right);

    QPolygon upper;
    upper << QPoint(cx, topY - TOP_HALF_HEIGHT)
          << QPoint(cx + TOP_HALF_WIDTH, topY)
          << QPoint(cx - TOP_HALF_WIDTH, topY);

    QPolygon lower;
    lower << QPoint(cx - TOP_HALF_WIDTH, topY)
          << QPoint(cx + TOP_HALF_WIDTH, topY)
          << QPoint(cx, topY + TOP_HALF_HEIGHT);

    painter.setBrush(base.lighter(106));
    painter.drawPolygon(upper);
    painter.setBrush(base);
    painter.drawPolygon(lower);

    QPolygon top;
    top << QPoint(cx, topY - TOP_HALF_HEIGHT)
        << QPoint(cx + TOP_HALF_WIDTH, topY)
        << QPoint(cx, topY + TOP_HALF_HEIGHT)
        << QPoint(cx - TOP_HALF_WIDTH, topY);

    painter.setPen(QPen(darker(base, 55), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPolygon(top);
}

void drawWall(QPainter& painter, int cx, int topY, const Wall& wall) {
    const int wallHeight = 22;
    const QPoint top(cx, topY - TOP_HALF_HEIGHT);
    const QPoint left(cx - TOP_HALF_WIDTH, topY);
    const QPoint right(cx + TOP_HALF_WIDTH, topY);

    if (wall.isDestroyed()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(48, 42, 34, 170));
        painter.drawEllipse(QPointF(cx - 18, topY - 2), 12, 4);
        painter.drawEllipse(QPointF(cx + 14, topY - 1), 9, 3);
        painter.setBrush(QColor(78, 68, 56));
        painter.drawPolygon(QPolygon()
                            << QPoint(cx - 26, topY + 2)
                            << QPoint(cx - 12, topY - 8)
                            << QPoint(cx + 1, topY + 3));
        return;
    }

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

    if (wall.durability() * 2 < wall.maxDurability()) {
        painter.setPen(QPen(QColor(34, 29, 24, 190), 1));
        painter.drawLine(cx - 26, topY - 16, cx - 14, topY - 24);
        painter.drawLine(cx + 8, topY - 12, cx + 22, topY - 24);
        painter.setPen(Qt::NoPen);
    }
}

void drawPlant(QPainter& painter, int cx, int topY, const Plant& plant) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 55));
    painter.drawEllipse(QPointF(cx, topY + 1), 13, 4);

    painter.setBrush(QColor(56, 46, 34));
    painter.drawEllipse(QPointF(cx - 9, topY), 8, 3);
    painter.drawEllipse(QPointF(cx + 9, topY), 8, 3);

    const int leaves = 3 + plant.growth();
    for (int i = 0; i < leaves; ++i) {
        const qreal dx = (i - (leaves - 1) / 2.0) * 7.0;
        const qreal tipX = cx + dx * 1.4;
        const qreal tipY = topY - 9 - plant.growth() * 3;
        painter.setPen(QPen(QColor(66, 102, 54), 2));
        painter.drawLine(QPointF(cx + dx, topY), QPointF(tipX, tipY));
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(78, 118, 62));
        painter.drawEllipse(QPointF(tipX, tipY), 3.5, 2.5);
    }

    if (plant.growth() >= 2) {
        painter.setBrush(QColor(176, 74, 52));
        painter.drawEllipse(QPointF(cx - 5, topY - 14), 2, 2);
        painter.drawEllipse(QPointF(cx + 6, topY - 16), 2, 2);
    }
}

void drawTree(QPainter& painter, int cx, int topY) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 80));
    painter.drawEllipse(QPointF(cx, topY + 2), 16, 6);
    painter.setBrush(QColor(54, 46, 38));
    painter.drawPolygon(QPolygon()
                        << QPoint(cx - 4, topY) << QPoint(cx + 4, topY)
                        << QPoint(cx + 3, topY - 36)
                        << QPoint(cx - 3, topY - 36));
    painter.setPen(QPen(QColor(54, 46, 38), 2));
    painter.drawLine(cx - 2, topY - 26, cx - 14, topY - 38);
    painter.drawLine(cx + 2, topY - 30, cx + 13, topY - 42);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(38, 50, 38));
    painter.drawEllipse(QPointF(cx - 12, topY - 42), 14, 10);
    painter.drawEllipse(QPointF(cx + 12, topY - 45), 15, 10);
    painter.setBrush(QColor(48, 62, 46));
    painter.drawEllipse(QPointF(cx, topY - 52), 16, 11);
}

void drawBush(QPainter& painter, int cx, int topY) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 70));
    painter.drawEllipse(QPointF(cx, topY + 2), 24, 9);
    painter.setBrush(QColor(42, 56, 40));
    painter.drawEllipse(QPointF(cx - 9, topY - 6), 13, 11);
    painter.drawEllipse(QPointF(cx + 8, topY - 8), 12, 10);
    painter.setBrush(QColor(56, 72, 50));
    painter.drawEllipse(QPointF(cx, topY - 14), 13, 11);
}

void drawCorpse(QPainter& painter, int cx, int topY) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 70));
    painter.drawEllipse(QPointF(cx, topY + 2), 19, 7);
    painter.setBrush(QColor(70, 72, 60));
    painter.drawEllipse(QPointF(cx - 3, topY - 3), 11, 5);
    painter.setBrush(QColor(96, 92, 80));
    painter.drawEllipse(QPointF(cx + 8, topY - 5), 6, 5);
}

void drawCampfire(QPainter& painter, int cx, int topY) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 80));
    painter.drawEllipse(QPointF(cx, topY + 2), 22, 8);
    for (int i = 0; i < 8; ++i) {
        const qreal angle = i * PI / 4.0;
        painter.setBrush(QColor(84, 82, 76));
        painter.drawEllipse(QPointF(cx + std::cos(angle) * 17.0,
                                    topY + std::sin(angle) * 8.0),
                            4.5, 3.5);
    }
    painter.setBrush(QColor(64, 44, 30));
    painter.drawPolygon(QPolygon()
                        << QPoint(cx - 16, topY - 2)
                        << QPoint(cx + 16, topY - 9)
                        << QPoint(cx + 18, topY - 4)
                        << QPoint(cx - 14, topY + 3));
    painter.drawPolygon(QPolygon()
                        << QPoint(cx - 16, topY - 9)
                        << QPoint(cx + 16, topY - 2)
                        << QPoint(cx + 14, topY + 3)
                        << QPoint(cx - 18, topY - 4));
    painter.setBrush(QColor(196, 92, 38));
    painter.drawEllipse(QPointF(cx - 5, topY - 6), 4, 2.2);
    painter.drawEllipse(QPointF(cx + 6, topY - 8), 3.5, 2.0);
    painter.setBrush(QColor(232, 150, 54));
    painter.drawEllipse(QPointF(cx + 1, topY - 10), 5, 3);
}

void drawDecoration(QPainter& painter, int cx, int topY, Decoration decoration) {
    switch (decoration) {
    case Decoration::Tree:
        drawTree(painter, cx, topY);
        break;
    case Decoration::Bush:
        drawBush(painter, cx, topY);
        break;
    case Decoration::Corpse:
        drawCorpse(painter, cx, topY);
        break;
    case Decoration::Campfire:
        drawCampfire(painter, cx, topY);
        break;
    case Decoration::None:
        break;
    }
}

void drawBloodSplat(QPainter& painter, int cx, int groundY, quint32 seed) {
    QRandomGenerator rng(seed);
    painter.save();
    painter.setPen(Qt::NoPen);

    for (int i = 0; i < 8; ++i) {
        const qreal ox = rng.bounded(-44, 45);
        const qreal oy = rng.bounded(-11, 18);
        const qreal rx = rng.bounded(5, 17);
        const qreal ry = rx * (0.28 + rng.generateDouble() * 0.24);
        const int alpha = rng.bounded(75, 140);
        painter.setBrush(QColor(104, 24, 20, alpha));
        painter.drawEllipse(QPointF(cx + ox, groundY + oy), rx, ry);
    }

    painter.setPen(QPen(QColor(88, 18, 16, 110), 2));
    const int streaks = rng.bounded(2, 5);
    for (int i = 0; i < streaks; ++i) {
        const qreal angle = rng.generateDouble() * 6.283185307179586;
        const qreal length = rng.bounded(8, 30);
        const qreal sx = cx + rng.bounded(-9, 10);
        const qreal sy = groundY + rng.bounded(-4, 8);
        painter.drawLine(QPointF(sx, sy),
                         QPointF(sx + std::cos(angle) * length,
                                 sy + std::sin(angle) * length * 0.35));
    }
    painter.restore();
}

void drawEntity(QPainter& painter, int cx, int topY, const Entity& entity) {
    const Sprite sprite = entity.sprite();
    const bool human = isHumanSprite(sprite);
    const QColor body = entity.color();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 70));
    painter.drawEllipse(QPointF(cx, topY + 1), human ? 16 : 17, 6);

    int headY = topY - 50;
    int shoulderY = topY - 43;
    int waistY = topY - 18;
    int halfBody = human ? 10 : 11;
    bool lean = false;

    switch (sprite) {
    case Sprite::Tank:
        halfBody = 15;
        shoulderY = topY - 52;
        headY = topY - 60;
        break;
    case Sprite::Runner:
        halfBody = 9;
        lean = true;
        break;
    case Sprite::Exploder:
        halfBody = 14;
        shoulderY = topY - 46;
        headY = topY - 54;
        break;
    default:
        break;
    }

    painter.setPen(QPen(body.darker(140), 4));
    painter.drawLine(cx - 5, waistY, cx - 6, topY);
    painter.drawLine(cx + 5, waistY, cx + 6, topY);
    painter.setPen(Qt::NoPen);

    QPolygon torso;
    torso << QPoint(cx - halfBody, waistY) << QPoint(cx + halfBody, waistY)
          << QPoint(cx + halfBody - 1, shoulderY)
          << QPoint(cx - halfBody + 1, shoulderY);
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

        if (sprite == Sprite::Exploder) {
            for (int i = 0; i < 5; ++i) {
                const qreal bx = cx + rng.bounded(-halfBody + 3, halfBody - 2);
                const qreal by = rng.bounded(shoulderY + 8, waistY - 2);
                painter.setBrush(QColor(150, 58, 38, 200));
                painter.drawEllipse(QPointF(bx, by), 3, 3);
            }
        }
    } else {
        switch (sprite) {
        case Sprite::Military:
            painter.setBrush(QColor(58, 64, 52));
            painter.drawEllipse(QPointF(headX, headY - 3), 9, 5);
            break;
        case Sprite::Doctor:
            painter.setBrush(QColor(150, 46, 40));
            painter.drawRect(QRectF(cx - 2, shoulderY + 7, 4, 10));
            painter.drawRect(QRectF(cx - 5, shoulderY + 10, 10, 4));
            break;
        case Sprite::Builder:
            painter.setBrush(QColor(196, 158, 58));
            painter.drawEllipse(QPointF(headX, headY - 4), 10, 4);
            painter.drawRect(QRectF(headX - 6, headY - 9, 12, 5));
            break;
        case Sprite::Farmer:
            painter.setBrush(QColor(178, 158, 96));
            painter.drawEllipse(QPointF(headX, headY - 4), 13, 4);
            painter.drawEllipse(QPointF(headX, headY - 7), 6, 4);
            break;
        default:
            break;
        }
    }

    if (entity.health() < entity.maxHealth()) {
        const int barWidth = halfBody * 2;
        const int barHeight = 4;
        const int barY = headY - 14;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(18, 18, 18, 210));
        painter.drawRect(QRectF(cx - barWidth / 2.0, barY, barWidth, barHeight));
        painter.setBrush(human ? QColor(118, 168, 104) : QColor(176, 54, 42));
        const qreal ratio =
            static_cast<qreal>(entity.health()) / entity.maxHealth();
        painter.drawRect(QRectF(cx - barWidth / 2.0, barY,
                                barWidth * ratio, barHeight));
    }
}

void drawSplat(QPainter& painter, qreal cx, qreal cy, qreal radius,
               quint32 seed) {
    QRandomGenerator rng(seed);
    painter.setPen(Qt::NoPen);

    for (int i = 0; i < 7; ++i) {
        const qreal ox = (rng.generateDouble() * 2.0 - 1.0) * radius;
        const qreal oy = (rng.generateDouble() * 2.0 - 1.0) * radius * 0.5;
        const qreal size = radius * (0.08 + rng.generateDouble() * 0.16);
        painter.setBrush(QColor(92, 20, 17, 90 + rng.bounded(60)));
        painter.drawEllipse(QPointF(cx + ox, cy + oy), size, size * 0.7);
    }

    painter.setBrush(QColor(82, 18, 15, 150));
    for (int i = 0; i < 3; ++i) {
        const qreal dx = (rng.generateDouble() * 2.0 - 1.0) * radius * 0.5;
        const qreal width = 2.0 + rng.generateDouble() * 3.0;
        const qreal length = radius * (0.6 + rng.generateDouble() * 1.2);
        painter.drawRoundedRect(QRectF(cx + dx, cy, width, length),
                                width / 2.0, width / 2.0);
        painter.drawEllipse(QPointF(cx + dx + width / 2.0, cy + length),
                            width * 0.6, width * 0.5);
    }
}

void paintGrime(QPainter& painter, const QRect& rect, quint32 seed,
                int splats) {
    QRandomGenerator rng(seed);
    painter.setPen(Qt::NoPen);

    for (int i = 0; i < rect.width() / 5; ++i) {
        const int x = rect.left() + rng.bounded(rect.width());
        const int y = rect.top() + rng.bounded(rect.height());
        painter.setBrush(QColor(0, 0, 0, rng.bounded(4, 16)));
        painter.drawRect(x, y, rng.bounded(1, 4), 1);
    }

    painter.setPen(QPen(QColor(60, 64, 58, 40), 1));
    for (int i = 0; i < 7; ++i) {
        const int x = rect.left() + rng.bounded(rect.width());
        const int y = rect.top() + rng.bounded(rect.height());
        painter.drawLine(x, y, x + rng.bounded(10, 40),
                         y + rng.bounded(-2, 3));
    }
    painter.setPen(Qt::NoPen);

    for (int i = 0; i < splats; ++i) {
        const qreal cx = rect.left() + rect.width() * (0.06 + 0.2 * i);
        const qreal cy = rect.top() + rect.height() * (i % 2 == 0 ? 0.55 : 0.28);
        drawSplat(painter, cx, cy, rect.height() * 0.42,
                  seed + static_cast<quint32>(i) * 17u);
    }
}

void drawZombieSilhouette(QPainter& painter, const QPointF& base,
                          qreal scale) {
    painter.save();
    painter.translate(base);
    painter.scale(scale, scale);
    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(0, 0, 0, 120));
    painter.drawEllipse(QPointF(0, 0), 24, 7);

    const QColor body(11, 13, 12);
    painter.setBrush(body);
    painter.drawRoundedRect(QRectF(-10, -58, 20, 44), 7, 7);
    painter.setBrush(QColor(13, 15, 14));
    painter.drawEllipse(QPointF(0, -64), 10, 11);

    painter.setPen(QPen(body, 7, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(-8, -50), QPointF(-24, -40));
    painter.drawLine(QPointF(8, -50), QPointF(26, -46));
    painter.drawLine(QPointF(-6, -16), QPointF(-10, 0));
    painter.drawLine(QPointF(6, -16), QPointF(12, 0));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(154, 40, 32, 210));
    painter.drawEllipse(QPointF(-4, -66), 1.8, 1.8);
    painter.drawEllipse(QPointF(4, -66), 1.8, 1.8);
    painter.restore();
}

void drawSurvivor(QPainter& painter, const QPointF& base, qreal scale) {
    painter.save();
    painter.translate(base);
    painter.scale(scale, scale);
    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(0, 0, 0, 140));
    painter.drawEllipse(QPointF(0, 2), 26, 8);

    painter.setBrush(QColor(26, 25, 27));
    painter.drawRoundedRect(QRectF(-16, -40, 32, 40), 12, 12);
    painter.setBrush(QColor(31, 30, 29));
    painter.drawEllipse(QPointF(-2, -46), 11, 12);
    painter.setBrush(QColor(176, 150, 122));
    painter.drawEllipse(QPointF(-3, -43), 7, 8);
    painter.setBrush(QColor(28, 26, 24));
    painter.drawEllipse(QPointF(-3, -50), 8, 7);

    painter.setBrush(QColor(22, 21, 22));
    painter.drawRoundedRect(QRectF(-18, -14, 16, 14), 6, 6);
    painter.drawRoundedRect(QRectF(2, -14, 16, 14), 6, 6);

    painter.setPen(QPen(QColor(22, 21, 22), 4, Qt::SolidLine,
                        Qt::RoundCap));
    painter.drawLine(QPointF(-12, -30), QPointF(-2, -17));
    painter.drawLine(QPointF(8, -32), QPointF(0, -17));
    painter.restore();
}

void paintMenuScene(QPainter& painter, const QRect& rect) {
    const qreal sceneWidth =
        std::min(rect.width() * 0.36, rect.height() * 0.78);
    if (sceneWidth < 200.0) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal k = sceneWidth / 640.0;
    painter.translate(rect.width() * 0.5, rect.height() * 0.63);
    painter.scale(k, k);

    painter.setPen(Qt::NoPen);
    QRadialGradient warmth(QPointF(10, -30), 330);
    warmth.setColorAt(0.0, QColor(198, 120, 56, 74));
    warmth.setColorAt(0.5, QColor(92, 48, 26, 32));
    warmth.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setBrush(warmth);
    painter.drawEllipse(QPointF(0, -20), 340, 250);

    drawZombieSilhouette(painter, QPointF(-300, 40), 0.85);
    drawZombieSilhouette(painter, QPointF(268, -38), 0.75);

    const qreal halfWidth = 175.0;
    const qreal halfHeight = 88.0;
    const qreal wallHeight = 92.0;
    const QPointF top(0, -halfHeight);
    const QPointF right(halfWidth, 0);
    const QPointF bottom(0, halfHeight);
    const QPointF leftPoint(-halfWidth, 0);

    QPolygonF floor;
    floor << top << right << bottom << leftPoint;
    painter.setPen(QPen(QColor(17, 15, 13), 2));
    painter.setBrush(QColor(46, 40, 32));
    painter.drawPolygon(floor);

    painter.setPen(QPen(QColor(66, 57, 45, 150), 1));
    for (int i = 1; i <= 3; ++i) {
        const qreal t = i / 4.0;
        painter.drawLine(
            QPointF(top.x() + (right.x() - top.x()) * t,
                    top.y() + (right.y() - top.y()) * t),
            QPointF(leftPoint.x() + (bottom.x() - leftPoint.x()) * t,
                    leftPoint.y() + (bottom.y() - leftPoint.y()) * t));
        painter.drawLine(
            QPointF(top.x() + (leftPoint.x() - top.x()) * t,
                    top.y() + (leftPoint.y() - top.y()) * t),
            QPointF(right.x() + (bottom.x() - right.x()) * t,
                    right.y() + (bottom.y() - right.y()) * t));
    }

    QPolygonF leftWall;
    leftWall << top << leftPoint
             << QPointF(leftPoint.x(), leftPoint.y() - wallHeight)
             << QPointF(top.x(), top.y() - wallHeight);
    painter.setPen(QPen(QColor(14, 13, 11), 2));
    painter.setBrush(QColor(30, 27, 23));
    painter.drawPolygon(leftWall);

    QPolygonF rightWall;
    rightWall << top << right
              << QPointF(right.x(), right.y() - wallHeight)
              << QPointF(top.x(), top.y() - wallHeight);
    painter.setBrush(QColor(23, 21, 18));
    painter.drawPolygon(rightWall);

    painter.setPen(QPen(QColor(48, 42, 34, 130), 1));
    for (int i = 1; i < 5; ++i) {
        const qreal t = i / 5.0;
        const QPointF a(top.x() + (leftPoint.x() - top.x()) * t,
                        top.y() + (leftPoint.y() - top.y()) * t);
        painter.drawLine(a, QPointF(a.x(), a.y() - wallHeight));
        const QPointF b(top.x() + (right.x() - top.x()) * t,
                        top.y() + (right.y() - top.y()) * t);
        painter.drawLine(b, QPointF(b.x(), b.y() - wallHeight));
    }

    const QPointF lampPos(24, -halfHeight - wallHeight + 22);
    painter.setPen(QPen(QColor(18, 16, 13), 2));
    painter.drawLine(lampPos, lampPos + QPointF(0, -26));
    QRadialGradient lampGlow(lampPos, 190);
    lampGlow.setColorAt(0.0, QColor(236, 176, 104, 90));
    lampGlow.setColorAt(0.45, QColor(160, 100, 52, 34));
    lampGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(lampGlow);
    painter.drawEllipse(lampPos, 190, 150);
    painter.setBrush(QColor(236, 182, 112));
    painter.drawEllipse(lampPos, 6, 8);

    painter.setBrush(QColor(72, 54, 40, 120));
    painter.drawEllipse(QPointF(30, 34), 92, 40);

    QPolygonF bed;
    bed << QPointF(-124, 16) << QPointF(-78, 42)
        << QPointF(-42, 24) << QPointF(-88, -2);
    painter.setPen(QPen(QColor(30, 24, 20), 2));
    painter.setBrush(QColor(62, 46, 39));
    painter.drawPolygon(bed);
    painter.setBrush(QColor(82, 66, 54));
    painter.drawEllipse(QPointF(-112, 14), 14, 8);

    painter.setPen(QPen(QColor(26, 21, 17), 2));
    painter.setBrush(QColor(56, 44, 32));
    painter.drawRoundedRect(QRectF(108, -8, 44, 34), 3, 3);
    painter.setBrush(QColor(44, 35, 26));
    painter.drawRect(QRectF(112, -18, 36, 12));

    drawSurvivor(painter, QPointF(18, 62), 1.05);

    painter.setPen(QPen(QColor(58, 45, 32), 5, Qt::SolidLine,
                        Qt::RoundCap));
    painter.drawLine(QPointF(-92, 92), QPointF(-30, 66));
    painter.drawLine(QPointF(-88, 70), QPointF(-36, 96));

    drawZombieSilhouette(painter, QPointF(-214, 152), 1.05);
    drawZombieSilhouette(painter, QPointF(232, 128), 1.0);
    drawZombieSilhouette(painter, QPointF(96, 214), 1.2);

    QRandomGenerator rng(0x5EEDu);
    for (int i = 0; i < 26; ++i) {
        const qreal x = -260.0 + rng.bounded(520);
        const qreal y = -120.0 + rng.bounded(300);
        painter.setBrush(QColor(214, 120, 58,
                                rng.bounded(20, 70)));
        painter.drawEllipse(QPointF(x, y), 1.4, 1.4);
    }

    painter.restore();
}

}  // namespace

HudBar::HudBar(bool top, QWidget* parent)
    : QWidget(parent), top_(top) {
    setAttribute(Qt::WA_StyledBackground, false);
}

void HudBar::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, top_ ? QColor(12, 15, 14) : QColor(9, 11, 10));
    bg.setColorAt(1.0, top_ ? QColor(7, 9, 9) : QColor(6, 8, 7));
    painter.fillRect(rect(), bg);

    painter.setPen(QPen(QColor(28, 33, 32), 1));
    if (top_) {
        painter.drawLine(0, height() - 1, width(), height() - 1);
    } else {
        painter.drawLine(0, 0, width(), 0);
    }

    paintGrime(painter, rect(), top_ ? 0xB100D1u : 0xB100D2u, 2);
}

ClockWidget::ClockWidget(QWidget* parent)
    : QWidget(parent), minutes_(7 * 60) {
    setMinimumSize(48, 48);
}

void ClockWidget::setMinutes(int minutes) {
    if (minutes_ == minutes) {
        return;
    }
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
        const qreal radians = (angle - 90.0) * PI / 180.0;
        const QPointF from(center.x() + std::cos(radians) * inner,
                           center.y() + std::sin(radians) * inner);
        const QPointF to(center.x() + std::cos(radians) * (radius - 5.0),
                         center.y() + std::sin(radians) * (radius - 5.0));
        painter.setPen(QPen(major ? QColor(172, 62, 52) : QColor(116, 44, 38),
                            major ? 3.0 : 1.5));
        painter.drawLine(from, to);
    }

    const qreal minute = minutes_ % 60;
    const qreal hour = (minutes_ / 60) % 12 + minute / 60.0;
    const qreal minuteAngle = (minute * 6.0 - 90.0) * PI / 180.0;
    const qreal hourAngle = (hour * 30.0 - 90.0) * PI / 180.0;

    painter.setPen(QPen(QColor(160, 48, 40),
                        std::max(3.0, radius * 0.075), Qt::SolidLine,
                        Qt::RoundCap));
    painter.drawLine(center, QPointF(center.x() + std::cos(hourAngle) *
                                                  radius * 0.46,
                                     center.y() + std::sin(hourAngle) *
                                                  radius * 0.46));

    painter.setPen(QPen(QColor(186, 62, 50),
                        std::max(2.0, radius * 0.042), Qt::SolidLine,
                        Qt::RoundCap));
    painter.drawLine(center, QPointF(center.x() + std::cos(minuteAngle) *
                                                  radius * 0.72,
                                     center.y() + std::sin(minuteAngle) *
                                                  radius * 0.72));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(150, 44, 38));
    painter.drawEllipse(center, std::max(3.0, radius * 0.08),
                        std::max(3.0, radius * 0.08));
}

MapView::MapView(Game& game, QWidget* parent)
    : QWidget(parent),
      game_(game),
      rainTimer_(nullptr),
      hoverRow_(-1),
      hoverCol_(-1),
      mapScale_(1.0),
      fireRow_(-1),
      fireCol_(-1) {
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::NoFocus);

    buildMapLayer();
    initRain();

    rainTimer_ = new QTimer(this);
    rainTimer_->setInterval(50);
    connect(rainTimer_, &QTimer::timeout, this, &MapView::stepRain);
    rainClock_.start();
}

QSize MapView::sizeHint() const {
    return QSize(1100, 780);
}

void MapView::refreshWorld() {
    hoverRow_ = -1;
    hoverCol_ = -1;
    buildMapLayer();
    rebuildStatic();
    update();
}

void MapView::syncWeather() {
    const bool shouldRain = game_.isRunning() && game_.rainIntensity() > 0.0;
    if (shouldRain) {
        if (!rainTimer_->isActive()) {
            rainClock_.restart();
            rainTimer_->start();
        }
    } else {
        rainTimer_->stop();
    }
}

void MapView::buildMapLayer() {
    const int width =
        (game_.grid().rows() + game_.grid().cols()) * TILE_WIDTH / 2 +
        2 * CACHE_PAD;
    const int height =
        (game_.grid().rows() + game_.grid().cols()) * TILE_HEIGHT / 2 +
        TILE_THICKNESS + TOP_HALF_HEIGHT + 1 + 2 * CACHE_PAD;

    mapLayer_ = QPixmap(width, height);
    mapLayer_.fill(Qt::transparent);

    QPainter painter(&mapLayer_);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int originX = width / 2;
    const int originY = TILE_THICKNESS + TOP_HALF_HEIGHT + CACHE_PAD;

    fireRow_ = -1;
    fireCol_ = -1;

    for (int sum = 0; sum <= game_.grid().rows() + game_.grid().cols() - 2;
         ++sum) {
        for (int row = 0; row < game_.grid().rows(); ++row) {
            const int col = sum - row;
            if (col < 0 || col >= game_.grid().cols()) {
                continue;
            }

            const Cell& cell = game_.grid().cell(row, col);
            const int cx = originX + (col - row) * TILE_WIDTH / 2;
            const int cy = originY + (col + row) * TILE_HEIGHT / 2;
            const int topY = cy - TILE_THICKNESS;

            drawBlock(painter, cx, cy, QColor(58, 72, 54));

            for (const QPoint& spot : game_.blood()) {
                if (spot.x() == col && spot.y() == row) {
                    quint32 seed = static_cast<quint32>(row * 131 + col * 977 +
                                                       19);
                    drawBloodSplat(painter, cx, topY, seed);
                    break;
                }
            }

            if (const Wall* wall = cell.wall()) {
                drawWall(painter, cx, topY, *wall);
            }
            if (const Plant* plant = cell.plant()) {
                drawPlant(painter, cx, topY, *plant);
            }
            if (cell.decoration() == Decoration::Campfire) {
                fireRow_ = row;
                fireCol_ = col;
            }
            drawDecoration(painter, cx, topY, cell.decoration());
            if (const Entity* entity = cell.occupant()) {
                drawEntity(painter, cx, topY, *entity);
            }
        }
    }
}

void MapView::updatePlacement() {
    if (mapLayer_.isNull() || width() <= 0 || height() <= 0) {
        mapScale_ = 1.0;
        mapTopLeft_ = QPoint(0, 0);
        return;
    }

    const QRect area =
        rect().adjusted(MAP_MARGIN, MAP_MARGIN, -MAP_MARGIN, -MAP_MARGIN);
    if (area.width() <= 0 || area.height() <= 0) {
        mapScale_ = 1.0;
        mapTopLeft_ = QPoint(0, 0);
        return;
    }

    mapScale_ = std::min(
        {static_cast<qreal>(area.width()) / mapLayer_.width(),
         static_cast<qreal>(area.height()) / mapLayer_.height(),
         MAX_MAP_SCALE});

    const QSize targetSize(
        qRound(mapLayer_.width() * mapScale_),
        qRound(mapLayer_.height() * mapScale_));
    mapTopLeft_ = QPoint(area.center().x() - targetSize.width() / 2,
                         area.center().y() - targetSize.height() / 2);
}

qreal MapView::daylight() const {
    const qreal hour = game_.minutesOfDay() / 60.0;
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
        painter.drawEllipse(QPointF(cx - radius * 0.45, cy + radius * 0.1),
                            radius * 0.62, radius * 0.32);
        painter.drawEllipse(QPointF(cx + radius * 0.5, cy + radius * 0.06),
                            radius * 0.55, radius * 0.28);
        painter.drawEllipse(QPointF(cx + radius * 0.1, cy - radius * 0.22),
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
    if (mapLayer_.isNull()) {
        return;
    }
    const QSize targetSize(
        qRound(mapLayer_.width() * mapScale_),
        qRound(mapLayer_.height() * mapScale_));
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(QRect(mapTopLeft_, targetSize), mapLayer_);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
}

void MapView::drawAmbient(QPainter& painter) {
    const int alpha = static_cast<int>(std::round(145.0 - 120.0 * daylight()));
    painter.fillRect(rect(), QColor(5, 9, 16, alpha));
}

void MapView::rebuildStatic() {
    if (width() < 32 || height() < 32) {
        return;
    }

    updatePlacement();

    staticScene_ = QPixmap(size());
    QPainter painter(&staticScene_);
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
                                    static_cast<qreal>(height())) *
                         0.62;

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
    if (mapLayer_.isNull() || mapScale_ <= 0.0) {
        return false;
    }

    const qreal cxCache = (point.x() - mapTopLeft_.x()) / mapScale_;
    const qreal cyCache = (point.y() - mapTopLeft_.y()) / mapScale_;
    const qreal originX = mapLayer_.width() / 2.0;
    const qreal planeY = TOP_HALF_HEIGHT + CACHE_PAD;

    const int offsets[] = {0, 16, 28, 40, 52, 64, 76};
    for (int offset : offsets) {
        const qreal dx = cxCache - originX;
        const qreal dy = cyCache + offset - planeY;
        const qreal u = dx / TOP_HALF_WIDTH;
        const qreal v = dy / TOP_HALF_HEIGHT;

        const int c = qRound((u + v) / 2.0);
        const int r = qRound((v - u) / 2.0);

        if (r < 0 || c < 0 || r >= game_.grid().rows() ||
            c >= game_.grid().cols()) {
            continue;
        }

        const qreal du = u - (c - r);
        const qreal dv = v - (c + r);
        if (std::abs(du) + std::abs(dv) > 1.0) {
            continue;
        }

        const Cell& cell = game_.grid().cell(r, c);
        if (offset > 0 && !cell.wall() && !cell.plant() &&
            !cell.occupant()) {
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
    if (dark <= 0.05 || fireRow_ < 0 || mapLayer_.isNull()) {
        return;
    }

    const qreal originX = mapLayer_.width() / 2.0;
    const qreal originY = TILE_THICKNESS + TOP_HALF_HEIGHT + CACHE_PAD;
    const qreal cxCache = originX + (fireCol_ - fireRow_) * TILE_WIDTH / 2.0;
    const qreal cyCache = originY + (fireCol_ + fireRow_) * TILE_HEIGHT / 2.0;
    const QPointF center =
        cacheToWidget(cxCache, cyCache - TILE_THICKNESS + 4);

    const qreal t = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    const qreal flicker =
        0.88 + 0.12 * std::sin(t * 9.0) * std::sin(t * 3.1 + 1.7);
    const qreal radius =
        TILE_WIDTH * 1.35 * mapScale_ * (0.98 + 0.02 * flicker);
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

    QRadialGradient flame(center - QPointF(0, 9 * mapScale_),
                          14 * mapScale_);
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

    const qreal originX = mapLayer_.width() / 2.0;
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
    const int count = std::max(40, (width() * height()) / 26000);
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
    const qreal dt = std::clamp(rainClock_.restart() / 1000.0, 0.01, 0.18);
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
    const qreal intensity = game_.rainIntensity();
    if (drops_.isEmpty() || intensity <= 0.0) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, false);
    QVector<QLineF> normal;
    QVector<QLineF> bright;
    normal.reserve(drops_.size());
    bright.reserve(drops_.size() / 8 + 1);

    for (int i = 0; i < drops_.size(); ++i) {
        if (intensity < 0.5 && i % 2 == 0) {
            continue;
        }
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

    const int normalAlpha = qRound(38 + 30 * intensity);
    const int brightAlpha = qRound(70 + 55 * intensity);
    painter.setPen(QPen(QColor(158, 178, 186, normalAlpha), 1));
    painter.drawLines(normal);
    painter.setPen(QPen(QColor(170, 190, 198, brightAlpha), 1));
    painter.drawLines(bright);
}

void MapView::drawTooltip(QPainter& painter) {
    if (hoverRow_ < 0 || hoverCol_ < 0) {
        return;
    }

    const Cell& cell = game_.grid().cell(hoverRow_, hoverCol_);
    QString title;
    QStringList lines;

    if (const Entity* entity = cell.occupant()) {
        title = entity->typeName();
        lines << QStringLiteral("HP      %1 / %2")
                     .arg(entity->health())
                     .arg(entity->maxHealth())
              << QStringLiteral("ATTACK  %1").arg(entity->attack())
              << QStringLiteral("SPEED   %1").arg(entity->speed());
        if (const auto* human = dynamic_cast<const Human*>(entity)) {
            lines << QStringLiteral("HUNGER  %1%%").arg(human->hunger());
            if (human->contaminationTimer() > 0) {
                lines << QStringLiteral("INFECT  %1 / %2")
                             .arg(human->contaminationTimer())
                             .arg(Human::InfectionTurns);
            }
        }
    } else if (const Wall* wall = cell.wall()) {
        title = wall->isDestroyed() ? QStringLiteral("Broken Wall")
                                    : QStringLiteral("Barricade");
        lines << QStringLiteral("DURAB.  %1 / %2")
                     .arg(wall->durability())
                     .arg(wall->maxDurability());
    } else if (const Plant* plant = cell.plant()) {
        title = QStringLiteral("Food Plant");
        lines << QStringLiteral("GROWTH  %1 / %2")
                     .arg(plant->growth())
                     .arg(Plant::MaxGrowth)
              << QStringLiteral("HARVEST %1").arg(2 + plant->growth());
    } else {
        switch (cell.decoration()) {
        case Decoration::Tree:
            title = QStringLiteral("Dead Tree");
            break;
        case Decoration::Bush:
            title = QStringLiteral("Berry Bush");
            break;
        case Decoration::Corpse:
            title = QStringLiteral("Corpse");
            lines << QStringLiteral("Something happened here.");
            break;
        case Decoration::Campfire:
            title = QStringLiteral("Campfire");
            lines << QStringLiteral("Warmth in the dark.");
            break;
        case Decoration::None:
            title = QStringLiteral("Empty Ground");
            lines << QStringLiteral("Nothing here.");
            break;
        }
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
    const int boxHeight =
        titleMetrics.height() + lineHeight * lines.size() + qRound(24 * ui);

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

    if (staticScene_.isNull()) {
        rebuildStatic();
    }

    if (!staticScene_.isNull()) {
        painter.drawPixmap(0, 0, staticScene_);
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
    int nextRow = -1;
    int nextCol = -1;
    if (cellAt(event->pos(), row, col)) {
        nextRow = row;
        nextCol = col;
    }

    if (nextRow != hoverRow_ || nextCol != hoverCol_) {
        hoverRow_ = nextRow;
        hoverCol_ = nextCol;
        update();
    }
}

void MapView::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);
    if (hoverRow_ >= 0 || hoverCol_ >= 0) {
        hoverRow_ = -1;
        hoverCol_ = -1;
        update();
    }
}

void MapView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    buildVignette();
    initRain();
    rebuildStatic();
}

MenuOverlay::MenuOverlay(QWidget* parent)
    : QWidget(parent),
      title_(nullptr),
      subtitle_(nullptr),
      story_(nullptr),
      hint_(nullptr),
      survivorsSpin_(nullptr),
      zombiesSpin_(nullptr),
      startButton_(nullptr),
      quitButton_(nullptr) {
    setAttribute(Qt::WA_StyledBackground, false);

    auto* outer = new QHBoxLayout(this);
    outer->setContentsMargins(64, 44, 64, 34);
    outer->setSpacing(40);

    auto* left = new QWidget(this);
    auto* column = new QVBoxLayout(left);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);

    title_ = new QLabel(QStringLiteral("ZOMBOGIG"), left);
    title_->setObjectName(QStringLiteral("menuTitle"));
    title_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    column->addWidget(title_);

    subtitle_ = new QLabel(QStringLiteral("THE CAMP AT THE END OF THE WORLD"),
                           left);
    subtitle_->setObjectName(QStringLiteral("menuSubtitle"));
    subtitle_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    column->addWidget(subtitle_);

    column->addSpacing(34);

    startButton_ = new QPushButton(QStringLiteral("START NEW GAME"), left);
    startButton_->setObjectName(QStringLiteral("primaryButton"));
    startButton_->setCursor(Qt::PointingHandCursor);
    startButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    column->addWidget(startButton_);

    column->addSpacing(8);

    quitButton_ = new QPushButton(QStringLiteral("EXIT"), left);
    quitButton_->setObjectName(QStringLiteral("secondaryButton"));
    quitButton_->setCursor(Qt::PointingHandCursor);
    quitButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    column->addWidget(quitButton_);

    column->addSpacing(30);

    auto* settings = new QWidget(left);
    auto* settingsRow = new QHBoxLayout(settings);
    settingsRow->setContentsMargins(0, 0, 0, 0);
    settingsRow->setSpacing(22);

    auto addSpin = [this, settingsRow](const QString& name, int value,
                                       int minimum, int maximum) {
        auto* block = new QWidget(this);
        auto* layout = new QVBoxLayout(block);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(3);

        auto* label = new QLabel(name, block);
        label->setObjectName(QStringLiteral("spinLabel"));
        label->setAlignment(Qt::AlignCenter);
        auto* spin = new QSpinBox(block);
        spin->setObjectName(QStringLiteral("spawnSpin"));
        spin->setRange(minimum, maximum);
        spin->setValue(value);
        spin->setAlignment(Qt::AlignCenter);
        spin->setAccelerated(true);
        spin->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        layout->addWidget(label);
        layout->addWidget(spin);
        settingsRow->addWidget(block);
        return spin;
    };

    survivorsSpin_ = addSpin(QStringLiteral("SURVIVORS"), 5, 1, 12);
    zombiesSpin_ = addSpin(QStringLiteral("ZOMBIES"), 8, 1, 30);
    settingsRow->addStretch(1);

    column->addWidget(settings);
    column->addStretch(1);

    hint_ = new QLabel(
        QStringLiteral(
            "SPACE  play / pause      1 2 3 4  speed      N  next hour      "
            "ESC  menu"),
        left);
    hint_->setObjectName(QStringLiteral("menuHint"));
    hint_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    column->addWidget(hint_);

    auto* credit = new QLabel(
        QStringLiteral("A C++ / Qt simulation  -  hope is a resource too"),
        left);
    credit->setObjectName(QStringLiteral("menuCredit"));
    credit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    column->addWidget(credit);

    outer->addWidget(left, 0);

    auto* gap = new QWidget(this);
    gap->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    outer->addWidget(gap, 1);

    auto* right = new QWidget(this);
    auto* rightColumn = new QVBoxLayout(right);
    rightColumn->setContentsMargins(0, 0, 0, 0);
    rightColumn->setSpacing(10);
    rightColumn->addStretch(1);

    auto* quoteRow = new QHBoxLayout();
    quoteRow->setContentsMargins(0, 0, 0, 0);
    quoteRow->setSpacing(16);

    auto* quoteBar = new QFrame(right);
    quoteBar->setObjectName(QStringLiteral("quoteBar"));
    quoteBar->setFixedWidth(3);
    quoteRow->addWidget(quoteBar);

    story_ = new QLabel(
        QStringLiteral(
            "\u201CThe outbreak started on a Tuesday. By Friday the city "
            "was gone.\n\n"
            "We reached an old farm with a handful of survivors and raised "
            "the barricades.\n\n"
            "The rain is heavy, the pantry is thin, and they are coming.\n\n"
            "Hold the camp for 48 hours.\u201D"),
        right);
    story_->setObjectName(QStringLiteral("menuStory"));
    story_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    story_->setWordWrap(true);
    story_->setMinimumWidth(320);
    story_->setMaximumWidth(460);
    quoteRow->addWidget(story_, 1);

    rightColumn->addLayout(quoteRow);

    auto* quoteCredit = new QLabel(QStringLiteral("- survivor's journal"),
                                   right);
    quoteCredit->setObjectName(QStringLiteral("menuQuoteCredit"));
    quoteCredit->setAlignment(Qt::AlignRight);
    rightColumn->addWidget(quoteCredit);
    rightColumn->addStretch(2);

    outer->addWidget(right, 0);
}

int MenuOverlay::survivors() const {
    return survivorsSpin_->value();
}

int MenuOverlay::zombies() const {
    return zombiesSpin_->value();
}

void MenuOverlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(8, 9, 10));
    bg.setColorAt(0.55, QColor(11, 9, 9));
    bg.setColorAt(1.0, QColor(18, 7, 6));
    painter.fillRect(rect(), bg);

    paintGrime(painter, rect(), 0x51C0DEu, 6);

    QRadialGradient gloom(QPointF(width() * 0.5, height() * 0.42),
                          std::max(width(), height()) * 0.7);
    gloom.setColorAt(0.0, QColor(0, 0, 0, 0));
    gloom.setColorAt(0.7, QColor(0, 0, 0, 40));
    gloom.setColorAt(1.0, QColor(0, 0, 0, 200));
    painter.fillRect(rect(), gloom);

    paintMenuScene(painter, rect());
}

EndOverlay::EndOverlay(QWidget* parent)
    : QWidget(parent),
      title_(nullptr),
      subtitle_(nullptr),
      detail_(nullptr),
      tagline_(nullptr),
      againButton_(nullptr),
      menuButton_(nullptr) {
    setAttribute(Qt::WA_StyledBackground, false);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(48, 48, 48, 48);
    outer->setSpacing(12);
    outer->addStretch(2);

    title_ = new QLabel(QStringLiteral("YOU SURVIVED"), this);
    title_->setObjectName(QStringLiteral("resultTitle"));
    title_->setAlignment(Qt::AlignCenter);
    outer->addWidget(title_);

    subtitle_ = new QLabel(this);
    subtitle_->setObjectName(QStringLiteral("resultSubtitle"));
    subtitle_->setAlignment(Qt::AlignCenter);
    outer->addWidget(subtitle_);

    detail_ = new QLabel(this);
    detail_->setObjectName(QStringLiteral("resultDetail"));
    detail_->setAlignment(Qt::AlignCenter);
    outer->addWidget(detail_);

    tagline_ = new QLabel(this);
    tagline_->setObjectName(QStringLiteral("resultTagline"));
    tagline_->setAlignment(Qt::AlignCenter);
    outer->addWidget(tagline_);

    outer->addSpacing(20);

    auto* buttons = new QWidget(this);
    auto* row = new QHBoxLayout(buttons);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(18);
    againButton_ = new QPushButton(QStringLiteral("PLAY AGAIN"), this);
    againButton_->setObjectName(QStringLiteral("primaryButton"));
    againButton_->setCursor(Qt::PointingHandCursor);
    menuButton_ = new QPushButton(QStringLiteral("MAIN MENU"), this);
    menuButton_->setObjectName(QStringLiteral("secondaryButton"));
    menuButton_->setCursor(Qt::PointingHandCursor);
    row->addWidget(againButton_);
    row->addWidget(menuButton_);
    auto* holder = new QWidget(this);
    auto* holderLayout = new QHBoxLayout(holder);
    holderLayout->setContentsMargins(0, 0, 0, 0);
    holderLayout->addStretch(1);
    holderLayout->addWidget(buttons);
    holderLayout->addStretch(1);
    outer->addWidget(holder);

    outer->addStretch(3);
}

void EndOverlay::setResult(bool won, int turns, int humans, int zombies) {
    title_->setText(won ? QStringLiteral("YOU SURVIVED")
                        : QStringLiteral("YOU DIED"));
    subtitle_->setText(won ? QStringLiteral("THE CAMP HELD")
                           : QStringLiteral("THE CAMP FELL"));
    detail_->setText(QStringLiteral("Day %1  -  %2 turns  -  %3 survivors  -  %4 zombies")
                         .arg(turns / 24 + 1)
                         .arg(turns)
                         .arg(humans)
                         .arg(zombies));
    tagline_->setText(
        won ? QStringLiteral("Dawn came. The barricades are still standing.")
            : QStringLiteral(
                  "Every dead survivor is another pair of teeth at the fence."));
}

void EndOverlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(8, 9, 8, 235));
    bg.setColorAt(1.0, QColor(24, 8, 7, 245));
    painter.fillRect(rect(), bg);

    paintGrime(painter, rect(), 0xBADF00Du, 5);

    QRadialGradient gloom(QPointF(width() * 0.5, height() * 0.45),
                          std::max(width(), height()) * 0.7);
    gloom.setColorAt(0.0, QColor(0, 0, 0, 0));
    gloom.setColorAt(0.75, QColor(0, 0, 0, 70));
    gloom.setColorAt(1.0, QColor(0, 0, 0, 210));
    painter.fillRect(rect(), gloom);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      page_(nullptr),
      topBar_(nullptr),
      bottomBar_(nullptr),
      statsHost_(nullptr),
      map_(nullptr),
      clock_(nullptr),
      menu_(nullptr),
      end_(nullptr),
      titleLabel_(nullptr),
      subtitleLabel_(nullptr),
      timeLabel_(nullptr),
      dayLabel_(nullptr),
      weatherLabel_(nullptr),
      waveLabel_(nullptr),
      eventLabel_(nullptr),
      hintLabel_(nullptr),
      zombieValue_(nullptr),
      humanValue_(nullptr),
      infectedValue_(nullptr),
      foodValue_(nullptr),
      defenseValue_(nullptr),
      autoButton_(nullptr),
      nextButton_(nullptr),
      speedGroup_(nullptr),
      autoTimer_(nullptr),
      speed_(1),
      autoPlaying_(false),
      uiScale_(0.0) {
    displayFont_ = displayFontFallback();

    setWindowTitle(QStringLiteral("ZomboGIG - Survival At The End"));
    setMinimumSize(1120, 720);

    buildGamePage();
    buildOverlays();

    autoTimer_ = new QTimer(this);
    connect(autoTimer_, &QTimer::timeout, this, &MainWindow::advanceTurn);

    setCentralWidget(page_);

    const QRect available =
        QGuiApplication::primaryScreen()->availableGeometry();
    resize(static_cast<int>(available.width() * 0.96),
           static_cast<int>(available.height() * 0.94));

    refreshHud();
    applyUiScale();
    updateAutoTimer();
    setFocus();
}

void MainWindow::buildGamePage() {
    page_ = new QWidget(this);
    auto* root = new QVBoxLayout(page_);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    topBar_ = new HudBar(true, page_);
    topBar_->setObjectName(QStringLiteral("topBar"));
    auto* top = new QHBoxLayout(topBar_);

    auto* titleBox = new QWidget(topBar_);
    auto* titleLayout = new QVBoxLayout(titleBox);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(1);
    titleLabel_ = new QLabel(QStringLiteral("ZOMBOGIG"), titleBox);
    titleLabel_->setObjectName(QStringLiteral("gameTitle"));
    subtitleLabel_ = new QLabel(QStringLiteral("SURVIVE THE NIGHT"), titleBox);
    subtitleLabel_->setObjectName(QStringLiteral("gameSubtitle"));
    titleLayout->addWidget(titleLabel_);
    titleLayout->addWidget(subtitleLabel_);
    top->addWidget(titleBox, 0, Qt::AlignVCenter);
    top->addStretch(1);

    auto* weatherBox = new QWidget(topBar_);
    auto* weatherLayout = new QVBoxLayout(weatherBox);
    weatherLayout->setContentsMargins(0, 0, 0, 0);
    weatherLayout->setSpacing(1);
    weatherLabel_ = new QLabel(QStringLiteral("LIGHT RAIN"), weatherBox);
    weatherLabel_->setObjectName(QStringLiteral("weatherValue"));
    weatherLabel_->setAlignment(Qt::AlignCenter);
    waveLabel_ = new QLabel(QStringLiteral("WAVE IN 8h"), weatherBox);
    waveLabel_->setObjectName(QStringLiteral("waveLabel"));
    waveLabel_->setAlignment(Qt::AlignCenter);
    weatherLayout->addWidget(weatherLabel_);
    weatherLayout->addWidget(waveLabel_);
    top->addWidget(weatherBox, 0, Qt::AlignVCenter);
    top->addStretch(1);

    auto* timeBox = new QWidget(topBar_);
    auto* timeLayout = new QVBoxLayout(timeBox);
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setSpacing(0);
    timeLabel_ = new QLabel(QStringLiteral("07:00"), timeBox);
    timeLabel_->setObjectName(QStringLiteral("timeValue"));
    timeLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dayLabel_ = new QLabel(QStringLiteral("DAY 1  -  41h LEFT"), timeBox);
    dayLabel_->setObjectName(QStringLiteral("dayLabel"));
    dayLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeLayout->addWidget(timeLabel_);
    timeLayout->addWidget(dayLabel_);
    top->addWidget(timeBox, 0, Qt::AlignVCenter);

    clock_ = new ClockWidget(topBar_);
    clock_->setMinutes(7 * 60);
    top->addWidget(clock_, 0, Qt::AlignVCenter);
    root->addWidget(topBar_);

    map_ = new MapView(game_, page_);
    root->addWidget(map_, 1);

    bottomBar_ = new HudBar(false, page_);
    bottomBar_->setObjectName(QStringLiteral("bottomBar"));
    auto* bottom = new QHBoxLayout(bottomBar_);

    statsHost_ = new QWidget(bottomBar_);
    auto* statsLayout = new QHBoxLayout(statsHost_);
    statsLayout->setContentsMargins(0, 0, 0, 0);

    auto addStat = [this, statsLayout](const QString& name,
                                       const QColor& color) -> QLabel* {
        auto* block = new QWidget(bottomBar_);
        auto* layout = new QVBoxLayout(block);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        auto* label = new QLabel(name, block);
        label->setObjectName(QStringLiteral("statLabel"));
        auto* value = new QLabel(QStringLiteral("0"), block);
        value->setObjectName(QStringLiteral("statValue"));
        value->setStyleSheet(
            QStringLiteral("color: %1;").arg(color.name()));
        layout->addWidget(label);
        layout->addWidget(value);
        statsLayout->addWidget(block, 0, Qt::AlignVCenter);
        return value;
    };

    zombieValue_ = addStat(QStringLiteral("ZOMBIES"), QColor(198, 82, 68));
    humanValue_ = addStat(QStringLiteral("SURVIVORS"), QColor(156, 174, 128));
    infectedValue_ = addStat(QStringLiteral("INFECTED"), QColor(176, 126, 156));
    foodValue_ = addStat(QStringLiteral("FOOD"), QColor(190, 154, 88));
    defenseValue_ = addStat(QStringLiteral("DEFENSE"), QColor(150, 166, 180));

    bottom->addWidget(statsHost_, 0, Qt::AlignVCenter);
    bottom->addSpacing(18);

    eventLabel_ = new QLabel(QStringLiteral("REC: Stay quiet."), bottomBar_);
    eventLabel_->setObjectName(QStringLiteral("eventLabel"));
    eventLabel_->setMinimumWidth(200);
    bottom->addWidget(eventLabel_, 1, Qt::AlignVCenter);

    hintLabel_ = new QLabel(
        QStringLiteral("SPACE  play/pause    1-4  speed    N  next hour"),
        bottomBar_);
    hintLabel_->setObjectName(QStringLiteral("hint"));
    bottom->addWidget(hintLabel_, 0, Qt::AlignVCenter);

    autoButton_ = new QPushButton(QStringLiteral("AUTO"), bottomBar_);
    autoButton_->setObjectName(QStringLiteral("autoButton"));
    autoButton_->setCheckable(true);
    autoButton_->setCursor(Qt::PointingHandCursor);
    connect(autoButton_, &QPushButton::clicked, this, [this] {
        autoPlaying_ = autoButton_->isChecked();
        updateAutoTimer();
        refreshHud();
    });
    bottom->addWidget(autoButton_, 0, Qt::AlignVCenter);

    speedGroup_ = new QButtonGroup(this);
    speedGroup_->setExclusive(true);
    for (int i = 1; i <= 4; ++i) {
        auto* button = new QPushButton(QStringLiteral("x%1").arg(i), bottomBar_);
        button->setObjectName(QStringLiteral("speedButton"));
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        speedGroup_->addButton(button, i);
        speedButtons_.push_back(button);
        bottom->addWidget(button, 0, Qt::AlignVCenter);
    }
    connect(speedGroup_, &QButtonGroup::idClicked, this,
            [this](int id) { setSpeed(id); });
    speedButtons_.first()->setChecked(true);

    nextButton_ =
        new QPushButton(QStringLiteral("NEXT HOUR"), bottomBar_);
    nextButton_->setObjectName(QStringLiteral("nextButton"));
    nextButton_->setCursor(Qt::PointingHandCursor);
    connect(nextButton_, &QPushButton::clicked, this, &MainWindow::advanceTurn);
    bottom->addWidget(nextButton_, 0, Qt::AlignVCenter);

    root->addWidget(bottomBar_);
}

void MainWindow::buildOverlays() {
    menu_ = new MenuOverlay(page_);
    end_ = new EndOverlay(page_);
    end_->hide();

    connect(menu_->startButton(), &QPushButton::clicked, this,
            &MainWindow::startGame);
    connect(menu_->quitButton(), &QPushButton::clicked, qApp,
            &QCoreApplication::quit);
    connect(end_->againButton(), &QPushButton::clicked, this,
            &MainWindow::replayGame);
    connect(end_->menuButton(), &QPushButton::clicked, this,
            &MainWindow::returnToMenu);

    menu_->raise();
    menu_->show();
}

void MainWindow::startGame() {
    game_.start(menu_->survivors(), menu_->zombies(),
                QRandomGenerator::global()->generate());

    autoPlaying_ = true;
    autoButton_->setChecked(true);
    setSpeed(1);

    menu_->hide();
    end_->hide();

    map_->refreshWorld();
    map_->syncWeather();
    clock_->setMinutes(game_.minutesOfDay());
    refreshHud();
    updateAutoTimer();
    setFocus();
}

void MainWindow::replayGame() {
    startGame();
}

void MainWindow::returnToMenu() {
    autoPlaying_ = false;
    autoButton_->setChecked(false);
    updateAutoTimer();
    end_->hide();
    menu_->show();
    menu_->raise();
    refreshHud();
    setFocus();
}

void MainWindow::advanceTurn() {
    if (!game_.isRunning()) {
        return;
    }
    game_.nextTurn();
    afterTurn();
}

void MainWindow::afterTurn() {
    map_->refreshWorld();
    map_->syncWeather();
    clock_->setMinutes(game_.minutesOfDay());
    refreshHud();

    if (game_.isOver()) {
        autoPlaying_ = false;
        autoButton_->setChecked(false);
        updateAutoTimer();
        end_->setResult(game_.humansWon(), game_.turn(),
                        game_.humanCount(), game_.zombieCount());
        end_->setGeometry(page_->rect());
        end_->show();
        end_->raise();
        refreshHud();
    }
}

void MainWindow::refreshHud() {
    zombieValue_->setText(QString::number(game_.zombieCount()));
    humanValue_->setText(QString::number(game_.humanCount()));
    infectedValue_->setText(QString::number(game_.infectedCount()));
    foodValue_->setText(QString::number(game_.food()));
    defenseValue_->setText(QString::number(game_.defense()));

    const int minutes = game_.minutesOfDay();
    timeLabel_->setText(QStringLiteral("%1:%2")
                            .arg(minutes / 60, 2, 10, QLatin1Char('0'))
                            .arg(minutes % 60, 2, 10, QLatin1Char('0')));
    dayLabel_->setText(QStringLiteral("DAY %1  -  %2h LEFT")
                           .arg(game_.day())
                           .arg(game_.turnsLeft()));
    weatherLabel_->setText(game_.weatherName());
    const int wave = game_.nextWaveIn();
    waveLabel_->setText(wave > 0 ? QStringLiteral("WAVE IN %1h").arg(wave)
                                 : QStringLiteral("FINAL HOURS"));
    eventLabel_->setText(QStringLiteral("REC: ") + game_.lastEvent());

    autoButton_->setText(autoPlaying_ ? QStringLiteral("PAUSE")
                                      : QStringLiteral("AUTO"));
    autoButton_->setChecked(autoPlaying_);
    for (int i = 0; i < speedButtons_.size(); ++i) {
        speedButtons_[i]->setChecked(i + 1 == speed_);
    }

    const bool running = game_.isRunning();
    autoButton_->setEnabled(running);
    nextButton_->setEnabled(running);
    for (QPushButton* button : speedButtons_) {
        button->setEnabled(running);
    }
}

void MainWindow::updateAutoTimer() {
    if (!autoTimer_) {
        return;
    }
    if (autoPlaying_ && game_.isRunning()) {
        autoTimer_->start(std::max(120, 1000 / speed_));
    } else {
        autoTimer_->stop();
    }
}

void MainWindow::toggleAutoPlay() {
    if (!game_.isRunning()) {
        return;
    }
    autoPlaying_ = !autoPlaying_;
    autoButton_->setChecked(autoPlaying_);
    updateAutoTimer();
    refreshHud();
}

void MainWindow::setSpeed(int speed) {
    speed_ = std::clamp(speed, 1, 4);
    for (int i = 0; i < speedButtons_.size(); ++i) {
        speedButtons_[i]->setChecked(i + 1 == speed_);
    }
    updateAutoTimer();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_F11) {
        if (isFullScreen()) {
            showMaximized();
        } else {
            showFullScreen();
        }
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen()) {
            showMaximized();
        } else if (!menu_->isVisible() && !end_->isVisible()) {
            returnToMenu();
        }
        return;
    }
    if (event->key() == Qt::Key_Space) {
        toggleAutoPlay();
        return;
    }
    if (event->key() == Qt::Key_N || event->key() == Qt::Key_Right) {
        advanceTurn();
        return;
    }
    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_4) {
        setSpeed(event->key() - Qt::Key_0);
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    applyUiScale();
    if (menu_ && page_) {
        menu_->setGeometry(page_->rect());
    }
    if (end_ && page_) {
        end_->setGeometry(page_->rect());
    }
}

void MainWindow::applyUiScale() {
    if (!topBar_ || !bottomBar_ || !clock_ || !hintLabel_) {
        return;
    }

    const qreal scale = std::clamp(height() / 1080.0, 0.82, 2.2);
    const auto px = [scale](qreal value) {
        return qRound(value * scale);
    };

    hintLabel_->setVisible(width() >= px(1560));
    eventLabel_->setVisible(width() >= px(1240));

    if (std::abs(scale - uiScale_) < 0.01) {
        return;
    }
    uiScale_ = scale;

    topBar_->setFixedHeight(px(110));
    bottomBar_->setFixedHeight(px(124));
    clock_->setFixedSize(px(84), px(84));

    topBar_->layout()->setContentsMargins(px(28), px(10), px(26), px(10));
    topBar_->layout()->setSpacing(px(16));
    bottomBar_->layout()->setContentsMargins(px(28), px(14), px(26), px(14));
    bottomBar_->layout()->setSpacing(px(10));
    statsHost_->layout()->setSpacing(px(30));

    autoButton_->setFixedHeight(px(38));
    nextButton_->setFixedHeight(px(38));
    nextButton_->setMinimumWidth(px(120));

    const QString body = QStringLiteral("Segoe UI");
    const QString display = displayFont_;

    QString css;
    css += QStringLiteral(
        "QLabel { color: #b8b2a0; background: transparent; }");
    css += QStringLiteral(
               "QLabel#gameTitle { color: #b2433a; font-family: \"%1\";"
               " font-size: %2px; font-weight: bold; }")
               .arg(display)
               .arg(px(32));
    css += QStringLiteral(
               "QLabel#gameSubtitle { color: #a4a496; font-size: %1px; }")
               .arg(px(11));
    css += QStringLiteral(
               "QLabel#weatherValue { color: #a8b2ae; font-size: %1px; }")
               .arg(px(14));
    css += QStringLiteral(
               "QLabel#waveLabel { color: #8a6a5e; font-size: %1px; }")
               .arg(px(11));
    css += QStringLiteral(
               "QLabel#timeValue { color: #d8d1bf; font-family: \"%1\";"
               " font-size: %2px; font-weight: bold; }")
               .arg(display)
               .arg(px(30));
    css += QStringLiteral(
               "QLabel#dayLabel { color: #837d6d; font-size: %1px; }")
               .arg(px(11));
    css += QStringLiteral(
               "QLabel#statLabel { color: #8a8474; font-size: %1px; }")
               .arg(px(10));
    css += QStringLiteral(
               "QLabel#statValue { font-family: \"%1\"; font-size: %2px;"
               " font-weight: bold; }")
               .arg(display)
               .arg(px(26));
    css += QStringLiteral("QLabel#eventLabel { color: #8c8577; font-size: %1px; }")
               .arg(px(12));
    css += QStringLiteral("QLabel#hint { color: #6f6a5e; font-size: %1px; }")
               .arg(px(11));

    css += QStringLiteral(
               "QPushButton { background-color: #141718; color: #c2bba9;"
               " border: 1px solid #2c3230; padding: %1px %2px;"
               " font-weight: bold; font-size: %3px; }"
               "QPushButton:hover { background-color: #1c2021;"
               " border-color: #4a534f; color: #d8d1bf; }"
               "QPushButton:pressed { background-color: #2a1412; }"
               "QPushButton:disabled { color: #5a564e; border-color: #222724; }")
               .arg(px(6))
               .arg(px(12))
               .arg(px(12));
    css += QStringLiteral(
               "QPushButton#nextButton { color: #d0705f; border-color: #5a2a26; }"
               "QPushButton#nextButton:hover { border-color: #8a3a32; }");
    css += QStringLiteral(
               "QPushButton#autoButton, QPushButton#speedButton {"
               " min-width: %1px; padding: %2px %3px; }"
               "QPushButton#autoButton:checked, QPushButton#speedButton:checked {"
               " background-color: #4a1c18; border-color: #a8463a;"
               " color: #f0d9bd; }")
               .arg(px(38))
               .arg(px(6))
               .arg(px(8));

    css += QStringLiteral(
               "QLabel#menuTitle { color: #b2433a; font-family: \"%1\";"
               " font-size: %2px; font-weight: bold; }")
               .arg(display)
               .arg(px(92));
    css += QStringLiteral(
               "QLabel#menuSubtitle { color: #9e9a88; font-size: %1px; }")
               .arg(px(16));
    css += QStringLiteral(
               "QLabel#menuStory { color: #b8b0a0; font-family: \"%1\";"
               " font-size: %2px; }")
               .arg(body)
               .arg(px(16));
    css += QStringLiteral(
               "QLabel#menuQuoteCredit { color: #6f6a5e; font-size: %1px;"
               " font-style: italic; }")
               .arg(px(12));
    css += QStringLiteral(
        "QFrame#quoteBar { background-color: #6a2a24; border: none; }");
    css += QStringLiteral(
               "QLabel#menuHint { color: #7d7566; font-size: %1px; }")
               .arg(px(12));
    css += QStringLiteral(
               "QLabel#menuCredit { color: #5f5a50; font-size: %1px; }")
               .arg(px(11));
    css += QStringLiteral(
               "QLabel#resultTitle { color: #b2433a; font-family: \"%1\";"
               " font-size: %2px; font-weight: bold; }")
               .arg(display)
               .arg(px(78));
    css += QStringLiteral(
               "QLabel#resultSubtitle { color: #c0b8a6; font-size: %1px; }")
               .arg(px(20));
    css += QStringLiteral(
               "QLabel#resultDetail { color: #8c8577; font-size: %1px; }")
               .arg(px(15));
    css += QStringLiteral(
               "QLabel#resultTagline { color: #9c7c70; font-size: %1px;"
               " font-style: italic; }")
               .arg(px(14));
    css += QStringLiteral(
               "QLabel#spinLabel { color: #8a8474; font-size: %1px; }")
               .arg(px(10));
    css += QStringLiteral(
               "QSpinBox#spawnSpin { background-color: #101314;"
               " color: #d8d1bf; border: 1px solid #2c3230;"
               " padding: %1px %2px; font-size: %3px; font-weight: bold;"
               " min-width: %4px; }"
               "QSpinBox#spawnSpin:focus { border-color: #5a2a26; }")
               .arg(px(2))
               .arg(px(4))
               .arg(px(14))
               .arg(px(56));
    css += QStringLiteral(
               "QPushButton#primaryButton { background-color: #2a1210;"
               " color: #e2c7aa; border: 1px solid #79322b;"
               " padding: %1px %2px; font-size: %3px; }"
               "QPushButton#primaryButton:hover { background-color: #3a1714;"
               " border-color: #b2433a; }"
               "QPushButton#secondaryButton { background-color: #101314;"
               " color: #a8a292; border: 1px solid #2c3230;"
               " padding: %1px %2px; font-size: %3px; }"
               "QPushButton#secondaryButton:hover { border-color: #555e59;"
               " color: #d8d1bf; }")
               .arg(px(9))
               .arg(px(22))
               .arg(px(14));

    setStyleSheet(css);
}
