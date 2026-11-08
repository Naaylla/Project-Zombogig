#pragma once

#include <QMainWindow>

#include "grid.h"

class ClockWidget;
class MapView;
class QHBoxLayout;
class QKeyEvent;
class QLabel;
class QPushButton;
class QResizeEvent;
class QSpinBox;
class QVBoxLayout;
class QWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void populateWorld(int zombies = 8, int humans = 5, int trees = 2);
    void clearWorld();
    void growPlants();
    void resolveAttacks();
    void applySpawnSettings();
    void refreshStats();
    void advanceTurn();
    void updateTimeLabels();
    void applyUiScale();

    Grid grid_;
    MapView* map_;
    ClockWidget* clock_;
    QWidget* topBar_;
    QWidget* bottomBar_;
    QWidget* statsHost_;
    QWidget* spawnPanel_;
    QHBoxLayout* topLayout_;
    QHBoxLayout* bottomLayout_;
    QHBoxLayout* statsLayout_;
    QHBoxLayout* spawnLayout_;
    QLabel* titleLabel_;
    QLabel* subtitleLabel_;
    QLabel* timeLabel_;
    QLabel* dayLabel_;
    QLabel* zombieValue_;
    QLabel* humanValue_;
    QLabel* defenseValue_;
    QLabel* foodValue_;
    QLabel* hintLabel_;
    QPushButton* skipButton_;
    QPushButton* applyButton_;
    QSpinBox* zombieSpin_;
    QSpinBox* humanSpin_;
    QSpinBox* treeSpin_;
    qreal uiScale_;
    int totalMinutes_;
};
