#include "mainwindow.h"

#include <algorithm>
#include <cmath>

#include <QColor>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>

#include "clockwidget.h"
#include "entities/human.h"
#include "entities/zombie.h"
#include "mapview.h"
#include "prop.h"
#include "soundmanager.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      grid_(12, 12),
      map_(nullptr),
      clock_(nullptr),
      sound_(nullptr),
      topBar_(nullptr),
      bottomBar_(nullptr),
      statsHost_(nullptr),
      topLayout_(nullptr),
      bottomLayout_(nullptr),
      statsLayout_(nullptr),
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
      soundButton_(nullptr),
      uiScale_(0.0),
      totalMinutes_(20 * 60) {
    populateWorld();

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    topBar_ = new QWidget(central);
    topBar_->setObjectName(QStringLiteral("topBar"));
    topBar_->setAttribute(Qt::WA_StyledBackground, true);

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

    bottomBar_ = new QWidget(central);
    bottomBar_->setObjectName(QStringLiteral("bottomBar"));
    bottomBar_->setAttribute(Qt::WA_StyledBackground, true);

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

    soundButton_ = new QPushButton(bottomBar_);
    soundButton_->setObjectName(QStringLiteral("soundButton"));
    soundButton_->setCursor(Qt::PointingHandCursor);
    connect(soundButton_, &QPushButton::clicked, this, &MainWindow::toggleSound);
    bottomLayout_->addWidget(soundButton_, 0, Qt::AlignVCenter);

    skipButton_ = new QPushButton(QStringLiteral("SKIP TURN  +1 HOUR"), bottomBar_);
    skipButton_->setObjectName(QStringLiteral("skipButton"));
    skipButton_->setCursor(Qt::PointingHandCursor);
    connect(skipButton_, &QPushButton::clicked, this, &MainWindow::advanceTurn);
    bottomLayout_->addWidget(skipButton_, 0, Qt::AlignVCenter);

    root->addWidget(bottomBar_);
    setCentralWidget(central);

    sound_ = new SoundManager(this);
    soundButton_->setText(sound_->available() ? QStringLiteral("SOUND ON")
                                              : QStringLiteral("SOUND N/A"));
    soundButton_->setEnabled(sound_->available());

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

    soundButton_->setFixedHeight(px(42));
    soundButton_->setMinimumWidth(px(126));
    skipButton_->setFixedHeight(px(46));
    skipButton_->setMinimumWidth(px(224));

    QString style;
    style += QStringLiteral(
        "QWidget#topBar { background-color: #0a0d0e;"
        " border-bottom: 1px solid #1b2021; }"
        "QWidget#bottomBar { background-color: #0a0d0e;"
        " border-top: 1px solid #1b2021; }"
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
                 "QPushButton#soundButton { background-color: #101314;"
                 " color: #9aa08f; border: 1px solid #2c3230;"
                 " padding: %1px %2px; font-size: %3px; font-weight: bold;"
                 " letter-spacing: %4px; }"
                 "QPushButton#soundButton:hover { border-color: #4a534f;"
                 " color: #c2c8b6; }"
                 "QPushButton#soundButton:disabled { color: #4f544c;"
                 " border-color: #1d2120; }")
                 .arg(px(6))
                 .arg(px(14))
                 .arg(px(11))
                 .arg(px(2));

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

void MainWindow::populateWorld() {
    auto placeProp = [this](int row, int col, PropKind kind) {
        if (row < 0 || col < 0 || row >= grid_.rows() || col >= grid_.cols()) {
            return;
        }
        Cell* cell = grid_.at(row, col);
        if (cell->prop() || cell->entity()) {
            return;
        }
        cell->setProp(new Prop(kind));
    };

    auto placeHuman = [this](int row, int col, HumanType type) {
        if (row < 0 || col < 0 || row >= grid_.rows() || col >= grid_.cols()) {
            return;
        }
        Cell* cell = grid_.at(row, col);
        if (cell->prop() || cell->entity()) {
            return;
        }
        cell->setEntity(new Human(type));
    };

    auto placeZombie = [this](int row, int col, ZombieType type) {
        if (row < 0 || col < 0 || row >= grid_.rows() || col >= grid_.cols()) {
            return;
        }
        Cell* cell = grid_.at(row, col);
        if (cell->prop() || cell->entity()) {
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
        {2, 9, PropKind::Crate},
        {3, 9, PropKind::Barrel},
        {9, 2, PropKind::Rock},
        {2, 2, PropKind::Tree},
        {9, 9, PropKind::Tree},
        {1, 5, PropKind::Bush},
        {10, 6, PropKind::Bush},
        {8, 2, PropKind::Rock},
        {5, 10, PropKind::Bush},
        {6, 10, PropKind::Bush},
        {10, 9, PropKind::Corpse},
        {1, 9, PropKind::Bush}};
    for (const PropSpec& prop : props) {
        placeProp(prop.row, prop.col, prop.kind);
    }

    placeProp(8, 7, PropKind::Campfire);

    struct HumanSpec {
        int row;
        int col;
        HumanType type;
    };
    const HumanSpec humans[] = {
        {5, 5, HumanType::Soldier},
        {5, 6, HumanType::Engineer},
        {6, 5, HumanType::Doctor},
        {6, 6, HumanType::Survivor},
        {9, 6, HumanType::Survivor}};
    for (const HumanSpec& human : humans) {
        placeHuman(human.row, human.col, human.type);
    }

    struct ZombieSpec {
        int row;
        int col;
        ZombieType type;
    };
    const ZombieSpec zombies[] = {
        {0, 3, ZombieType::Walker},
        {1, 10, ZombieType::Runner},
        {9, 5, ZombieType::Brute},
        {10, 8, ZombieType::Crawler},
        {3, 1, ZombieType::Walker},
        {8, 10, ZombieType::Runner},
        {11, 6, ZombieType::Walker},
        {6, 0, ZombieType::Runner}};
    for (const ZombieSpec& zombie : zombies) {
        placeZombie(zombie.row, zombie.col, zombie.type);
    }
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
    updateTimeLabels();
    map_->setMinutes(totalMinutes_);
    clock_->setMinutes(totalMinutes_);
}

void MainWindow::updateTimeLabels() {
    const int day = totalMinutes_ / (24 * 60) + 1;
    const int minutesOfDay = totalMinutes_ % (24 * 60);

    timeLabel_->setText(QStringLiteral("%1:%2")
                            .arg(minutesOfDay / 60, 2, 10, QLatin1Char('0'))
                            .arg(minutesOfDay % 60, 2, 10, QLatin1Char('0')));
    dayLabel_->setText(QStringLiteral("DAY %1").arg(day));
}

void MainWindow::toggleSound() {
    if (!sound_ || !sound_->available()) {
        return;
    }
    sound_->setAmbienceEnabled(!sound_->ambienceEnabled());
    soundButton_->setText(sound_->ambienceEnabled() ? QStringLiteral("SOUND ON")
                                                    : QStringLiteral("SOUND OFF"));
}
