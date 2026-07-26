#include "main_window.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
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
    setWindowTitle(QStringLiteral("Resgoth"));
    setMinimumWidth(500);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    auto *form = new QFormLayout();

    steamGameCombo = new QComboBox(centralWidget);
    steamGameCombo->setToolTip(QStringLiteral("Choose an installed Steam game to open its folder in Browse. This does not choose an EXE."));
    auto *resetSteamButton = new QPushButton(QStringLiteral("Reset"), centralWidget);
    resetSteamButton->setToolTip(QStringLiteral("Clear the Steam game selection without rescanning libraries."));
    auto *steamGamesLayout = new QHBoxLayout();
    steamGamesLayout->addWidget(steamGameCombo);
    steamGamesLayout->addWidget(resetSteamButton);
    form->addRow(QStringLiteral("Steam game:"), steamGamesLayout);

    gamePathEdit = new QLineEdit(centralWidget);
    gamePathEdit->setPlaceholderText(QStringLiteral("Path to the game EXE"));
    gamePathEdit->setToolTip(QStringLiteral("Select the exact executable to launch. Resgoth starts this EXE directly."));
    auto *browseButton = new QPushButton(QStringLiteral("Browse…"), centralWidget);
    browseButton->setToolTip(QStringLiteral("Choose the game executable. A selected Steam game opens its install folder."));
    auto *gamePathLayout = new QHBoxLayout();
    gamePathLayout->addWidget(gamePathEdit);
    gamePathLayout->addWidget(browseButton);
    form->addRow(QStringLiteral("Game EXE:"), gamePathLayout);

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
    restoreButton = new QPushButton(QStringLiteral("Restore now"), centralWidget);
    restoreButton->setEnabled(false);
    restoreButton->setToolTip(QStringLiteral("Available after a launch mode has been applied."));
    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(restoreButton);
    buttonsLayout->addWidget(launchButton, 0, Qt::AlignRight);
    layout->addLayout(buttonsLayout);
    setCentralWidget(centralWidget);

    connect(browseButton, &QPushButton::clicked, this, [this] {
        QString initialDirectory = steamGameCombo->currentData().toString();
        if (initialDirectory.isEmpty()) {
            initialDirectory = gamePathEdit->text();
        }
        const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Select Game"), initialDirectory,
                                                          QStringLiteral("Programs (*.exe);;All Files (*)"));
        if (!path.isEmpty()) {
            gamePathEdit->setText(path);
        }
    });
    connect(gamePathEdit, &QLineEdit::textChanged, this, [this] { updateValidation(); });
    connect(resetSteamButton, &QPushButton::clicked, this, [this] { steamGameCombo->setCurrentIndex(0); });
    connect(modeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { updateValidation(); });
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
}

void MainWindow::loadConfig() {
    const AppConfig config = configStore.read();
    gamePathEdit->setText(config.gamePath);
    loadSteamGames();
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

void MainWindow::loadSteamGames() {
    steamGameCombo->clear();
    steamGameCombo->addItem(QStringLiteral("Manual selection"));
    for (const SteamGameInfo &game : steamLibraries.installedGames()) {
        steamGameCombo->addItem(QStringLiteral("%1 (%2)").arg(game.name, game.appId), game.installDirectory);
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
    const bool valid = displaySelected && gameExists;

    launchButton->setEnabled(valid && !gameLauncher.isRunning());
    if (!gameExists) {
        statusLabel->setText(QStringLiteral("Select an existing game EXE file."));
    } else if (!displaySelected) {
        statusLabel->setText(QStringLiteral("Could not get modes for the primary display."));
    } else {
        statusLabel->setText(QStringLiteral("Configuration is valid. Settings will be saved next to the application."));
    }
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
    if (!gameLauncher.launch(config.gamePath, &launchError)) {
        restoreDisplayMode();
        statusLabel->setText(launchError + QStringLiteral(" Original display mode restored."));
        return;
    }

    launchButton->setEnabled(false);
    restoreButton->setEnabled(true);
    statusLabel->setText(QStringLiteral("Applying launch mode and starting the game…"));
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
    config.monitorDeviceName = primaryDeviceName;
    const int modeIndex = modeCombo->currentIndex();
    if (modeIndex >= 0) {
        config.width = modeCombo->itemData(modeIndex, modeWidthRole).toInt();
        config.height = modeCombo->itemData(modeIndex, modeHeightRole).toInt();
        config.refreshRate = modeCombo->itemData(modeIndex, modeRefreshRateRole).toInt();
    }
    return config;
}
