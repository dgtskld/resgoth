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
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr auto modeWidthRole = Qt::UserRole;
constexpr auto modeHeightRole = Qt::UserRole + 1;
constexpr auto modeRefreshRateRole = Qt::UserRole + 2;
}

MainWindow::MainWindow() {
    setupUi();
    loadConfig();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveConfig();
    QMainWindow::closeEvent(event);
}

void MainWindow::setupUi() {
    setWindowTitle(QStringLiteral("Resgoth"));
    setMinimumWidth(500);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    auto *form = new QFormLayout();

    gamePathEdit = new QLineEdit(centralWidget);
    auto *browseButton = new QPushButton(QStringLiteral("Browse…"), centralWidget);
    auto *gamePathLayout = new QHBoxLayout();
    gamePathLayout->addWidget(gamePathEdit);
    gamePathLayout->addWidget(browseButton);
    form->addRow(QStringLiteral("Game:"), gamePathLayout);

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
    layout->addWidget(launchButton, 0, Qt::AlignRight);
    setCentralWidget(centralWidget);

    connect(browseButton, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Select Game"), gamePathEdit->text(),
                                                          QStringLiteral("Programs (*.exe);;All Files (*)"));
        if (!path.isEmpty()) {
            gamePathEdit->setText(path);
        }
    });
    connect(gamePathEdit, &QLineEdit::textChanged, this, [this] { updateValidation(); });
    connect(modeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { updateValidation(); });
    connect(launchButton, &QPushButton::clicked, this, [this] {
        statusLabel->setText(QStringLiteral("Game launching will be added in stage 4."));
    });
}

void MainWindow::loadConfig() {
    const AppConfig config = configStore.read();
    gamePathEdit->setText(config.gamePath);
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
    modeCombo->clear();
    for (const DisplayMode &mode : displayModes.modesFor(primaryDeviceName)) {
        modeCombo->addItem(mode.label());
        const int index = modeCombo->count() - 1;
        modeCombo->setItemData(index, mode.width, modeWidthRole);
        modeCombo->setItemData(index, mode.height, modeHeightRole);
        modeCombo->setItemData(index, mode.refreshRate, modeRefreshRateRole);
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
    const bool valid = gameExists && displaySelected;

    launchButton->setEnabled(valid);
    if (!gameExists) {
        statusLabel->setText(QStringLiteral("Select an existing game EXE file."));
    } else if (!displaySelected) {
        statusLabel->setText(QStringLiteral("Could not get modes for the primary display."));
    } else {
        statusLabel->setText(QStringLiteral("Configuration is valid. Settings will be saved next to the application."));
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
