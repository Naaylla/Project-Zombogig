#include "mainwindow.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <vector>

#include <QColor>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLinearGradient>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QPushButton>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QScreen>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

#include "clockwidget.h"
#include "entities/human.h"
#include "entities/zombie.h"
#include "mapview.h"
#include "prop.h"

namespace {

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
                bool topBar) {
    QRandomGenerator rng(seed);
    painter.setPen(Qt::NoPen);

    for (int i = 0; i < rect.width() / 5; ++i) {
        const int x = rect.left() + rng.bounded(rect.width());
        const int y = rect.top() + rng.bounded(rect.height());
        painter.setBrush(QColor(0, 0, 0, rng.bounded(4, 16)));
        painter.drawRect(x, y, rng.bounded(1, 4), 1);
    }

    painter.setPen(QPen(QColor(60, 64, 58, 40), 1));
    for (int i = 0; i < 6; ++i) {
        const int x = rect.left() + rng.bounded(rect.width());
        const int y = rect.top() + rng.bounded(rect.height());
        painter.drawLine(x, y, x + rng.bounded(10, 40), y + rng.bounded(-2, 3));
    }
    painter.setPen(Qt::NoPen);

    const int splats = topBar ? 2 : 3;
    const qreal topPositions[] = {0.03, 0.93};
    const qreal bottomPositions[] = {0.02, 0.36, 0.93};
    const qreal* positions = topBar ? topPositions : bottomPositions;
    for (int i = 0; i < splats; ++i) {
        const qreal cx = rect.left() + rect.width() * positions[i];
        const qreal cy = rect.top() + rect.height() * (i == 0 ? 0.55 : 0.32);
        drawSplat(painter, cx, cy, rect.height() * 0.5,
                  seed + static_cast<quint32>(i) * 17u);
    }
}

