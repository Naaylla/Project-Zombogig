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
class QVBoxLayout;
class QWidget;
class SoundManager;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void populateWorld();
    void refreshStats();
    void advanceTurn();
    void updateTimeLabels();
    void toggleSound();
    void applyUiScale();

    Grid grid_;
    MapView* map_;
    ClockWidget* clock_;
    SoundManager* sound_;
    QWidget* topBar_;
    QWidget* bottomBar_;
    QWidget* statsHost_;
    QHBoxLayout* topLayout_;
    QHBoxLayout* bottomLayout_;
    QHBoxLayout* statsLayout_;
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
    QPushButton* soundButton_;
    qreal uiScale_;
    int totalMinutes_;
};
