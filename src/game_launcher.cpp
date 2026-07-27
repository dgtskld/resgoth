#include "game_launcher.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include <array>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

namespace {
constexpr int steamProcessPollIntervalMs = 500;
constexpr int steamProcessStartupTimeoutMs = 60000;

QString normalizedPath(const QString &path) {
    return QDir::cleanPath(QDir::fromNativeSeparators(QFileInfo(path).absoluteFilePath())).toCaseFolded();
}

QSet<qint64> matchingProcessIds(const QString &expectedPath) {
    QSet<qint64> result;
    const QString normalizedExpectedPath = normalizedPath(expectedPath);
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return result;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry)) {
        do {
            const HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
            if (!process) {
                continue;
            }

            std::array<wchar_t, 32768> imagePath{};
            DWORD imagePathLength = static_cast<DWORD>(imagePath.size());
            const bool hasImagePath = QueryFullProcessImageNameW(process, 0, imagePath.data(), &imagePathLength) != FALSE;
            CloseHandle(process);
            if (!hasImagePath) {
                continue;
            }

            if (normalizedPath(QString::fromWCharArray(imagePath.data(), static_cast<qsizetype>(imagePathLength))) ==
                normalizedExpectedPath) {
                result.insert(static_cast<qint64>(entry.th32ProcessID));
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return result;
}
} // namespace

GameLauncher::GameLauncher(QObject *parent)
    : QObject(parent) {
    connect(&process, &QProcess::started, this, [this] {
        state = State::DirectProcess;
        emit gameStarted();
    });
    connect(&process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](const int exitCode, const QProcess::ExitStatus exitStatus) {
                state = State::Idle;
                const QString message = exitStatus == QProcess::NormalExit
                                            ? QStringLiteral("Game exited with code %1.").arg(exitCode)
                                            : QStringLiteral("Game process crashed.");
                emit gameFinished(message);
            });
    connect(&process, &QProcess::errorOccurred, this, [this](const QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            state = State::Idle;
            emit launchError(process.errorString());
        }
    });
    steamProcessTimer.setInterval(steamProcessPollIntervalMs);
    connect(&steamProcessTimer, &QTimer::timeout, this, &GameLauncher::checkSteamProcess);
}

bool GameLauncher::launchDirect(const QString &gamePath, QString *errorMessage) {
    if (isRunning()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("A game launch is already in progress.");
        }
        return false;
    }

    const QFileInfo gameFile(gamePath);
    if (!gameFile.exists() || !gameFile.isFile()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("The selected game executable does not exist.");
        }
        return false;
    }

    process.setWorkingDirectory(gameFile.absolutePath());
    process.setProgram(gameFile.absoluteFilePath());
    process.setArguments({});
    process.start();
    return true;
}

bool GameLauncher::launchSteam(const QString &appId, const QString &gamePath, QString *errorMessage) {
    if (isRunning()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("A game launch is already in progress.");
        }
        return false;
    }

    const QFileInfo gameFile(gamePath);
    if (appId.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Select a Steam game before launching.");
        }
        return false;
    }
    if (!gameFile.exists() || !gameFile.isFile()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Select the game executable that Steam starts.");
        }
        return false;
    }

    expectedProcessPath = gameFile.absoluteFilePath();
    existingProcessIds = matchingProcessIds(expectedProcessPath);
    if (!existingProcessIds.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("The selected game executable is already running. Close it before launching through Steam.");
        }
        return false;
    }

    if (!QDesktopServices::openUrl(QUrl(QStringLiteral("steam://run/%1").arg(appId)))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Windows could not open the Steam launch link.");
        }
        return false;
    }

    state = State::WaitingForSteamProcess;
    steamStartupTimer.start();
    steamProcessTimer.start();
    return true;
}

bool GameLauncher::isRunning() const {
    return state != State::Idle || process.state() != QProcess::NotRunning;
}

void GameLauncher::checkSteamProcess() {
    const QSet<qint64> currentProcessIds = matchingProcessIds(expectedProcessPath);
    if (state == State::WaitingForSteamProcess) {
        for (const qint64 processId : currentProcessIds) {
            if (!existingProcessIds.contains(processId)) {
                trackedProcessId = processId;
                state = State::TrackingSteamProcess;
                emit gameStarted();
                return;
            }
        }
        if (steamStartupTimer.elapsed() >= steamProcessStartupTimeoutMs) {
            steamProcessTimer.stop();
            state = State::Idle;
            emit launchError(QStringLiteral("Steam did not start the selected game executable within 60 seconds."));
        }
        return;
    }

    if (state == State::TrackingSteamProcess && !currentProcessIds.contains(trackedProcessId)) {
        finishSteamTracking(QStringLiteral("Game process exited."));
    }
}

void GameLauncher::finishSteamTracking(const QString &message) {
    steamProcessTimer.stop();
    trackedProcessId = 0;
    expectedProcessPath.clear();
    existingProcessIds.clear();
    state = State::Idle;
    emit gameFinished(message);
}