class HudBar : public QWidget {
public:
    explicit HudBar(bool top, QWidget* parent = nullptr)
        : QWidget(parent), top_(top) {
        setAttribute(Qt::WA_StyledBackground, false);
    }

protected:
    void paintEvent(QPaintEvent*) override {
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

        paintGrime(painter, rect(), top_ ? 0xB100D1u : 0xB100D2u, top_);
    }

private:
    bool top_;
};

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      grid_(12, 12),
      map_(nullptr),
      clock_(nullptr),
      topBar_(nullptr),
      bottomBar_(nullptr),
      statsHost_(nullptr),
      spawnPanel_(nullptr),
      topLayout_(nullptr),
      bottomLayout_(nullptr),
      statsLayout_(nullptr),
      spawnLayout_(nullptr),
      titleLabel_(nullptr),
      subtitleLabel_(nullptr),
      timeLabel_(nullptr),
      dayLabel_(nullptr),
      zombieValue_(nullptr),
      humanValue_(nullptr),
      defenseValue_(nullptr),
      foodValue_(nullptr),
      hintLabel_(nullptr),
      skipButton_(nullptr),
      applyButton_(nullptr),
      zombieSpin_(nullptr),
      humanSpin_(nullptr),
      treeSpin_(nullptr),
      uiScale_(0.0),
      totalMinutes_(20 * 60) {
    populateWorld();

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    topBar_ = new HudBar(true, central);
    topBar_->setObjectName(QStringLiteral("topBar"));

    topLayout_ = new QHBoxLayout(topBar_);

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
    topLayout_->addWidget(titleBox, 0, Qt::AlignVCenter);
    topLayout_->addStretch(1);

    auto* timeBox = new QWidget(topBar_);
    auto* timeLayout = new QVBoxLayout(timeBox);
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setSpacing(0);

    timeLabel_ = new QLabel(timeBox);
    timeLabel_->setObjectName(QStringLiteral("timeValue"));
    timeLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    dayLabel_ = new QLabel(timeBox);
    dayLabel_->setObjectName(QStringLiteral("dayLabel"));
    dayLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeLayout->addWidget(timeLabel_);
    timeLayout->addWidget(dayLabel_);
    topLayout_->addWidget(timeBox, 0, Qt::AlignVCenter);

    clock_ = new ClockWidget(topBar_);
    clock_->setMinutes(totalMinutes_);
    topLayout_->addWidget(clock_, 0, Qt::AlignVCenter);
    root->addWidget(topBar_);

    map_ = new MapView(grid_, central);
    map_->setMinutes(totalMinutes_);
    root->addWidget(map_, 1);

    bottomBar_ = new HudBar(false, central);
    bottomBar_->setObjectName(QStringLiteral("bottomBar"));

    bottomLayout_ = new QHBoxLayout(bottomBar_);

    statsHost_ = new QWidget(bottomBar_);
    statsLayout_ = new QHBoxLayout(statsHost_);
    statsLayout_->setContentsMargins(0, 0, 0, 0);

    auto addStat = [this](const QString& name, const QColor& color)
        -> QLabel* {
        auto* block = new QWidget(bottomBar_);
        auto* layout = new QVBoxLayout(block);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        auto* label = new QLabel(name, block);
        label->setObjectName(QStringLiteral("statLabel"));
        auto* value = new QLabel(QStringLiteral("0"), block);
        value->setObjectName(QStringLiteral("statValue"));
        value->setStyleSheet(QStringLiteral("color: %1;").arg(color.name()));
        layout->addWidget(label);
        layout->addWidget(value);
        statsLayout_->addWidget(block, 0, Qt::AlignVCenter);
        return value;
    };

    zombieValue_ = addStat(QStringLiteral("ZOMBIES"), QColor(198, 82, 68));
    humanValue_ = addStat(QStringLiteral("HUMANS"), QColor(156, 174, 128));
    defenseValue_ = addStat(QStringLiteral("DEFENSE"), QColor(150, 166, 180));
    foodValue_ = addStat(QStringLiteral("FOOD"), QColor(190, 154, 88));

    bottomLayout_->addWidget(statsHost_, 0, Qt::AlignVCenter);
    bottomLayout_->addStretch(1);

    hintLabel_ = new QLabel(QStringLiteral("Hover a tile to inspect it."), bottomBar_);
    hintLabel_->setObjectName(QStringLiteral("hint"));
    bottomLayout_->addWidget(hintLabel_, 0, Qt::AlignVCenter);

    spawnPanel_ = new QWidget(bottomBar_);
    spawnLayout_ = new QHBoxLayout(spawnPanel_);
    spawnLayout_->setContentsMargins(0, 0, 0, 0);

    auto addParam = [this](const QString& name, int value, int max)
        -> QSpinBox* {
        auto* block = new QWidget(spawnPanel_);
        auto* layout = new QVBoxLayout(block);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        auto* label = new QLabel(name, block);
        label->setObjectName(QStringLiteral("spawnLabel"));
        auto* spin = new QSpinBox(block);
        spin->setObjectName(QStringLiteral("spawnSpin"));
        spin->setRange(0, max);
        spin->setValue(value);
        spin->setAlignment(Qt::AlignCenter);
        spin->setAccelerated(true);
        spin->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        QPalette spinPalette = spin->palette();
        spinPalette.setColor(QPalette::Button, QColor(0x1a, 0x20, 0x21));
        spinPalette.setColor(QPalette::ButtonText, QColor(0xc2, 0xc8, 0xb6));
        spinPalette.setColor(QPalette::Base, QColor(0x10, 0x13, 0x14));
        spinPalette.setColor(QPalette::Text, QColor(0xd8, 0xd1, 0xbf));
        spin->setPalette(spinPalette);
        layout->addWidget(label);
        layout->addWidget(spin);
        spawnLayout_->addWidget(block);
        return spin;
    };

    zombieSpin_ = addParam(QStringLiteral("ZOMBIES"), 8, 40);
    humanSpin_ = addParam(QStringLiteral("HUMANS"), 5, 30);
    treeSpin_ = addParam(QStringLiteral("TREES"), 2, 40);

    applyButton_ = new QPushButton(QStringLiteral("APPLY"), spawnPanel_);
    applyButton_->setObjectName(QStringLiteral("applyButton"));
    applyButton_->setCursor(Qt::PointingHandCursor);
    connect(applyButton_, &QPushButton::clicked, this,
            &MainWindow::applySpawnSettings);
    spawnLayout_->addWidget(applyButton_, 0, Qt::AlignBottom);

    bottomLayout_->addWidget(spawnPanel_, 0, Qt::AlignVCenter);

    skipButton_ = new QPushButton(QStringLiteral("SKIP TURN  +1 HOUR"), bottomBar_);
    skipButton_->setObjectName(QStringLiteral("skipButton"));
    skipButton_->setCursor(Qt::PointingHandCursor);
    connect(skipButton_, &QPushButton::clicked, this, &MainWindow::advanceTurn);
    bottomLayout_->addWidget(skipButton_, 0, Qt::AlignVCenter);

    root->addWidget(bottomBar_);
    setCentralWidget(central);

    setWindowTitle(QStringLiteral("ZomboGIG - Survival Isometric"));
    setMinimumSize(1120, 720);
    const QRect available =
        QGuiApplication::primaryScreen()->availableGeometry();
    resize(static_cast<int>(available.width() * 0.96),
           static_cast<int>(available.height() * 0.94));

    updateTimeLabels();
    refreshStats();

    applyUiScale();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    applyUiScale();
}

