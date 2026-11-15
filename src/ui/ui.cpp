#include <algorithm>
#include <cmath>
#include <utility>

#include <QAbstractSpinBox>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QEvent>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QLinearGradient>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
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
#include <QVector>
#include <QWidget>

class QKeyEvent;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QPushButton;
class QResizeEvent;
class QSpinBox;
class QTimer;
class QButtonGroup;
class QCheckBox;
class QLineEdit;

class HudBar : public QWidget {
public:
    explicit HudBar(bool top, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool top_;
};

class ClockWidget : public QWidget {
public:
    explicit ClockWidget(QWidget* parent = nullptr);

    void setMinutes(int minutes);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int minutes_;
};

class MapView : public QWidget {
public:
    explicit MapView(Game& game, QWidget* parent = nullptr);

    QSize sizeHint() const override;
    void refreshWorld();
    void syncWeather();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    struct Drop {
        qreal x;
        qreal y;
        qreal speed;
        qreal length;
    };

    void buildMapLayer();
    void rebuildStatic();
    void updatePlacement();
    void buildVignette();
    void initRain();
    void stepRain();

    void drawBackground(QPainter& painter);
    void drawMap(QPainter& painter);
    void drawAmbient(QPainter& painter);
    void drawCampfireGlow(QPainter& painter);
    void drawHover(QPainter& painter);
    void drawRain(QPainter& painter);
    void drawTooltip(QPainter& painter);

    qreal daylight() const;
    QPointF cacheToWidget(qreal x, qreal y) const;
    bool cellAt(const QPointF& point, int& row, int& col) const;

    Game& game_;
    QPixmap mapLayer_;
    QPixmap staticScene_;
    QPixmap vignette_;
    QVector<Drop> drops_;
    QTimer* rainTimer_;
    QElapsedTimer rainClock_;
    QPoint mousePos_;
    int hoverRow_;
    int hoverCol_;
    qreal mapScale_;
    QPoint mapTopLeft_;
    int fireRow_;
    int fireCol_;
};

class MenuOverlay : public QWidget {
public:
    explicit MenuOverlay(QWidget* parent = nullptr);

    QPushButton* startButton() const { return startButton_; }
    QPushButton* quitButton() const { return quitButton_; }
    QPushButton* launchButton() const { return launchButton_; }
    QPushButton* backButton() const { return backButton_; }

    void showSetup();
    void showMenu();

    int survivors() const;
    int zombies() const;
    int plants() const;
    int defense() const;
    unsigned seed() const;
    std::vector<HumanType> humanTypes() const;
    std::vector<ZombieType> zombieTypes() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QWidget* menuPage_;
    QWidget* setupPage_;
    QLabel* title_;
    QLabel* subtitle_;
    QLabel* story_;
    QSpinBox* survivorsSpin_;
    QSpinBox* zombiesSpin_;
    QSpinBox* plantsSpin_;
    QSpinBox* defenseSpin_;
    QLineEdit* seedEdit_;
    QVector<QCheckBox*> humanChecks_;
    QVector<QCheckBox*> zombieChecks_;
    QPushButton* startButton_;
    QPushButton* quitButton_;
    QPushButton* launchButton_;
    QPushButton* backButton_;
};

class EndOverlay : public QWidget {
public:
    explicit EndOverlay(QWidget* parent = nullptr);

    void setResult(bool won, int turns, int humans, int zombies);
    QPushButton* againButton() const { return againButton_; }
    QPushButton* menuButton() const { return menuButton_; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* title_;
    QLabel* subtitle_;
    QLabel* detail_;
    QLabel* tagline_;
    QPushButton* againButton_;
    QPushButton* menuButton_;
};

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void buildGamePage();
    void buildOverlays();
    void applyUiScale();

    void startGame();
    void replayGame();
    void returnToMenu();
    void advanceTurn();
    void afterTurn();
    void refreshHud();
    void updateAutoTimer();
    void toggleAutoPlay();
    void setSpeed(int speed);

    Game game_;
    QWidget* page_;
    QWidget* topBar_;
    QWidget* bottomBar_;
    QWidget* sidePanel_;
    QWidget* statsHost_;
    MapView* map_;
    ClockWidget* clock_;
    MenuOverlay* menu_;
    EndOverlay* end_;

    QLabel* titleLabel_;
    QLabel* subtitleLabel_;
    QLabel* timeLabel_;
    QLabel* dayLabel_;
    QLabel* weatherLabel_;
    QLabel* waveLabel_;
    QLabel* eventLabel_;
    QLabel* hintLabel_;
    QLabel* zombieValue_;
    QLabel* humanValue_;
    QLabel* infectedValue_;
    QLabel* foodValue_;
    QLabel* defenseValue_;

    QPushButton* autoButton_;
    QPushButton* nextButton_;
    QVector<QPushButton*> speedButtons_;
    QButtonGroup* speedGroup_;
    QTimer* autoTimer_;

    int speed_;
    bool autoPlaying_;
    qreal uiScale_;
    QString displayFont_;
};

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

//les sprites sont des choix de dessin propres a l'interface
enum class Sprite {
    Farmer,
    Doctor,
    Military,
    Builder,
    Child,
    Tank,
    Runner,
    Exploder
};

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

//associe le type de personnage a son sprite dessine
Sprite spriteForEntity(const Entity& entity) {
    if (const auto* human = dynamic_cast<const Human*>(&entity)) {
        switch (human->type_humain()) {
        case HumanType::Farmer:
            return Sprite::Farmer;
        case HumanType::Doctor:
            return Sprite::Doctor;
        case HumanType::Military:
            return Sprite::Military;
        case HumanType::Builder:
            return Sprite::Builder;
        case HumanType::Child:
            return Sprite::Child;
        }
    }

    if (const auto* zombie = dynamic_cast<const Zombie*>(&entity)) {
        switch (zombie->type_zombie()) {
        case ZombieType::Tank:
            return Sprite::Tank;
        case ZombieType::Runner:
            return Sprite::Runner;
        case ZombieType::Exploder:
            return Sprite::Exploder;
        }
    }

    return Sprite::Farmer;
}

bool isHumanSprite(Sprite sprite) {
    return sprite == Sprite::Farmer || sprite == Sprite::Doctor ||
           sprite == Sprite::Military || sprite == Sprite::Builder ||
           sprite == Sprite::Child;
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

void drawWallSegment(QPainter& painter, const QPoint& first,
                     const QPoint& second, int height, QColor color,
                     bool damaged = false, bool drawPosts = true) {
    const QPoint firstTop(first.x(), first.y() - height);
    const QPoint secondTop(second.x(), second.y() - height);

    painter.setPen(QPen(QColor(46, 40, 33), 1));
    painter.setBrush(color);
    painter.drawPolygon(QPolygon()
                        << first << second << secondTop << firstTop);

    painter.setPen(QPen(QColor(50, 43, 35, 160), 1));
    for (int i = 1; i <= 2; ++i) {
        const int drop = height * i / 3;
        painter.drawLine(first.x(), first.y() - drop,
                         second.x(), second.y() - drop);
    }

    if (damaged) {
        painter.setPen(QPen(QColor(34, 29, 24, 190), 1));
        const int centerX = (first.x() + second.x()) / 2;
        const int centerY = (first.y() + second.y()) / 2;
        painter.drawLine(centerX - 5, centerY - height / 3,
                         centerX + 5, centerY - height * 2 / 3);
    }

    if (drawPosts) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(54, 47, 38));
        painter.drawRect(QRectF(first.x() - 2, first.y() - height,
                                4, height + 3));
        painter.drawRect(QRectF(second.x() - 2, second.y() - height,
                                4, height + 3));
    }
}

