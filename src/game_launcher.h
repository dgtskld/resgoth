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
    void gameFinished(const QString &message);
    void launchError(const QString &message);

private:
    QProcess process;
};