void MainWindow::applyUiScale() {
    if (!topBar_ || !bottomBar_ || !clock_) {
        return;
    }

    const qreal scale = std::clamp(height() / 1080.0, 0.9, 2.4);
    const auto px = [scale](qreal value) {
        return qRound(value * scale);
    };

    hintLabel_->setVisible(width() >= px(1360));

    if (std::abs(scale - uiScale_) < 0.01) {
        return;
    }
    uiScale_ = scale;

    topBar_->setFixedHeight(px(116));
    bottomBar_->setFixedHeight(px(112));
    clock_->setFixedSize(px(88), px(88));

    topLayout_->setContentsMargins(px(30), px(10), px(28), px(10));
    topLayout_->setSpacing(px(18));
    bottomLayout_->setContentsMargins(px(30), px(14), px(28), px(14));
    bottomLayout_->setSpacing(px(26));
    statsLayout_->setSpacing(px(38));
    spawnLayout_->setContentsMargins(0, 0, 0, 0);
    spawnLayout_->setSpacing(px(10));

    skipButton_->setFixedHeight(px(46));
    skipButton_->setMinimumWidth(px(224));

    zombieSpin_->setFixedHeight(px(32));
    humanSpin_->setFixedHeight(px(32));
    treeSpin_->setFixedHeight(px(32));
    zombieSpin_->setFixedWidth(px(72));
    humanSpin_->setFixedWidth(px(72));
    treeSpin_->setFixedWidth(px(72));
    applyButton_->setFixedHeight(px(34));
    applyButton_->setMinimumWidth(px(64));

    spawnPanel_->setVisible(width() >= px(1320));

    QString style;
    style += QStringLiteral(
        "QLabel { color: #b8b2a0; background: transparent; }");

    style += QStringLiteral(
                 "QLabel#gameTitle { color: #b2433a; font-size: %1px;"
                 " font-weight: bold; letter-spacing: %2px; }")
                 .arg(px(34))
                 .arg(px(6));
    style += QStringLiteral(
                 "QLabel#gameSubtitle { color: #a4a496; font-size: %1px;"
                 " letter-spacing: %2px; }")
                 .arg(px(11))
                 .arg(px(4));
    style += QStringLiteral(
                 "QLabel#timeValue { color: #d8d1bf; font-size: %1px;"
                 " font-weight: bold; letter-spacing: %2px; }")
                 .arg(px(31))
                 .arg(px(3));
    style += QStringLiteral(
                 "QLabel#dayLabel { color: #837d6d; font-size: %1px;"
                 " letter-spacing: %2px; }")
                 .arg(px(11))
                 .arg(px(3));
    style += QStringLiteral(
                 "QLabel#statLabel { color: #8a8474; font-size: %1px;"
                 " letter-spacing: %2px; }")
                 .arg(px(11))
                 .arg(px(2));
    style += QStringLiteral(
                 "QLabel#statValue { font-size: %1px; font-weight: bold; }")
                 .arg(px(27));
    style += QStringLiteral("QLabel#hint { color: #6f6a5e; font-size: %1px; }")
                 .arg(px(12));
    style += QStringLiteral(
                 "QPushButton#skipButton { background-color: #141718;"
                 " color: #d0705f; border: 1px solid #5a2a26;"
                 " padding: %1px %2px; font-weight: bold; font-size: %3px;"
                 " letter-spacing: %4px; }"
                 "QPushButton#skipButton:hover { background-color: #1c2021;"
                 " border-color: #8a3a32; }"
                 "QPushButton#skipButton:pressed { background-color: #2a1412; }")
                 .arg(px(8))
                 .arg(px(20))
                 .arg(px(13))
                 .arg(px(2));
    style += QStringLiteral(
                 "QLabel#spawnLabel { color: #8a8474; font-size: %1px;"
                 " letter-spacing: %2px; }")
                 .arg(px(10))
                 .arg(px(1));
    style += QStringLiteral(
                 "QSpinBox#spawnSpin { background-color: #101314;"
                 " color: #d8d1bf; border: 1px solid #2c3230;"
                 " padding: %1px %2px; font-size: %3px; font-weight: bold; }"
                 "QSpinBox#spawnSpin:focus { border-color: #5a2a26; }")
                 .arg(px(2))
                 .arg(px(4))
                 .arg(px(13));
    style += QStringLiteral(
                 "QPushButton#applyButton { background-color: #141718;"
                 " color: #b8b2a0; border: 1px solid #2c3230;"
                 " padding: %1px %2px; font-weight: bold; font-size: %3px;"
                 " letter-spacing: %4px; }"
                 "QPushButton#applyButton:hover { background-color: #1c2021;"
                 " border-color: #4a534f; color: #d8d1bf; }"
                 "QPushButton#applyButton:pressed { background-color: #2a1412; }")
                 .arg(px(6))
                 .arg(px(10))
                 .arg(px(10))
                 .arg(px(1));

    setStyleSheet(style);
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
    if (event->key() == Qt::Key_Escape && isFullScreen()) {
        showMaximized();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::clearWorld() {
    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            Cell* cell = grid_.at(row, col);
            cell->setProp(nullptr);
            cell->setEntity(nullptr);
        }
    }
}

