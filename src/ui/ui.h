#pragma once

#include <QElapsedTimer>
#include <QMainWindow>
#include <QPoint>
#include <QPixmap>
#include <QVector>
#include <QWidget>

#include "logic/game.h"

class QKeyEvent;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QPushButton;
class QResizeEvent;
class QSpinBox;
class QTimer;
class QButtonGroup;

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
    int survivors() const;
    int zombies() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* title_;
    QLabel* subtitle_;
    QLabel* story_;
    QLabel* hint_;
    QSpinBox* survivorsSpin_;
    QSpinBox* zombiesSpin_;
    QPushButton* startButton_;
    QPushButton* quitButton_;
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
