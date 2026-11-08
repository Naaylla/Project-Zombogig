#pragma once

#include <QObject>
#include <QString>

class QProcess;

class SoundManager : public QObject {
    Q_OBJECT

public:
    explicit SoundManager(QObject* parent = nullptr);
    ~SoundManager() override;

    bool available() const { return !backend_.isEmpty(); }
    bool ambienceEnabled() const { return ambienceEnabled_; }

    void setAmbienceEnabled(bool enabled);

private:
    void detectBackend();
    void ensureSounds();
    void startAmbience();
    void stopAmbience();

    QString backend_;
    QString soundDir_;
    QProcess* ambienceProcess_;
    bool ambienceEnabled_;
};