void MainWindow::growPlants() {
    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            Prop* prop = grid_.at(row, col)->prop();
            if (prop) {
                prop->grow();
            }
        }
    }
}

void MainWindow::resolveAttacks() {
    const int offsets[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                               {0, 1},   {1, -1}, {1, 0},  {1, 1}};

    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            const Entity* zombie = grid_.at(row, col)->entity();
            if (!zombie || zombie->faction() != Faction::Zombie) {
                continue;
            }

            for (const auto& offset : offsets) {
                const int targetRow = row + offset[0];
                const int targetCol = col + offset[1];
                if (targetRow < 0 || targetCol < 0 ||
                    targetRow >= grid_.rows() || targetCol >= grid_.cols()) {
                    continue;
                }

                const Entity* human =
                    grid_.at(targetRow, targetCol)->entity();
                if (human && human->faction() == Faction::Human) {
                    map_->addBloodAt(targetRow, targetCol);
                    break;
                }
            }
        }
    }
}

void MainWindow::populateWorld(int zombies, int humans, int trees) {
    clearWorld();

    const int requestedZombies = std::max(0, zombies);
    const int requestedHumans = std::max(0, humans);
    const int requestedTrees = std::max(0, trees);

    auto placeProp = [this](int row, int col, PropKind kind) {
        if (row < 0 || col < 0 || row >= grid_.rows() || col >= grid_.cols()) {
            return;
        }
        Cell* cell = grid_.at(row, col);
        if (!cell->walkable() || cell->prop() || cell->entity()) {
            return;
        }
        cell->setProp(new Prop(kind));
    };

    auto placeHuman = [this](int row, int col, HumanType type) {
        if (row < 0 || col < 0 || row >= grid_.rows() || col >= grid_.cols()) {
            return;
        }
        Cell* cell = grid_.at(row, col);
        if (!cell->walkable() || cell->prop() || cell->entity()) {
            return;
        }
        cell->setEntity(new Human(type));
    };

    auto placeZombie = [this](int row, int col, ZombieType type) {
        if (row < 0 || col < 0 || row >= grid_.rows() || col >= grid_.cols()) {
            return;
        }
        Cell* cell = grid_.at(row, col);
        if (!cell->walkable() || cell->prop() || cell->entity()) {
            return;
        }
        cell->setEntity(new Zombie(type));
    };

    const int walls[][2] = {
        {4, 4}, {4, 5}, {4, 6}, {4, 7},
        {5, 4}, {5, 7},
        {6, 4}, {6, 7},
        {7, 4}, {7, 5}, {7, 7},
        {8, 4}, {8, 5}, {8, 6}};
    for (const auto& wall : walls) {
        placeProp(wall[0], wall[1], PropKind::Wall);
    }

    struct PropSpec {
        int row;
        int col;
        PropKind kind;
    };
    const PropSpec props[] = {
        {1, 5, PropKind::Bush},
        {10, 6, PropKind::Bush},
        {5, 10, PropKind::Bush},
        {6, 10, PropKind::Bush},
        {1, 9, PropKind::Bush},
        {9, 3, PropKind::Plant},
        {9, 4, PropKind::Plant},
        {10, 3, PropKind::Plant},
        {10, 4, PropKind::Plant},
        {8, 3, PropKind::Plant}};
    for (const PropSpec& prop : props) {
        placeProp(prop.row, prop.col, prop.kind);
    }

    placeProp(8, 7, PropKind::Campfire);

    struct HumanSpec {
        int row;
        int col;
        HumanType type;
    };
    const HumanSpec humanSpots[] = {
        {5, 5, HumanType::Farmer},
        {5, 6, HumanType::Builder},
        {6, 5, HumanType::Doctor},
        {6, 6, HumanType::Military},
        {9, 6, HumanType::Military}};
    const int baseHumans =
        std::min(requestedHumans, static_cast<int>(std::size(humanSpots)));
    for (int i = 0; i < baseHumans; ++i) {
        placeHuman(humanSpots[i].row, humanSpots[i].col, humanSpots[i].type);
    }

    struct ZombieSpec {
        int row;
        int col;
        ZombieType type;
    };
    const ZombieSpec zombieSpots[] = {
        {0, 3, ZombieType::Tank},
        {1, 10, ZombieType::Runner},
        {9, 5, ZombieType::Tank},
        {10, 8, ZombieType::Exploder},
        {3, 1, ZombieType::Runner},
        {8, 10, ZombieType::Exploder},
        {11, 6, ZombieType::Runner},
        {6, 0, ZombieType::Tank}};
    const int baseZombies =
        std::min(requestedZombies, static_cast<int>(std::size(zombieSpots)));
    for (int i = 0; i < baseZombies; ++i) {
        placeZombie(zombieSpots[i].row, zombieSpots[i].col,
                    zombieSpots[i].type);
    }

    const int treeSpots[][2] = {{2, 2}, {9, 9}};
    const int baseTrees = std::min(requestedTrees, 2);
    for (int i = 0; i < baseTrees; ++i) {
        placeProp(treeSpots[i][0], treeSpots[i][1], PropKind::Tree);
    }

    std::vector<QPoint> freeCells;
    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            const Cell* cell = grid_.at(row, col);
            if (cell->walkable() && !cell->prop() && !cell->entity()) {
                freeCells.emplace_back(col, row);
            }
        }
    }

    auto takeFree = [&freeCells]() -> QPoint {
        if (freeCells.empty()) {
            return QPoint(-1, -1);
        }
        const int index =
            QRandomGenerator::global()->bounded(
                static_cast<int>(freeCells.size()));
        const QPoint spot = freeCells[static_cast<size_t>(index)];
        freeCells[static_cast<size_t>(index)] = freeCells.back();
        freeCells.pop_back();
        return spot;
    };

    for (int i = baseTrees; i < requestedTrees; ++i) {
        const QPoint spot = takeFree();
        if (spot.x() < 0) {
            break;
        }
        placeProp(spot.y(), spot.x(), PropKind::Tree);
    }

    for (int i = baseHumans; i < requestedHumans; ++i) {
        const QPoint spot = takeFree();
        if (spot.x() < 0) {
            break;
        }
        const auto type = static_cast<HumanType>(i % 4);
        placeHuman(spot.y(), spot.x(), type);
    }

    for (int i = baseZombies; i < requestedZombies; ++i) {
        const QPoint spot = takeFree();
        if (spot.x() < 0) {
            break;
        }
        const auto type = static_cast<ZombieType>(i % 3);
        placeZombie(spot.y(), spot.x(), type);
    }
}