void drawWall(QPainter& painter, int cx, int topY, const Wall& wall) {
    const int wallHeight = 22;
    const QPoint top(cx, topY - TOP_HALF_HEIGHT);
    const QPoint left(cx - TOP_HALF_WIDTH, topY);
    const QPoint right(cx + TOP_HALF_WIDTH, topY);
    const QPoint bottom(cx, topY + TOP_HALF_HEIGHT);

    if (wall.est_detruit()) {
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

    if (wall.est_porte()) {
        QPoint first = top;
        QPoint second = right;
        QPoint swing(TOP_HALF_WIDTH, -TOP_HALF_HEIGHT);

        if (wall.ligne() == Game::fin_base) {
            first = left;
            second = bottom;
        } else if (wall.colonne() == Game::debut_base) {
            first = top;
            second = left;
            swing = QPoint(TOP_HALF_WIDTH, TOP_HALF_HEIGHT);
        } else if (wall.colonne() == Game::fin_base) {
            first = right;
            second = bottom;
            swing = QPoint(-TOP_HALF_WIDTH, -TOP_HALF_HEIGHT);
        }

        const int doorHeight = wallHeight * 4 / 5;
        const QPoint edge = second - first;
        const QPoint hinge(first.x() + edge.x() / 10,
                           first.y() + edge.y() / 10);
        QPoint end;

        if (wall.est_ouverte()) {
            end = QPoint(hinge.x() + swing.x() * 4 / 5,
                         hinge.y() + swing.y() * 4 / 5);
        } else {
            end = QPoint(second.x() - edge.x() / 10,
                         second.y() - edge.y() / 10);
        }

        drawWallSegment(painter, hinge, end, doorHeight,
                        QColor(118, 86, 52),
                        wall.durabilite() * 2 < wall.durabilite_max(),
                        !wall.est_ouverte());
        if (wall.est_ouverte()) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(54, 47, 38));
            painter.drawRect(QRectF(hinge.x() - 2,
                                    hinge.y() - doorHeight,
                                    4, doorHeight + 3));
        }
        return;
    }

    if (wall.est_maison()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(92, 74, 56));
        painter.drawPolygon(QPolygon()
                            << QPoint(cx - 40, topY)
                            << QPoint(cx - 34, topY - 38)
                            << QPoint(cx + 34, topY - 38)
                            << QPoint(cx + 40, topY));
        painter.setBrush(QColor(58, 46, 36));
        painter.drawPolygon(QPolygon()
                            << QPoint(cx - 46, topY - 36)
                            << QPoint(cx, topY - 62)
                            << QPoint(cx + 46, topY - 36));
        painter.setBrush(QColor(206, 168, 92, 190));
        painter.drawRect(QRectF(cx - 8, topY - 28, 14, 12));
        painter.setBrush(QColor(46, 36, 28));
        painter.drawRect(QRectF(cx + 16, topY - 22, 12, 20));

        if (wall.durabilite() * 2 < wall.durabilite_max()) {
            painter.setPen(QPen(QColor(34, 29, 24, 190), 1));
            painter.drawLine(cx - 24, topY - 30, cx - 10, topY - 20);
            painter.setPen(Qt::NoPen);
        }
        return;
    }

    const QColor color(72, 63, 52);
    const bool damaged =
        wall.durabilite() * 2 < wall.durabilite_max();
    bool drew_segment = false;

    if (wall.ligne() == Game::debut_base) {
        drawWallSegment(painter, top, right, wallHeight, color, damaged);
        drew_segment = true;
    }
    if (wall.ligne() == Game::fin_base) {
        drawWallSegment(painter, left, bottom, wallHeight, color, damaged);
        drew_segment = true;
    }
    if (wall.colonne() == Game::debut_base) {
        drawWallSegment(painter, top, left, wallHeight, color, damaged);
        drew_segment = true;
    }
    if (wall.colonne() == Game::fin_base) {
        drawWallSegment(painter, right, bottom, wallHeight, color, damaged);
        drew_segment = true;
    }

    if (!drew_segment) {
        drawWallSegment(painter, left, top, wallHeight, color, damaged);
        drawWallSegment(painter, top, right, wallHeight, color, damaged);
    }
}

void drawPlant(QPainter& painter, int cx, int topY, const Plant& plant) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 55));
    painter.drawEllipse(QPointF(cx, topY + 1), 13, 4);

    painter.setBrush(QColor(56, 46, 34));
    painter.drawEllipse(QPointF(cx - 9, topY), 8, 3);
    painter.drawEllipse(QPointF(cx + 9, topY), 8, 3);

    const int leaves = 3 + plant.croissance();
    for (int i = 0; i < leaves; ++i) {
        const qreal dx = (i - (leaves - 1) / 2.0) * 7.0;
        const qreal tipX = cx + dx * 1.4;
        const qreal tipY = topY - 9 - plant.croissance() * 3;
        painter.setPen(QPen(QColor(66, 102, 54), 2));
        painter.drawLine(QPointF(cx + dx, topY), QPointF(tipX, tipY));
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(78, 118, 62));
        painter.drawEllipse(QPointF(tipX, tipY), 3.5, 2.5);
    }

    if (plant.croissance() >= 2) {
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

void drawCorpse(QPainter& painter, int cx, int topY, qreal opacity = 1.0) {
    painter.save();
    painter.setOpacity(opacity);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 70));
    painter.drawEllipse(QPointF(cx, topY + 5), 29, 10);

    painter.setPen(QPen(QColor(48, 52, 45), 1));
    painter.setBrush(QColor(70, 74, 63));
    painter.drawPolygon(QPolygonF()
                        << QPointF(cx - 27, topY - 4)
                        << QPointF(cx + 9, topY - 11)
                        << QPointF(cx + 18, topY - 3)
                        << QPointF(cx - 22, topY + 6));

    painter.setPen(QPen(QColor(48, 52, 45), 3));
    painter.drawLine(QPointF(cx - 12, topY - 1),
                     QPointF(cx - 23, topY - 10));
    painter.drawLine(QPointF(cx - 8, topY + 2),
                     QPointF(cx - 16, topY + 11));
    painter.drawLine(QPointF(cx + 5, topY + 1),
                     QPointF(cx + 14, topY + 9));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(96, 92, 80));
    painter.drawEllipse(QPointF(cx + 24, topY - 6), 8, 7);
    painter.restore();
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

