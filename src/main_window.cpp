#include "main_window.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr auto modeWidthRole = Qt::UserRole;
constexpr auto modeHeightRole = Qt::UserRole + 1;
constexpr auto modeRefreshRateRole = Qt::UserRole + 2;
constexpr auto steamAppIdRole = Qt::UserRole + 3;
const QString steamLaunchMethod = QStringLiteral("steam");
const QString manualLaunchMethod = QStringLiteral("manual");

bool isCommonResolution(const DisplayMode &mode) {
    static const QSet<QPair<int, int>> commonResolutions{
        {640, 480}, {720, 480}, {720, 576}, {800, 600}, {1024, 768}, {1152, 864},
        {1280, 720}, {1280, 768}, {1280, 800}, {1280, 960}, {1280, 1024},
        {1360, 768}, {1366, 768}, {1440, 900}, {1536, 864}, {1600, 900}, {1600, 1200},
        {1680, 1050}, {1920, 1080}, {1920, 1200}, {2048, 1152}, {2560, 1080},
        {2560, 1440}, {2560, 1600}, {2880, 1620}, {3200, 1800}, {3440, 1440},
        {3840, 1600}, {3840, 2160}, {5120, 1440}, {5120, 2160}, {7680, 2160}, {7680, 4320},
    };
    return commonResolutions.contains({mode.width, mode.height});
}
}

MainWindow::MainWindow() {
    setupUi();
    loadConfig();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    restoreDisplayMode();
    saveConfig();
    QMainWindow::closeEvent(event);
}

void MainWindow::setupUi() {
    setWindowIcon(QIcon(QStringLiteral(":/resgoth.png")));
    setWindowTitle(QStringLiteral("Resgoth"));
    setMinimumWidth(500);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    auto *form = new QFormLayout();

    launchMethodCombo = new QComboBox(centralWidget);
    launchMethodCombo->addItem(QStringLiteral("Steam"), steamLaunchMethod);
    launchMethodCombo->addItem(QStringLiteral("Manual EXE"), manualLaunchMethod);
    launchMethodCombo->setToolTip(QStringLiteral("Choose whether to launch a discovered Steam game or start an EXE directly."));
    form->addRow(QStringLiteral("Launch via:"), launchMethodCombo);

    steamGameCombo = new QComboBox(centralWidget);
    steamGameCombo->setToolTip(QStringLiteral("Choose the Steam game to launch. Also select its final game EXE below: Resgoth uses that EXE only to detect when the game exits and restore the display mode. If Steam starts a launcher first, choose the EXE it starts afterward."));
    auto *resetSteamButton = new QPushButton(QStringLiteral("Reset"), centralWidget);
    resetSteamButton->setToolTip(QStringLiteral("Clear the Steam game selection and selected EXE without rescanning libraries."));
    steamGameField = new QWidget(centralWidget);
    auto *steamGamesLayout = new QHBoxLayout(steamGameField);
    steamGamesLayout->setContentsMargins(0, 0, 0, 0);
    steamGamesLayout->addWidget(steamGameCombo);
    steamGamesLayout->addWidget(resetSteamButton);
    steamGameLabel = new QLabel(QStringLiteral("Steam game:"), centralWidget);
    form->addRow(steamGameLabel, steamGameField);

    gamePathEdit = new QLineEdit(centralWidget);
    gamePathEdit->setPlaceholderText(QStringLiteral("Path to the game EXE"));
    auto *browseButton = new QPushButton(QStringLiteral("Browse…"), centralWidget);
    auto *gamePathLayout = new QHBoxLayout();
    gamePathLayout->addWidget(gamePathEdit);
    gamePathLayout->addWidget(browseButton);
    gamePathLabel = new QLabel(centralWidget);
    form->addRow(gamePathLabel, gamePathLayout);

    displayLabel = new QLabel(centralWidget);
    displayLabel->setWordWrap(true);
    form->addRow(QStringLiteral("Display:"), displayLabel);

    currentModeLabel = new QLabel(centralWidget);
    form->addRow(QStringLiteral("Current mode:"), currentModeLabel);

    modeCombo = new QComboBox(centralWidget);
    form->addRow(QStringLiteral("Launch mode:"), modeCombo);

    layout->addLayout(form);
    statusLabel = new QLabel(centralWidget);
    statusLabel->setWordWrap(true);
    layout->addWidget(statusLabel);

    launchButton = new QPushButton(QStringLiteral("Launch"), centralWidget);
    applyButton = new QPushButton(QStringLiteral("Apply now"), centralWidget);
    applyButton->setToolTip(QStringLiteral("Apply the selected display mode without starting the game."));
    restoreButton = new QPushButton(QStringLiteral("Restore now"), centralWidget);
    restoreButton->setEnabled(false);
    restoreButton->setToolTip(QStringLiteral("Available after a launch mode has been applied."));
    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(restoreButton);
    buttonsLayout->addWidget(launchButton, 0, Qt::AlignRight);
    layout->addLayout(buttonsLayout);
    setCentralWidget(centralWidget);

    connect(browseButton, &QPushButton::clicked, this, [this] {
        QString initialDirectory = QStringLiteral("C:/");
        if (launchMethodCombo->currentData().toString() == steamLaunchMethod) {
            initialDirectory = steamGameCombo->currentData().toString();
        }
        const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Select Game"), initialDirectory,
                                                          QStringLiteral("Programs (*.exe);;All Files (*)"));
        if (!path.isEmpty()) {
            gamePathEdit->setText(path);
        }
    });
    connect(gamePathEdit, &QLineEdit::textChanged, this, [this] { updateValidation(); });
    connect(launchMethodCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        updateLaunchMethodUi();
        updateValidation();
    });
    connect(resetSteamButton, &QPushButton::clicked, this, [this] {
        steamGameCombo->setCurrentIndex(0);
        gamePathEdit->clear();
    });
    connect(modeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { updateValidation(); });
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::applyDisplayMode);
    connect(launchButton, &QPushButton::clicked, this, &MainWindow::launchGame);
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::restoreDisplayMode);
    connect(&gameLauncher, &GameLauncher::gameStarted, this, [this] {
        statusLabel->setText(QStringLiteral("Game is running. The original display mode will be restored when it exits."));
    });
    connect(&gameLauncher, &GameLauncher::gameFinished, this, [this](const QString &message) {
        restoreDisplayMode();
        statusLabel->setText(message + QStringLiteral(" Original display mode restored."));
        updateValidation();
    });
    connect(&gameLauncher, &GameLauncher::launchError, this, [this](const QString &message) {
        restoreDisplayMode();
        statusLabel->setText(QStringLiteral("Could not start the game: %1 Original display mode restored.").arg(message));
        updateValidation();
    });

    updateLaunchMethodUi();
}

