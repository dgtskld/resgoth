#include "game_launcher.h"

#include <QFileInfo>

GameLauncher::GameLauncher(QObject *parent)
    : QObject(parent) {
    connect(&process, &QProcess::started, this, &GameLauncher::gameStarted);
    connect(&process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](const int exitCode, const QProcess::ExitStatus exitStatus) {
                const QString message = exitStatus == QProcess::NormalExit
                                            ? QStringLiteral("Game exited with code %1.").arg(exitCode)
                                            : QStringLiteral("Game process crashed.");
                emit gameFinished(message);
            });
    connect(&process, &QProcess::errorOccurred, this, [this](const QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            emit launchError(process.errorString());
        }
    });
}

bool GameLauncher::launch(const QString &gamePath, QString *errorMessage) {
    if (isRunning()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("A game process is already running.");
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

bool GameLauncher::isRunning() const {
    return process.state() != QProcess::NotRunning;
}