void MainWindow::applySpawnSettings() {
    if (!zombieSpin_ || !humanSpin_ || !treeSpin_ || !map_) {
        return;
    }
    map_->clearBlood();
    populateWorld(zombieSpin_->value(), humanSpin_->value(),
                  treeSpin_->value());
    map_->refreshWorld();
    refreshStats();
}

void MainWindow::refreshStats() {
    int zombies = 0;
    int humans = 0;
    int defense = 0;
    int food = 0;

    for (int row = 0; row < grid_.rows(); ++row) {
        for (int col = 0; col < grid_.cols(); ++col) {
            const Cell* cell = grid_.at(row, col);
            if (const Entity* entity = cell->entity()) {
                if (entity->faction() == Faction::Zombie) {
                    ++zombies;
                } else {
                    ++humans;
                }
                defense += entity->defense();
            }
            if (const Prop* prop = cell->prop()) {
                defense += prop->defense();
                food += prop->food();
            }
        }
    }

    zombieValue_->setText(QString::number(zombies));
    humanValue_->setText(QString::number(humans));
    defenseValue_->setText(QString::number(defense));
    foodValue_->setText(QString::number(food));
}

void MainWindow::advanceTurn() {
    totalMinutes_ += 60;
    growPlants();
    resolveAttacks();
    updateTimeLabels();
    map_->refreshWorld();
    map_->setMinutes(totalMinutes_);
    clock_->setMinutes(totalMinutes_);
    refreshStats();
}

void MainWindow::updateTimeLabels() {
    const int day = totalMinutes_ / (24 * 60) + 1;
    const int minutesOfDay = totalMinutes_ % (24 * 60);

    timeLabel_->setText(QStringLiteral("%1:%2")
                            .arg(minutesOfDay / 60, 2, 10, QLatin1Char('0'))
                            .arg(minutesOfDay % 60, 2, 10, QLatin1Char('0')));
    dayLabel_->setText(QStringLiteral("DAY %1").arg(day));
}
