#pragma once

#include <QPoint>
#include <QPixmap>
#include <QSize>
#include <QVector>
#include <QWidget>

#include "grid.h"

class QMouseEvent;
class QPainter;
class QPaintEvent;
class QResizeEvent;
class QTimer;

class MapView : public QWidget {
    Q_OBJECT

public:
    explicit MapView(Grid& grid, QWidget* parent = nullptr);

    QSize sizeHint() const override;
    void setMinutes(int minutes);
    void refreshWorld();
    void addBloodAt(int row, int col);
    void clearBlood();
    bool isNight() const;

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

    struct BloodStain {
        int row;
        int col;
        quint32 seed;
    };

    void buildMapCache();
    void rebuildScene();
    void updateMapPlacement();
    void drawBackground(QPainter& painter);
    void drawMap(QPainter& painter);
    void drawAmbient(QPainter& painter);
    void drawCampfireGlow(QPainter& painter);
    void drawHover(QPainter& painter);
    void drawRain(QPainter& painter);
    void drawTooltip(QPainter& painter);
    void buildVignette();
    void initRain();
    void stepRain();

    qreal daylight() const;
    QPointF cacheToWidget(qreal x, qreal y) const;
    bool cellAt(const QPointF& point, int& row, int& col) const;

    Grid& grid_;
    QPixmap mapCache_;
    QPixmap scene_;
    QPixmap vignette_;
    QVector<Drop> drops_;
    QVector<BloodStain> bloodStains_;
    QTimer* rainTimer_;
    QPoint mousePos_;
    int hoverRow_;
    int hoverCol_;
    int minutes_;
    qreal mapScale_;
    QPoint mapTopLeft_;
};
