#include "soundmanager.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRandomGenerator>
#include <QStandardPaths>

namespace {

constexpr int SAMPLE_RATE = 44100;
constexpr qreal PI = 3.14159265358979323846;

void appendLe32(QByteArray& data, quint32 value) {
    data.append(static_cast<char>(value & 0xFF));
    data.append(static_cast<char>((value >> 8) & 0xFF));
    data.append(static_cast<char>((value >> 16) & 0xFF));
    data.append(static_cast<char>((value >> 24) & 0xFF));
}

void appendLe16(QByteArray& data, quint16 value) {
    data.append(static_cast<char>(value & 0xFF));
    data.append(static_cast<char>((value >> 8) & 0xFF));
}

QByteArray makeWav(const std::vector<float>& samples) {
    const quint32 dataSize = static_cast<quint32>(samples.size()) * 2;
    QByteArray wav;
    wav.reserve(static_cast<int>(44 + dataSize));
    wav.append("RIFF", 4);
    appendLe32(wav, 36 + dataSize);
    wav.append("WAVE", 4);
    wav.append("fmt ", 4);
    appendLe32(wav, 16);
    appendLe16(wav, 1);
    appendLe16(wav, 1);
    appendLe32(wav, SAMPLE_RATE);
    appendLe32(wav, SAMPLE_RATE * 2);
    appendLe16(wav, 2);
    appendLe16(wav, 16);
    wav.append("data", 4);
    appendLe32(wav, dataSize);

    for (float value : samples) {
        const float clamped = std::clamp(value, -1.0f, 1.0f);
        appendLe16(wav, static_cast<quint16>(static_cast<qint16>(clamped * 32000.0f)));
    }
    return wav;
}

bool writeSound(const QString& path, const std::vector<float>& samples) {
    const QFileInfo info(path);
    if (info.exists() && info.size() > 44) {
        return true;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    const QByteArray wav = makeWav(samples);
    return file.write(wav) == wav.size();
}

std::vector<float> rainSamples() {
    const int count = SAMPLE_RATE * 4;
    std::vector<float> samples(count);
    QRandomGenerator rng(0x63D2u);
    qreal low = 0.0;
    qreal rumble = 0.0;
    for (int i = 0; i < count; ++i) {
        const qreal white = rng.generateDouble() * 2.0 - 1.0;
        low += 0.16 * (white - low);
        rumble += 0.02 * (white - rumble);
        const qreal mod = 0.85 + 0.15 * std::sin(2.0 * PI * 3.0 * i / count);
        samples[i] = static_cast<float>((low * 0.5 + rumble * 0.8) * 0.16 * mod);
    }
    return samples;
}

QString windowsPath(const QString& path) {
    if (path.startsWith("/mnt/") && path.size() > 7) {
        const QChar drive = path.at(5).toUpper();
        return QStringLiteral("%1:/%2").arg(drive).arg(path.mid(7));
    }
    return QDir::toNativeSeparators(path);
}

QString powershellPath() {
    const QString wslPath =
        QStringLiteral("/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe");
    if (QFileInfo::exists(wslPath)) {
        return wslPath;
    }
    return QStandardPaths::findExecutable(QStringLiteral("powershell.exe"));
}

QString shellQuote(const QString& value) {
    QString quoted = value;
    quoted.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QLatin1Char('\'') + quoted + QLatin1Char('\'');
}

}  // namespace

SoundManager::SoundManager(QObject* parent)
    : QObject(parent),
      ambienceProcess_(nullptr),
      ambienceEnabled_(true) {
    detectBackend();
    ensureSounds();
    setAmbienceEnabled(true);
}

SoundManager::~SoundManager() {
    stopAmbience();
}

void SoundManager::detectBackend() {
    if (qEnvironmentVariableIsSet("ZOMBOGIG_NO_SOUND")) {
        return;
    }
    if (!QStandardPaths::findExecutable(QStringLiteral("paplay")).isEmpty()) {
        backend_ = QStringLiteral("paplay");
    } else if (!QStandardPaths::findExecutable(QStringLiteral("pw-play")).isEmpty()) {
        backend_ = QStringLiteral("pw-play");
    } else if (!QStandardPaths::findExecutable(QStringLiteral("aplay")).isEmpty()) {
        backend_ = QStringLiteral("aplay");
    } else if (!powershellPath().isEmpty()) {
        backend_ = QStringLiteral("powershell");
    }
}

void SoundManager::ensureSounds() {
    if (backend_.isEmpty() || !soundDir_.isEmpty()) {
        return;
    }

    soundDir_ = QCoreApplication::applicationDirPath() + QStringLiteral("/sounds");
    if (!QDir().mkpath(soundDir_)) {
        soundDir_.clear();
        backend_.clear();
        return;
    }

    const bool ok = writeSound(soundDir_ + QStringLiteral("/rain.wav"),
                               rainSamples());
    if (!ok) {
        soundDir_.clear();
        backend_.clear();
    }
}

void SoundManager::setAmbienceEnabled(bool enabled) {
    ambienceEnabled_ = enabled;
    if (enabled) {
        startAmbience();
    } else {
        stopAmbience();
    }
}

void SoundManager::startAmbience() {
    stopAmbience();
    if (!ambienceEnabled_ || backend_.isEmpty() || soundDir_.isEmpty()) {
        return;
    }

    const QString rain = soundDir_ + QStringLiteral("/rain.wav");
    ambienceProcess_ = new QProcess(this);

    if (backend_ == QStringLiteral("powershell")) {
        const QString script = QStringLiteral(
            "$p = New-Object System.Media.SoundPlayer '%1'; "
            "$p.PlayLooping(); Start-Sleep -Seconds 86400")
                                   .arg(windowsPath(rain));
        ambienceProcess_->setProgram(powershellPath());
        ambienceProcess_->setArguments({QStringLiteral("-NoProfile"),
                                        QStringLiteral("-NonInteractive"),
                                        QStringLiteral("-WindowStyle"),
                                        QStringLiteral("Hidden"),
                                        QStringLiteral("-Command"),
                                        script});
    } else {
        const QString command = QStringLiteral("while true; do %1 %2; done")
                                    .arg(backend_, shellQuote(rain));
        ambienceProcess_->setProgram(QStringLiteral("bash"));
        ambienceProcess_->setArguments({QStringLiteral("-c"), command});
    }

    ambienceProcess_->start();
}

void SoundManager::stopAmbience() {
    if (!ambienceProcess_) {
        return;
    }
    ambienceProcess_->kill();
    ambienceProcess_->waitForFinished(300);
    delete ambienceProcess_;
    ambienceProcess_ = nullptr;
}
