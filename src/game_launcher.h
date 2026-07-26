#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

class GameLauncher final : public QObject {
    Q_OBJECT

public:
    explicit GameLauncher(QObject *parent = nullptr);

    [[nodiscard]] bool launch(const QString &gamePath, QString *errorMessage = nullptr);
    [[nodiscard]] bool isRunning() const;

signals:
    void gameStarted();
    void gameFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void launchError(QProcess::ProcessError error, const QString &message);

private:
    QProcess process;
};