void drawFoodAid(QPainter& painter, int cx, int topY) {
    const int groundY = topY + 3;
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 75));
    painter.drawEllipse(QPointF(cx, groundY + 4), 22, 7);

    QPolygonF top;
    top << QPointF(cx, groundY - 17)
        << QPointF(cx + 17, groundY - 9)
        << QPointF(cx, groundY - 1)
        << QPointF(cx - 17, groundY - 9);
    painter.setBrush(QColor(184, 137, 79));
    painter.drawPolygon(top);

    QPolygonF leftSide;
    leftSide << QPointF(cx - 17, groundY - 9)
             << QPointF(cx, groundY - 1)
             << QPointF(cx, groundY + 11)
             << QPointF(cx - 17, groundY + 3);
    painter.setBrush(QColor(132, 91, 52));
    painter.drawPolygon(leftSide);

    QPolygonF rightSide;
    rightSide << QPointF(cx + 17, groundY - 9)
              << QPointF(cx, groundY - 1)
              << QPointF(cx, groundY + 11)
              << QPointF(cx + 17, groundY + 3);
    painter.setBrush(QColor(101, 69, 43));
    painter.drawPolygon(rightSide);

    painter.setPen(QPen(QColor(238, 215, 164), 2));
    painter.drawLine(QPointF(cx - 7, groundY - 10),
                     QPointF(cx + 7, groundY - 10));
    painter.drawLine(QPointF(cx, groundY - 14),
                     QPointF(cx, groundY - 6));
    painter.restore();
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

//dessine l'entite selon le sprite fourni par son type
void drawEntity(QPainter& painter, int cx, int topY, const Entity& entity) {
    const Sprite sprite = spriteForEntity(entity);
    const bool human = isHumanSprite(sprite);
    const QColor body = entity.couleur();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 70));
    painter.drawEllipse(QPointF(cx, topY + 1), human ? 16 : 17, 6);

    int headY = topY - 50;
    int shoulderY = topY - 43;
    int waistY = topY - 18;
    int halfBody = human ? 10 : 11;
    bool lean = false;

    if (sprite == Sprite::Child) {
        halfBody = 7;
        headY = topY - 36;
        shoulderY = topY - 30;
        waistY = topY - 13;
    }

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

    if (entity.vie() < entity.vie_max()) {
        const int barWidth = halfBody * 2;
        const int barHeight = 4;
        const int barY = headY - 14;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(18, 18, 18, 210));
        painter.drawRect(QRectF(cx - barWidth / 2.0, barY, barWidth, barHeight));
        painter.setBrush(human ? QColor(118, 168, 104) : QColor(176, 54, 42));
        const qreal ratio =
            static_cast<qreal>(entity.vie()) / entity.vie_max();
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

    const qreal splatRadius =
        std::clamp(rect.height() * 0.42, 22.0, 78.0);
    for (int i = 0; i < splats; ++i) {
        const qreal cx = rect.left() + rect.width() * (0.09 + 0.15 * i);
        const qreal cy = rect.top() + rect.height() * (i % 2 == 0 ? 0.55 : 0.28);
        drawSplat(painter, cx, cy, splatRadius,
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

void drawDoorGuard(QPainter& painter, const QPointF& base, qreal scale) {
    painter.save();
    painter.translate(base);
    painter.scale(scale, scale);
    painter.rotate(2.0);
    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(0, 0, 0, 165));
    painter.drawEllipse(QPointF(-2, 2), 26, 8);

    painter.setBrush(QColor(32, 30, 31));
    painter.drawRoundedRect(QRectF(-11.5, -27, 9.5, 28), 4, 4);
    painter.drawRoundedRect(QRectF(2, -27, 9.5, 28), 4, 4);

    QPolygonF torso;
    torso << QPointF(-14, -26) << QPointF(12, -26)
          << QPointF(17, -52) << QPointF(-10, -48);
    painter.setBrush(QColor(32, 30, 31));
    painter.drawPolygon(torso);

    QPolygonF rim;
    rim << QPointF(-14, -26) << QPointF(-10, -48)
        << QPointF(-5, -48) << QPointF(-8, -26);
    painter.setBrush(QColor(96, 80, 58, 120));
    painter.drawPolygon(rim);

    painter.setPen(QPen(QColor(32, 30, 31), 6, Qt::SolidLine,
                        Qt::RoundCap));
    painter.drawLine(QPointF(8, -48), QPointF(26, -41));
    painter.drawLine(QPointF(7, -38), QPointF(23, -30));
    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(132, 106, 82));
    painter.drawEllipse(QPointF(28, -41), 3.6, 4.0);
    painter.drawEllipse(QPointF(25, -30), 3.6, 4.0);

    painter.setBrush(QColor(26, 24, 22));
    painter.drawEllipse(QPointF(7, -58), 9, 10);
    painter.setBrush(QColor(88, 70, 54));
    painter.drawEllipse(QPointF(11, -56), 5, 6);
    painter.setBrush(QColor(26, 24, 22));
    painter.drawEllipse(QPointF(13, -60), 4, 3.2);
    painter.restore();
}

