#pragma once

#include <memory>
#include <optional>
#include <QString>
#include <QVector>

struct MonitorInfo {
    QString deviceName;
    QString displayName;
};

struct DisplayMode {
    int width = 0;
    int height = 0;
    int refreshRate = 0;

    [[nodiscard]] QString label() const;
};

struct DisplayOperationResult {
    bool succeeded = false;
    QString error;
};

class DisplayModeService {
public:
    DisplayModeService();
    ~DisplayModeService();

    DisplayModeService(const DisplayModeService &) = delete;
    DisplayModeService &operator=(const DisplayModeService &) = delete;

    [[nodiscard]] QVector<MonitorInfo> monitors() const;
    [[nodiscard]] std::optional<MonitorInfo> primaryMonitor() const;
    [[nodiscard]] std::optional<DisplayMode> currentMode(const QString &deviceName) const;
    [[nodiscard]] QVector<DisplayMode> modesFor(const QString &deviceName) const;
    [[nodiscard]] DisplayOperationResult captureCurrentMode(const QString &deviceName);
    [[nodiscard]] DisplayOperationResult applyMode(const QString &deviceName, const DisplayMode &mode);
    [[nodiscard]] DisplayOperationResult restoreMode();
    [[nodiscard]] bool hasSavedMode() const;

private:
    struct SavedDisplayMode;
    std::unique_ptr<SavedDisplayMode> savedMode;
};
