#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QProcess>
#include <QSet>
#include <QString>
#include <QTimer>

class GameLauncher final : public QObject {
    Q_OBJECT

public:
    explicit GameLauncher(QObject *parent = nullptr);

    [[nodiscard]] bool launchDirect(const QString &gamePath, QString *errorMessage = nullptr);
    [[nodiscard]] bool launchSteam(const QString &appId, const QString &gamePath, QString *errorMessage = nullptr);
    [[nodiscard]] bool isRunning() const;

signals:
    void gameStarted();
    void gameFinished(const QString &message);
    void launchError(const QString &message);

private:
    enum class State {
        Idle,
        DirectProcess,
        WaitingForSteamProcess,
        TrackingSteamProcess,
    };

    void checkSteamProcess();
    void finishSteamTracking(const QString &message);

    QProcess process;
    QTimer steamProcessTimer;
    QElapsedTimer steamStartupTimer;
    QSet<qint64> existingProcessIds;
    qint64 trackedProcessId = 0;
    QString expectedProcessPath;
    State state = State::Idle;
};