void MainWindow::loadConfig() {
    const AppConfig config = configStore.read();
    gamePathEdit->setText(config.gamePath);
    const int launchMethodIndex = launchMethodCombo->findData(config.launchMethod);
    launchMethodCombo->setCurrentIndex(launchMethodIndex >= 0 ? launchMethodIndex : 0);
    loadSteamGames();
    for (int index = 1; index < steamGameCombo->count(); ++index) {
        if (steamGameCombo->itemData(index, steamAppIdRole).toString() == config.steamAppId) {
            steamGameCombo->setCurrentIndex(index);
            break;
        }
    }
    loadPrimaryDisplay();
    reloadModes();
    for (int index = 0; index < modeCombo->count(); ++index) {
        if (modeCombo->itemData(index, modeWidthRole).toInt() == config.width &&
            modeCombo->itemData(index, modeHeightRole).toInt() == config.height &&
            modeCombo->itemData(index, modeRefreshRateRole).toInt() == config.refreshRate) {
            modeCombo->setCurrentIndex(index);
            break;
        }
    }
    updateValidation();
}

void MainWindow::updateLaunchMethodUi() {
    const bool usesSteam = launchMethodCombo->currentData().toString() == steamLaunchMethod;
    steamGameLabel->setVisible(usesSteam);
    steamGameField->setVisible(usesSteam);
    gamePathLabel->setText(usesSteam ? QStringLiteral("Game process EXE:") : QStringLiteral("Game EXE:"));
    gamePathEdit->setToolTip(usesSteam
                                 ? QStringLiteral("Select the final game EXE. Steam launches the game; Resgoth uses this path to detect when it exits. If a launcher starts another EXE, select that final EXE.")
                                 : QStringLiteral("Select the exact executable to launch directly."));
}

void MainWindow::loadSteamGames() {
    steamGameCombo->clear();
    steamGameCombo->addItem(QStringLiteral("Manual selection"));
    for (const SteamGameInfo &game : steamLibraries.installedGames()) {
        steamGameCombo->addItem(QStringLiteral("%1 (%2)").arg(game.name, game.appId), game.installDirectory);
        steamGameCombo->setItemData(steamGameCombo->count() - 1, game.appId, steamAppIdRole);
    }
}

void MainWindow::loadPrimaryDisplay() {
    const std::optional<MonitorInfo> primaryMonitor = displayModes.primaryMonitor();
    if (!primaryMonitor) {
        primaryDeviceName.clear();
        displayLabel->setText(QStringLiteral("Primary display not found"));
        currentModeLabel->setText(QStringLiteral("Unavailable"));
        return;
    }
    primaryDeviceName = primaryMonitor->deviceName;
    displayLabel->setText(QStringLiteral("Primary — %1").arg(primaryMonitor->displayName));
    updateCurrentMode();
}

void MainWindow::updateCurrentMode() {
    const std::optional<DisplayMode> currentMode = displayModes.currentMode(primaryDeviceName);
    currentModeLabel->setText(currentMode ? currentMode->label() : QStringLiteral("Unavailable"));
}

