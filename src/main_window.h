#pragma once

#include <QMainWindow>

#include "app_config.h"
#include "display_mode_service.h"
#include "game_launcher.h"
#include "steam_library_service.h"

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
    void updateLaunchMethodUi();
    void saveConfig() const;
    void updateValidation();
    void loadSteamGames();
    void applyDisplayMode();
    void launchGame();
    void restoreDisplayMode();
    [[nodiscard]] AppConfig currentConfig() const;

    AppConfigStore configStore;
    DisplayModeService displayModes;
    GameLauncher gameLauncher;
    SteamLibraryService steamLibraries;
    QLineEdit *gamePathEdit = nullptr;
    QComboBox *launchMethodCombo = nullptr;
    QComboBox *steamGameCombo = nullptr;
    QLabel *steamGameLabel = nullptr;
    QWidget *steamGameField = nullptr;
    QLabel *gamePathLabel = nullptr;
    QLabel *displayLabel = nullptr;
    QLabel *currentModeLabel = nullptr;
    QComboBox *modeCombo = nullptr;
    QLabel *statusLabel = nullptr;
    QPushButton *applyButton = nullptr;
    QPushButton *launchButton = nullptr;
    QPushButton *restoreButton = nullptr;
    QString primaryDeviceName;
};