void drawDoor(QPainter& painter, const QPointF& top, const QPointF& right,
              qreal wallHeight) {
    const auto at = [&](qreal t) {
        return QPointF(top.x() + (right.x() - top.x()) * t,
                       top.y() + (right.y() - top.y()) * t);
    };
    const QPointF a = at(0.44);
    const QPointF b = at(0.78);
    const qreal doorHeight = wallHeight * 0.86;

    QPolygonF door;
    door << a << b
         << QPointF(b.x(), b.y() - doorHeight)
         << QPointF(a.x(), a.y() - doorHeight);
    painter.setPen(QPen(QColor(86, 68, 46), 3));
    painter.setBrush(QColor(50, 40, 29));
    painter.drawPolygon(door);

    painter.setPen(QPen(QColor(30, 24, 18), 1));
    for (int i = 1; i < 5; ++i) {
        const qreal t = i / 5.0;
        const QPointF foot(a.x() + (b.x() - a.x()) * t,
                           a.y() + (b.y() - a.y()) * t);
        painter.drawLine(foot, QPointF(foot.x(), foot.y() - doorHeight));
    }
    painter.drawLine(QPointF(a.x(), a.y() - doorHeight * 0.5),
                     QPointF(b.x(), b.y() - doorHeight * 0.5));

    const QPointF handle(b.x() - (b.x() - a.x()) * 0.18,
                         b.y() - doorHeight * 0.46);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(158, 126, 74));
    painter.drawEllipse(handle, 2.2, 3.2);
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
    QRadialGradient warmth(QPointF(10, -30), 380);
    warmth.setColorAt(0.0, QColor(198, 120, 56, 62));
    warmth.setColorAt(0.45, QColor(110, 58, 30, 28));
    warmth.setColorAt(0.75, QColor(60, 32, 18, 12));
    warmth.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setBrush(warmth);
    painter.drawEllipse(QPointF(0, -20), 390, 290);

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
    painter.setBrush(QColor(58, 50, 39));
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

    const auto leftWallPoint = [&](qreal t) {
        return QPointF(top.x() + (leftPoint.x() - top.x()) * t,
                       top.y() + (leftPoint.y() - top.y()) * t);
    };

    QPolygonF leftWall;
    leftWall << top << leftPoint
             << QPointF(leftPoint.x(), leftPoint.y() - wallHeight)
             << QPointF(top.x(), top.y() - wallHeight);
    painter.setPen(QPen(QColor(14, 13, 11), 2));
    painter.setBrush(QColor(34, 30, 25));
    painter.drawPolygon(leftWall);

    painter.setPen(QPen(QColor(76, 64, 48, 130), 1.2));
    painter.drawLine(QPointF(top.x(), top.y() - wallHeight),
                     QPointF(leftPoint.x(), leftPoint.y() - wallHeight));

    painter.setPen(QPen(QColor(46, 40, 32, 130), 1));
    for (int i = 1; i < 5; ++i) {
        const qreal t = i / 5.0;
        const QPointF base = leftWallPoint(t);
        painter.drawLine(base, QPointF(base.x(), base.y() - wallHeight));
    }

    QPolygonF rightWall;
    rightWall << top << right
              << QPointF(right.x(), right.y() - wallHeight)
              << QPointF(top.x(), top.y() - wallHeight);
    painter.setBrush(QColor(30, 27, 23));
    painter.drawPolygon(rightWall);

    painter.setPen(QPen(QColor(80, 68, 50, 130), 1.2));
    painter.drawLine(
        QPointF(top.x(), top.y() - wallHeight),
        QPointF(right.x(), right.y() - wallHeight));

    painter.setPen(QPen(QColor(48, 42, 34, 130), 1));
    for (int i = 1; i < 5; ++i) {
        const qreal t = i / 5.0;
        const QPointF b(top.x() + (right.x() - top.x()) * t,
                        top.y() + (right.y() - top.y()) * t);
        painter.drawLine(b, QPointF(b.x(), b.y() - wallHeight));
    }

    drawDoor(painter, top, right, wallHeight);

    QPainterPath floorClip;
    floorClip.addPolygon(floor);
    painter.save();
    painter.setClipPath(floorClip);

    QRandomGenerator stainRng(0xB100D7u);
    for (int index = 0; index < 11; ++index) {
        const qreal floorU =
            0.12 + stainRng.generateDouble() * 0.76;
        const qreal floorV =
            0.12 + stainRng.generateDouble() * 0.76;
        const qreal x = (floorU - floorV) * halfWidth;
        const qreal y =
            -halfHeight + (floorU + floorV) * halfHeight;

        QPainterPath splat;
        const int blobCount = 3 + stainRng.bounded(3);

        for (int blobIndex = 0; blobIndex < blobCount; ++blobIndex) {
            const qreal blobX =
                x + (stainRng.generateDouble() - 0.5) * 16.0;
            const qreal blobY =
                y + (stainRng.generateDouble() - 0.5) * 9.0;
            const qreal radius =
                3.0 + stainRng.generateDouble() * 5.5;

            splat.addEllipse(
                QPointF(blobX, blobY), radius,
                radius * (0.6 + stainRng.generateDouble() * 0.4));
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(
            QColor(72, 17, 14, 55 + stainRng.bounded(55)));
        painter.drawPath(splat);
    }

    painter.restore();

    const QPointF lampPos(0, -halfHeight - wallHeight + 22);
    painter.setPen(QPen(QColor(18, 16, 13), 2));
    painter.drawLine(lampPos, lampPos + QPointF(0, -26));
    QRadialGradient lampGlow(lampPos, 240);
    lampGlow.setColorAt(0.0, QColor(236, 178, 108, 120));
    lampGlow.setColorAt(0.45, QColor(168, 108, 56, 48));
    lampGlow.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(lampGlow);
    painter.drawEllipse(lampPos, 240, 190);
    painter.setBrush(QColor(238, 184, 114));
    painter.drawEllipse(lampPos, 7, 9);

    painter.setBrush(QColor(150, 96, 50, 55));
    painter.drawEllipse(QPointF(0, 28), 150, 68);
    painter.setBrush(QColor(214, 150, 82, 40));
    painter.drawEllipse(QPointF(0, 23), 84, 34);

    const QPointF doorMid(top.x() + (right.x() - top.x()) * 0.61,
                          top.y() + (right.y() - top.y()) * 0.61);
    drawDoorGuard(painter, QPointF(doorMid.x() - 14, doorMid.y() + 4), 1.15);

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
    const bool shouldRain = game_.en_cours() && game_.intensite_pluie() > 0.0;
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
        (game_.grille().lignes() + game_.grille().colonnes()) * TILE_WIDTH / 2 +
        2 * CACHE_PAD;
    const int height =
        (game_.grille().lignes() + game_.grille().colonnes()) * TILE_HEIGHT / 2 +
        TILE_THICKNESS + TOP_HALF_HEIGHT + 1 + 2 * CACHE_PAD;

    mapLayer_ = QPixmap(width, height);
    mapLayer_.fill(Qt::transparent);

    QPainter painter(&mapLayer_);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int originX = width / 2;
    const int originY = TILE_THICKNESS + TOP_HALF_HEIGHT + CACHE_PAD;

    fireRow_ = -1;
    fireCol_ = -1;

    for (int sum = 0; sum <= game_.grille().lignes() + game_.grille().colonnes() - 2;
         ++sum) {
        for (int row = 0; row < game_.grille().lignes(); ++row) {
            const int col = sum - row;
            if (col < 0 || col >= game_.grille().colonnes()) {
                continue;
            }

            const Cell& cell = game_.grille().cellule(row, col);
            const int cx = originX + (col - row) * TILE_WIDTH / 2;
            const int cy = originY + (col + row) * TILE_HEIGHT / 2;
            const int topY = cy - TILE_THICKNESS;

            drawBlock(painter, cx, cy, QColor(58, 72, 54));

            for (const QPoint& spot : game_.sang()) {
                if (spot.x() == col && spot.y() == row) {
                    quint32 seed = static_cast<quint32>(row * 131 + col * 977 +
                                                       19);
                    drawBloodSplat(painter, cx, topY, seed);
                    break;
                }
            }

            if (const Wall* wall = cell.mur()) {
                drawWall(painter, cx, topY, *wall);
            }
            if (const Plant* plant = cell.plante()) {
                drawPlant(painter, cx, topY, *plant);
            }
            if (cell.decoration() == Decoration::Campfire) {
                fireRow_ = row;
                fireCol_ = col;
            }
            drawDecoration(painter, cx, topY, cell.decoration());
            if (cell.a_un_cadavre()) {
                const qreal opacity =
                    cell.tours_cadavre() /
                    static_cast<qreal>(Cell::duree_cadavre);
                const bool plant_on_cell = cell.plante() != nullptr;
                drawCorpse(painter,
                           cx + (plant_on_cell ? 32 : 0),
                           topY + (plant_on_cell ? 15 : 0),
                           opacity);
            }
            if (cell.ravitaillement() > 0) {
                drawFoodAid(painter, cx, topY);
            }
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
    const qreal hour = game_.minutes_du_jour() / 60.0;
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

        if (r < 0 || c < 0 || r >= game_.grille().lignes() ||
            c >= game_.grille().colonnes()) {
            continue;
        }

        const qreal du = u - (c - r);
        const qreal dv = v - (c + r);
        if (std::abs(du) + std::abs(dv) > 1.0) {
            continue;
        }

        const Cell& cell = game_.grille().cellule(r, c);
        if (offset > 0 && !cell.mur() && !cell.plante() &&
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
    const Cell& cell = game_.grille().cellule(hoverRow_, hoverCol_);
    const Entity* entity = cell.occupant();
    const Human* human = dynamic_cast<const Human*>(entity);

    if (human != nullptr && human->type_humain() == HumanType::Military) {
        const auto centre_case = [this, originX, planeY](qreal ligne,
                                                         qreal colonne) {
            return cacheToWidget(
                originX + (colonne - ligne) * TOP_HALF_WIDTH,
                planeY + (colonne + ligne) * TOP_HALF_HEIGHT);
        };

        painter.save();
        painter.setPen(QPen(QColor(238, 186, 98, 100), 1, Qt::DotLine));
        painter.setBrush(QColor(238, 186, 98, 28));

        for (int ligne = 0; ligne < game_.grille().lignes(); ligne++) {
            for (int colonne = 0;
                 colonne < game_.grille().colonnes(); colonne++) {
                if (Game::distance_manhattan(
                        hoverRow_, hoverCol_, ligne, colonne) >
                    Human::portee_tir) {
                    continue;
                }

                const QPointF tileCenter = centre_case(ligne, colonne);
                QPolygonF tile;
                tile << QPointF(tileCenter.x(),
                                tileCenter.y() - halfH)
                     << QPointF(tileCenter.x() + halfW, tileCenter.y())
                     << QPointF(tileCenter.x(),
                                tileCenter.y() + halfH)
                     << QPointF(tileCenter.x() - halfW, tileCenter.y());
                painter.drawPolygon(tile);
            }
        }

        painter.restore();
    }

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
    const qreal intensity = game_.intensite_pluie();
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

    const Cell& cell = game_.grille().cellule(hoverRow_, hoverCol_);
    QString title;
    QStringList lines;

    if (const Entity* entity = cell.occupant()) {
        title = entity->nom_type();
        lines << QStringLiteral("HP      %1 / %2")
                     .arg(entity->vie())
                     .arg(entity->vie_max())
              << QStringLiteral("ATTACK  %1").arg(entity->attaque())
              << QStringLiteral("SPEED   %1").arg(entity->vitesse());
        if (const auto* human = dynamic_cast<const Human*>(entity)) {
            lines << QStringLiteral("HUNGER  %1%%").arg(human->faim());
            if (human->type_humain() == HumanType::Military) {
                lines << QStringLiteral("SHOT RANGE  %1 tiles")
                             .arg(Human::portee_tir);
            }
            if (human->compteur_infection() > 0) {
                lines << QStringLiteral("INFECT  %1 / %2")
                             .arg(human->compteur_infection())
                             .arg(Human::duree_infection);
            }
        }
    } else if (const Wall* wall = cell.mur()) {
        if (wall->est_detruit()) {
            title = QStringLiteral("Ruined Wall");
        } else if (wall->est_maison()) {
            title = QStringLiteral("House");
        } else if (wall->est_porte() && wall->est_ouverte()) {
            title = QStringLiteral("Open Door");
        } else if (wall->est_porte()) {
            title = QStringLiteral("Door");
        } else {
            title = QStringLiteral("Barricade");
        }
        lines << QStringLiteral("DURAB.  %1 / %2")
                     .arg(wall->durabilite())
                     .arg(wall->durabilite_max());
    } else if (cell.a_un_cadavre()) {
        title = QStringLiteral("Zombie Corpse");
        lines << QStringLiteral("DISINTEGRATES IN %1 TURNS")
                     .arg(cell.tours_cadavre());
    } else if (const Plant* plant = cell.plante()) {
        title = QStringLiteral("Food Plant");
        lines << QStringLiteral("GROWTH  %1 / %2")
                     .arg(plant->croissance())
                     .arg(Plant::croissance_max)
              << QStringLiteral("HARVEST %1").arg(2 + plant->croissance());
    } else if (cell.ravitaillement() > 0) {
        title = QStringLiteral("Food Aid");
        lines << QStringLiteral("%1 food; any survivor collects it.")
                     .arg(cell.ravitaillement());
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

    if (cell.ravitaillement() > 0 &&
        title != QStringLiteral("Food Aid")) {
        lines << QStringLiteral("FOOD AID  %1")
                     .arg(cell.ravitaillement());
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
      menuPage_(nullptr),
      setupPage_(nullptr),
      title_(nullptr),
      subtitle_(nullptr),
      story_(nullptr),
      survivorsSpin_(nullptr),
      zombiesSpin_(nullptr),
      plantsSpin_(nullptr),
      defenseSpin_(nullptr),
      seedEdit_(nullptr),
      startButton_(nullptr),
      quitButton_(nullptr),
      launchButton_(nullptr),
      backButton_(nullptr) {
    setAttribute(Qt::WA_StyledBackground, false);

    auto* stack = new QVBoxLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    menuPage_ = new QWidget(this);
    auto* outer = new QHBoxLayout(menuPage_);
    outer->setContentsMargins(64, 44, 64, 34);
    outer->setSpacing(40);

    auto* left = new QWidget(menuPage_);
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

    column->addStretch(3);

    startButton_ = new QPushButton(QStringLiteral("START NEW GAME"), left);
    startButton_->setObjectName(QStringLiteral("menuChoicePrimary"));
    startButton_->setCursor(Qt::PointingHandCursor);
    startButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    column->addWidget(startButton_);

    column->addSpacing(10);

    quitButton_ = new QPushButton(QStringLiteral("EXIT"), left);
    quitButton_->setObjectName(QStringLiteral("menuChoice"));
    quitButton_->setCursor(Qt::PointingHandCursor);
    quitButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    column->addWidget(quitButton_);

    column->addStretch(2);

    outer->addWidget(left, 0);

    auto* gap = new QWidget(menuPage_);
    gap->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    outer->addWidget(gap, 1);

    auto* right = new QWidget(menuPage_);
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
            "Stay behind the doors, tend the plants, and keep everyone "
            "fed.\n\n"
            "Clear the horde before the pantry runs dry, or the camp "
            "falls.\u201D"),
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
    rightColumn->addStretch(1);

    outer->addWidget(right, 0);
    stack->addWidget(menuPage_);

    setupPage_ = new QWidget(this);
    auto* setupOuter = new QVBoxLayout(setupPage_);
    setupOuter->setContentsMargins(80, 40, 80, 40);
    setupOuter->addStretch(1);

    auto* setupTitle = new QLabel(QStringLiteral("CAMP SETUP"), setupPage_);
    setupTitle->setObjectName(QStringLiteral("setupTitle"));
    setupTitle->setAlignment(Qt::AlignCenter);
    setupOuter->addWidget(setupTitle);

    auto* setupSubtitle = new QLabel(
        QStringLiteral("Decide what waits behind the barricades."),
        setupPage_);
    setupSubtitle->setObjectName(QStringLiteral("setupSubtitle"));
    setupSubtitle->setAlignment(Qt::AlignCenter);
    setupOuter->addWidget(setupSubtitle);

    setupOuter->addSpacing(18);

    auto* panel = new QWidget(setupPage_);
    panel->setObjectName(QStringLiteral("setupPanel"));
    panel->setAttribute(Qt::WA_StyledBackground, true);
    panel->setMaximumWidth(760);
    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(34, 28, 34, 28);
    panelLayout->setSpacing(14);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(10);

    auto addSpin = [&grid](QWidget* host, int row, int col,
                           const QString& name, int value, int minimum,
                           int maximum) {
        auto* label = new QLabel(name, host);
        label->setObjectName(QStringLiteral("spinLabel"));
        auto* spin = new QSpinBox(host);
        spin->setObjectName(QStringLiteral("spawnSpin"));
        spin->setRange(minimum, maximum);
        spin->setValue(value);
        spin->setAlignment(Qt::AlignCenter);
        spin->setAccelerated(true);
        spin->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        grid->addWidget(label, row, col * 2);
        grid->addWidget(spin, row, col * 2 + 1);
        return spin;
    };

    survivorsSpin_ =
        addSpin(panel, 0, 0, QStringLiteral("SURVIVORS"), 5, 1, 12);
    zombiesSpin_ =
        addSpin(panel, 0, 1, QStringLiteral("ZOMBIES"), 8, 1, 30);
    plantsSpin_ = addSpin(panel, 1, 0, QStringLiteral("PLANTS"), 6, 0, 6);
    defenseSpin_ =
        addSpin(panel, 1, 1, QStringLiteral("DEFENSE"), 14, 4, 20);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);
    panelLayout->addLayout(grid);

    auto makeChecks = [&panel](const QString& title,
                               const QStringList& names,
                               QVector<QCheckBox*>& target) {
        auto* label = new QLabel(title, panel);
        label->setObjectName(QStringLiteral("setupSection"));
        auto* row = new QHBoxLayout();
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(18);
        for (const QString& name : names) {
            auto* check = new QCheckBox(name, panel);
            check->setObjectName(QStringLiteral("setupCheck"));
            check->setChecked(true);
            target.push_back(check);
            row->addWidget(check);
        }
        row->addStretch(1);
        return std::make_pair(label, row);
    };

    auto humanRow = makeChecks(
        QStringLiteral("SURVIVOR ROLES"),
        {QStringLiteral("FARMER"), QStringLiteral("DOCTOR"),
         QStringLiteral("MILITARY"), QStringLiteral("BUILDER")},
        humanChecks_);
    panelLayout->addWidget(humanRow.first);
    panelLayout->addLayout(humanRow.second);

    auto zombieRow = makeChecks(
        QStringLiteral("ZOMBIE STRAINS"),
        {QStringLiteral("TANK"), QStringLiteral("RUNNER"),
         QStringLiteral("EXPLODER")},
        zombieChecks_);
    panelLayout->addWidget(zombieRow.first);
    panelLayout->addLayout(zombieRow.second);

    auto* seedRow = new QHBoxLayout();
    seedRow->setContentsMargins(0, 0, 0, 0);
    seedRow->setSpacing(12);
    auto* seedLabel = new QLabel(QStringLiteral("SEED"), panel);
    seedLabel->setObjectName(QStringLiteral("spinLabel"));
    seedEdit_ = new QLineEdit(panel);
    seedEdit_->setObjectName(QStringLiteral("seedEdit"));
    seedEdit_->setPlaceholderText(QStringLiteral("random"));
    seedRow->addWidget(seedLabel);
    seedRow->addWidget(seedEdit_, 1);
    auto* randomButton = new QPushButton(QStringLiteral("RANDOM"), panel);
    randomButton->setObjectName(QStringLiteral("secondaryButton"));
    randomButton->setCursor(Qt::PointingHandCursor);
    connect(randomButton, &QPushButton::clicked, this, [this] {
        seedEdit_->setText(QString::number(
            QRandomGenerator::global()->generate()));
    });
    seedRow->addWidget(randomButton);
    panelLayout->addSpacing(4);
    panelLayout->addLayout(seedRow);

    panelLayout->addSpacing(10);

    auto* actions = new QHBoxLayout();
    actions->setContentsMargins(0, 0, 0, 0);
    actions->setSpacing(16);
    backButton_ = new QPushButton(QStringLiteral("BACK"), panel);
    backButton_->setObjectName(QStringLiteral("secondaryButton"));
    backButton_->setCursor(Qt::PointingHandCursor);
    launchButton_ =
        new QPushButton(QStringLiteral("START THE SIEGE"), panel);
    launchButton_->setObjectName(QStringLiteral("primaryButton"));
    launchButton_->setCursor(Qt::PointingHandCursor);
    actions->addWidget(backButton_, 1);
    actions->addWidget(launchButton_, 2);
    panelLayout->addLayout(actions);

    auto* holder = new QWidget(setupPage_);
    auto* holderRow = new QHBoxLayout(holder);
    holderRow->setContentsMargins(0, 0, 0, 0);
    holderRow->addStretch(1);
    holderRow->addWidget(panel, 0);
    holderRow->addStretch(1);
    setupOuter->addWidget(holder);
    setupOuter->addStretch(1);

    stack->addWidget(setupPage_);
    setupPage_->hide();

    connect(startButton_, &QPushButton::clicked, this,
            &MenuOverlay::showSetup);
    connect(backButton_, &QPushButton::clicked, this,
            &MenuOverlay::showMenu);
}

void MenuOverlay::showSetup() {
    if (seedEdit_->text().trimmed().isEmpty()) {
        seedEdit_->setText(QString::number(
            QRandomGenerator::global()->generate()));
    }
    menuPage_->hide();
    setupPage_->show();
}

void MenuOverlay::showMenu() {
    setupPage_->hide();
    menuPage_->show();
}

int MenuOverlay::survivors() const {
    return survivorsSpin_->value();
}

int MenuOverlay::zombies() const {
    return zombiesSpin_->value();
}

int MenuOverlay::plants() const {
    return plantsSpin_->value();
}

int MenuOverlay::defense() const {
    return defenseSpin_->value();
}

unsigned MenuOverlay::seed() const {
    bool ok = false;
    const uint value = seedEdit_->text().trimmed().toUInt(&ok);
    if (!ok || value == 0) {
        return QRandomGenerator::global()->generate();
    }
    return value;
}

std::vector<HumanType> MenuOverlay::humanTypes() const {
    static const HumanType all[] = {
        HumanType::Farmer, HumanType::Doctor,
        HumanType::Military, HumanType::Builder};
    std::vector<HumanType> selected;
    for (int i = 0; i < humanChecks_.size(); ++i) {
        if (humanChecks_[i]->isChecked()) {
            selected.push_back(all[i]);
        }
    }
    return selected;
}

std::vector<ZombieType> MenuOverlay::zombieTypes() const {
    static const ZombieType all[] = {
        ZombieType::Tank, ZombieType::Runner, ZombieType::Exploder};
    std::vector<ZombieType> selected;
    for (int i = 0; i < zombieChecks_.size(); ++i) {
        if (zombieChecks_[i]->isChecked()) {
            selected.push_back(all[i]);
        }
    }
    return selected;
}

void MenuOverlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(8, 9, 10));
    bg.setColorAt(0.55, QColor(11, 9, 9));
    bg.setColorAt(1.0, QColor(18, 7, 6));
    painter.fillRect(rect(), bg);

    QRadialGradient gloom(QPointF(width() * 0.5, height() * 0.42),
                          std::max(width(), height()) * 0.7);
    gloom.setColorAt(0.0, QColor(0, 0, 0, 0));
    gloom.setColorAt(0.55, QColor(0, 0, 0, 0));
    gloom.setColorAt(0.8, QColor(0, 0, 0, 30));
    gloom.setColorAt(1.0, QColor(0, 0, 0, 150));
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
      sidePanel_(nullptr),
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

    auto* middle = new QWidget(page_);
    auto* middleLayout = new QHBoxLayout(middle);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(0);

    map_ = new MapView(game_, middle);
    middleLayout->addWidget(map_, 1);

    sidePanel_ = new QWidget(middle);
    sidePanel_->setObjectName(QStringLiteral("sidePanel"));
    sidePanel_->setAttribute(Qt::WA_StyledBackground, true);
    auto* side = new QVBoxLayout(sidePanel_);

    auto* panelTitle = new QLabel(QStringLiteral("CONTROLS"), sidePanel_);
    panelTitle->setObjectName(QStringLiteral("panelTitle"));
    side->addWidget(panelTitle);

    nextButton_ =
        new QPushButton(QStringLiteral("NEXT HOUR"), sidePanel_);
    nextButton_->setObjectName(QStringLiteral("nextButton"));
    nextButton_->setCursor(Qt::PointingHandCursor);
    connect(nextButton_, &QPushButton::clicked, this, &MainWindow::advanceTurn);
    side->addWidget(nextButton_);

    autoButton_ = new QPushButton(QStringLiteral("AUTO"), sidePanel_);
    autoButton_->setObjectName(QStringLiteral("autoButton"));
    autoButton_->setCheckable(true);
    autoButton_->setCursor(Qt::PointingHandCursor);
    connect(autoButton_, &QPushButton::clicked, this, [this] {
        autoPlaying_ = autoButton_->isChecked();
        updateAutoTimer();
        refreshHud();
    });
    side->addWidget(autoButton_);

    auto* speedLabel = new QLabel(QStringLiteral("SPEED"), sidePanel_);
    speedLabel->setObjectName(QStringLiteral("panelLabel"));
    side->addWidget(speedLabel);

    auto* speedGrid = new QGridLayout();
    speedGrid->setContentsMargins(0, 0, 0, 0);
    speedGrid->setSpacing(6);
    speedGroup_ = new QButtonGroup(this);
    speedGroup_->setExclusive(true);
    for (int i = 1; i <= 4; ++i) {
        auto* button =
            new QPushButton(QStringLiteral("x%1").arg(i), sidePanel_);
        button->setObjectName(QStringLiteral("speedButton"));
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        speedGroup_->addButton(button, i);
        speedButtons_.push_back(button);
        speedGrid->addWidget(button, (i - 1) / 2, (i - 1) % 2);
    }
    connect(speedGroup_, &QButtonGroup::idClicked, this,
            [this](int id) { setSpeed(id); });
    speedButtons_.first()->setChecked(true);
    side->addLayout(speedGrid);

    side->addStretch(1);

    hintLabel_ = new QLabel(
        QStringLiteral("SPACE  play/pause\n1-4  speed\nN  next hour\n"
                       "ESC  menu\nHOVER A TILE  view stats"),
        sidePanel_);
    hintLabel_->setObjectName(QStringLiteral("hint"));
    hintLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hintLabel_->setWordWrap(true);
    side->addWidget(hintLabel_);

    middleLayout->addWidget(sidePanel_, 0);
    root->addWidget(middle, 1);

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

    root->addWidget(bottomBar_);
}

void MainWindow::buildOverlays() {
    menu_ = new MenuOverlay(page_);
    end_ = new EndOverlay(page_);
    end_->hide();

    connect(menu_->launchButton(), &QPushButton::clicked, this,
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
    game_.demarrer(menu_->survivors(), menu_->zombies(), menu_->plants(),
                menu_->defense(), menu_->humanTypes(), menu_->zombieTypes(),
                menu_->seed());

    autoPlaying_ = false;
    autoButton_->setChecked(false);
    setSpeed(1);

    menu_->hide();
    end_->hide();

    map_->refreshWorld();
    map_->syncWeather();
    clock_->setMinutes(game_.minutes_du_jour());
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
    menu_->showMenu();
    menu_->show();
    menu_->raise();
    refreshHud();
    setFocus();
}

void MainWindow::advanceTurn() {
    if (!game_.en_cours()) {
        return;
    }
    game_.tour_suivant();
    afterTurn();
}

void MainWindow::afterTurn() {
    map_->refreshWorld();
    map_->syncWeather();
    clock_->setMinutes(game_.minutes_du_jour());
    refreshHud();

    if (game_.est_termine()) {
        autoPlaying_ = false;
        autoButton_->setChecked(false);
        updateAutoTimer();
        end_->setResult(game_.humains_gagnent(), game_.tour(),
                        game_.nombre_humains(), game_.nombre_zombies());
        end_->setGeometry(page_->rect());
        end_->show();
        end_->raise();
        refreshHud();
    }
}

void MainWindow::refreshHud() {
    zombieValue_->setText(QString::number(game_.nombre_zombies()));
    humanValue_->setText(QString::number(game_.nombre_humains()));
    infectedValue_->setText(QString::number(game_.nombre_infectes()));
    foodValue_->setText(QString::number(game_.nourriture()));
    defenseValue_->setText(QString::number(game_.defense()));

    const int minutes = game_.minutes_du_jour();
    timeLabel_->setText(QStringLiteral("%1:%2")
                            .arg(minutes / 60, 2, 10, QLatin1Char('0'))
                            .arg(minutes % 60, 2, 10, QLatin1Char('0')));
    dayLabel_->setText(QStringLiteral("DAY %1  -  %2 SURVIVORS")
                           .arg(game_.jour())
                           .arg(game_.nombre_humains()));
    weatherLabel_->setText(game_.nom_meteo());
    const int wave = game_.prochaine_vague_dans();
    waveLabel_->setText(QStringLiteral("WAVE IN %1h").arg(wave));
    eventLabel_->setText(QStringLiteral("REC: ") + game_.dernier_evenement());

    autoButton_->setText(autoPlaying_ ? QStringLiteral("PAUSE")
                                      : QStringLiteral("AUTO"));
    autoButton_->setChecked(autoPlaying_);
    for (int i = 0; i < speedButtons_.size(); ++i) {
        speedButtons_[i]->setChecked(i + 1 == speed_);
    }

    const bool running = game_.en_cours();
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
    if (autoPlaying_ && game_.en_cours()) {
        autoTimer_->start(std::max(120, 1000 / speed_));
    } else {
        autoTimer_->stop();
    }
}

void MainWindow::toggleAutoPlay() {
    if (!game_.en_cours()) {
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
        } else if (menu_->isVisible()) {
            menu_->showMenu();
        } else if (!end_->isVisible()) {
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
    if (!topBar_ || !bottomBar_ || !sidePanel_ || !clock_ || !hintLabel_) {
        return;
    }

    const qreal scale = std::clamp(height() / 1080.0, 0.82, 2.2);
    const auto px = [scale](qreal value) {
        return qRound(value * scale);
    };

    hintLabel_->setVisible(true);
    eventLabel_->setVisible(width() >= px(980));

    if (std::abs(scale - uiScale_) < 0.01) {
        return;
    }
    uiScale_ = scale;

    topBar_->setFixedHeight(px(110));
    bottomBar_->setFixedHeight(px(96));
    sidePanel_->setFixedWidth(px(196));
    clock_->setFixedSize(px(84), px(84));

    topBar_->layout()->setContentsMargins(px(28), px(10), px(26), px(10));
    topBar_->layout()->setSpacing(px(16));
    bottomBar_->layout()->setContentsMargins(px(28), px(10), px(26), px(10));
    bottomBar_->layout()->setSpacing(px(10));
    statsHost_->layout()->setSpacing(px(26));
    sidePanel_->layout()->setContentsMargins(px(18), px(16), px(18), px(16));
    sidePanel_->layout()->setSpacing(px(10));

    autoButton_->setFixedHeight(px(40));
    for (QPushButton* button : speedButtons_) {
        button->setFixedHeight(px(34));
    }
    nextButton_->setFixedHeight(px(52));
    nextButton_->setMinimumWidth(0);

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
    css += QStringLiteral("QLabel#hint { color: #8a8474; font-size: %1px; }")
               .arg(px(13));
    css += QStringLiteral(
               "QWidget#sidePanel { background-color: #080b0a;"
               " border-left: 1px solid #1c2221; }");
    css += QStringLiteral(
               "QLabel#panelTitle { color: #b2433a; font-size: %1px;"
               " font-weight: bold; }")
               .arg(px(12));
    css += QStringLiteral(
               "QLabel#panelLabel { color: #8a8474; font-size: %1px;"
               " padding-top: %2px; }")
               .arg(px(10))
               .arg(px(4));

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
               "QPushButton#menuChoicePrimary { background: transparent;"
               " border: none; text-align: left; color: #d86b5c;"
               " font-family: \"%1\"; font-size: %2px; font-weight: bold;"
               " padding: %3px 0; }"
               "QPushButton#menuChoicePrimary:hover { color: #f08a76; }"
               "QPushButton#menuChoicePrimary:pressed { color: #b2433a; }")
               .arg(display)
               .arg(px(46))
               .arg(px(10));
    css += QStringLiteral(
               "QPushButton#menuChoice { background: transparent;"
               " border: none; text-align: left; color: #8f8878;"
               " font-family: \"%1\"; font-size: %2px; font-weight: bold;"
               " padding: %3px 0; }"
               "QPushButton#menuChoice:hover { color: #c9c1ae; }"
               "QPushButton#menuChoice:pressed { color: #b2433a; }")
               .arg(display)
               .arg(px(22))
               .arg(px(4));
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
        "QWidget#setupPanel { background-color: rgba(11, 14, 13, 235);"
               " border: 1px solid #2c3230; }");
    css += QStringLiteral(
               "QLabel#setupTitle { color: #b2433a; font-family: \"%1\";"
               " font-size: %2px; font-weight: bold; }")
               .arg(display)
               .arg(px(56));
    css += QStringLiteral(
               "QLabel#setupSubtitle { color: #9e9a88; font-size: %1px; }")
               .arg(px(14));
    css += QStringLiteral(
               "QLabel#setupSection { color: #b2433a; font-size: %1px;"
               " font-weight: bold; padding-top: %2px; }")
               .arg(px(12))
               .arg(px(6));
    css += QStringLiteral(
               "QCheckBox#setupCheck { color: #c2bba9; font-size: %1px;"
               " spacing: %2px; }"
               "QCheckBox#setupCheck::indicator { width: %3px; height: %3px;"
               " border: 1px solid #3a423f; background-color: #101314; }"
               "QCheckBox#setupCheck::indicator:checked {"
               " background-color: #7a2f28; border-color: #a8463a; }")
               .arg(px(13))
               .arg(px(8))
               .arg(px(16));
    css += QStringLiteral(
               "QLineEdit#seedEdit { background-color: #101314;"
               " color: #d8d1bf; border: 1px solid #2c3230;"
               " padding: %1px %2px; font-size: %3px; }"
               "QLineEdit#seedEdit:focus { border-color: #5a2a26; }")
               .arg(px(6))
               .arg(px(8))
               .arg(px(13));
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
               " min-width: %4px; min-height: %5px; }"
               "QSpinBox#spawnSpin:focus { border-color: #5a2a26; }")
               .arg(px(2))
               .arg(px(4))
               .arg(px(14))
               .arg(px(40))
               .arg(px(26));
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