void MainWindow::reloadModes() {
    const int previousWidth = modeCombo->currentData(modeWidthRole).toInt();
    const int previousHeight = modeCombo->currentData(modeHeightRole).toInt();
    const int previousRefreshRate = modeCombo->currentData(modeRefreshRateRole).toInt();
    modeCombo->clear();
    for (const DisplayMode &mode : displayModes.modesFor(primaryDeviceName)) {
        if (!isCommonResolution(mode)) {
            continue;
        }
        modeCombo->addItem(mode.label());
        const int index = modeCombo->count() - 1;
        modeCombo->setItemData(index, mode.width, modeWidthRole);
        modeCombo->setItemData(index, mode.height, modeHeightRole);
        modeCombo->setItemData(index, mode.refreshRate, modeRefreshRateRole);
        if (mode.width == previousWidth && mode.height == previousHeight && mode.refreshRate == previousRefreshRate) {
            modeCombo->setCurrentIndex(index);
        }
    }
    updateValidation();
}

void MainWindow::saveConfig() const {
    configStore.write(currentConfig());
}

void MainWindow::updateValidation() {
    const QFileInfo gameFile(gamePathEdit->text());
    const bool gameExists = gameFile.exists() && gameFile.isFile();
    const bool displaySelected = !primaryDeviceName.isEmpty() && modeCombo->currentIndex() >= 0;
    const bool usesSteam = launchMethodCombo->currentData().toString() == steamLaunchMethod;
    const bool steamGameSelected = !steamGameCombo->currentData(steamAppIdRole).toString().isEmpty();
    const bool valid = displaySelected && gameExists && (!usesSteam || steamGameSelected);

    launchButton->setEnabled(valid && !gameLauncher.isRunning());
    applyButton->setEnabled(valid && !gameLauncher.isRunning());
    if (!gameExists) {
        statusLabel->setText(usesSteam ? QStringLiteral("Select the final game EXE that Steam starts.")
                                       : QStringLiteral("Select an existing game EXE file."));
    } else if (usesSteam && !steamGameSelected) {
        statusLabel->setText(QStringLiteral("Select a Steam game to launch."));
    } else if (!displaySelected) {
        statusLabel->setText(QStringLiteral("Could not get modes for the primary display."));
    } else {
        statusLabel->setText(QStringLiteral("Configuration is valid. Settings will be saved next to the application."));
    }
}

void MainWindow::applyDisplayMode() {
    if (gameLauncher.isRunning()) {
        return;
    }

    const AppConfig config = currentConfig();
    const DisplayMode requestedMode{config.width, config.height, config.refreshRate};
    const DisplayOperationResult modeResult = displayModes.applyMode(config.monitorDeviceName, requestedMode);
    if (!modeResult.succeeded) {
        statusLabel->setText(QStringLiteral("Could not apply the launch mode: %1").arg(modeResult.error));
        return;
    }

    updateCurrentMode();
    restoreButton->setEnabled(true);
    statusLabel->setText(QStringLiteral("Launch mode applied. Select Restore now to return to the original mode."));
}

void MainWindow::launchGame() {
    if (gameLauncher.isRunning()) {
        return;
    }

    const AppConfig config = currentConfig();
    const DisplayMode requestedMode{config.width, config.height, config.refreshRate};
    const DisplayOperationResult modeResult = displayModes.applyMode(config.monitorDeviceName, requestedMode);
    if (!modeResult.succeeded) {
        statusLabel->setText(QStringLiteral("Could not apply the launch mode: %1").arg(modeResult.error));
        return;
    }

    QString launchError;
    const bool usesSteam = config.launchMethod == steamLaunchMethod;
    const bool launched = usesSteam ? gameLauncher.launchSteam(config.steamAppId, config.gamePath, &launchError)
                                    : gameLauncher.launchDirect(config.gamePath, &launchError);
    if (!launched) {
        restoreDisplayMode();
        statusLabel->setText(launchError + QStringLiteral(" Original display mode restored."));
        return;
    }

    launchButton->setEnabled(false);
    applyButton->setEnabled(false);
    restoreButton->setEnabled(true);
    statusLabel->setText(usesSteam ? QStringLiteral("Launch mode applied. Starting Steam and waiting for the game process…")
                                   : QStringLiteral("Applying launch mode and starting the game…"));
}

void MainWindow::restoreDisplayMode() {
    if (!displayModes.hasSavedMode()) {
        return;
    }

    const DisplayOperationResult restoreResult = displayModes.restoreMode();
    updateCurrentMode();
    restoreButton->setEnabled(false);
    if (!restoreResult.succeeded) {
        statusLabel->setText(QStringLiteral("Could not restore the original display mode: %1").arg(restoreResult.error));
    }
}

AppConfig MainWindow::currentConfig() const {
    AppConfig config;
    config.gamePath = gamePathEdit->text();
    config.steamAppId = steamGameCombo->currentData(steamAppIdRole).toString();
    config.launchMethod = launchMethodCombo->currentData().toString();
    config.monitorDeviceName = primaryDeviceName;
    const int modeIndex = modeCombo->currentIndex();
    if (modeIndex >= 0) {
        config.width = modeCombo->itemData(modeIndex, modeWidthRole).toInt();
        config.height = modeCombo->itemData(modeIndex, modeHeightRole).toInt();
        config.refreshRate = modeCombo->itemData(modeIndex, modeRefreshRateRole).toInt();
    }
    return config;
}
