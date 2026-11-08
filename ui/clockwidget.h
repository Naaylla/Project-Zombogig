#pragma once

#include <QWidget>

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
