#pragma once

#include <QMainWindow>

#include "app_config.h"
#include "display_mode_service.h"
#include "game_launcher.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

class MainWindow final : public QMainWindow {
public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUi();
    void loadConfig();
    void loadPrimaryDisplay();
    void updateCurrentMode();
    void reloadModes();
    void saveConfig() const;
    void updateValidation();
    void launchGame();
    void restoreDisplayMode();
    [[nodiscard]] AppConfig currentConfig() const;

    AppConfigStore configStore;
    DisplayModeService displayModes;
    GameLauncher gameLauncher;
    QLineEdit *gamePathEdit = nullptr;
    QLabel *displayLabel = nullptr;
    QLabel *currentModeLabel = nullptr;
    QComboBox *modeCombo = nullptr;
    QLabel *statusLabel = nullptr;
    QPushButton *launchButton = nullptr;
    QString primaryDeviceName;
};
