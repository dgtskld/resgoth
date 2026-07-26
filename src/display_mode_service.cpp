#include "display_mode_service.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <string>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace {
void writeDisplayLog(const QString &message) {
    const QString path = QDir(QCoreApplication::applicationDirPath()).filePath("resgoth.log");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString(Qt::ISODate) << " [display] " << message << '\n';
}

QString displayChangeError(const LONG result) {
    switch (result) {
    case DISP_CHANGE_BADDUALVIEW:
        return QStringLiteral("DualView does not support this change");
    case DISP_CHANGE_BADFLAGS:
        return QStringLiteral("Win32 API received unsupported flags");
    case DISP_CHANGE_BADMODE:
        return QStringLiteral("this display mode is not supported by the monitor");
    case DISP_CHANGE_BADPARAM:
        return QStringLiteral("Win32 API received invalid parameters");
    case DISP_CHANGE_FAILED:
        return QStringLiteral("the display driver rejected the mode change");
    case DISP_CHANGE_NOTUPDATED:
        return QStringLiteral("Windows could not update the user settings");
    case DISP_CHANGE_RESTART:
        return QStringLiteral("Windows must be restarted before the mode can change");
    default:
        return QStringLiteral("unknown Win32 error (%1)").arg(result);
    }
}

DisplayOperationResult failed(const QString &message) {
    writeDisplayLog(message);
    return {false, message};
}

bool isTestableMode(const std::wstring &deviceName, const DEVMODEW &mode) {
    DEVMODEW testMode = mode;
    return ChangeDisplaySettingsExW(deviceName.c_str(), &testMode, nullptr, CDS_TEST, nullptr) == DISP_CHANGE_SUCCESSFUL;
}
} // namespace

struct DisplayModeService::SavedDisplayMode {
    QString deviceName;
    DEVMODEW mode{};
};

DisplayModeService::DisplayModeService() = default;
DisplayModeService::~DisplayModeService() = default;

QString DisplayMode::label() const {
    const std::array<std::pair<int, int>, 7> commonRatios{{{16, 9}, {16, 10}, {4, 3}, {5, 4}, {3, 2}, {21, 9}, {32, 9}}};
    const double ratio = height > 0 ? static_cast<double>(width) / height : 0.0;
    QString aspectRatio;
    for (const auto &[ratioWidth, ratioHeight] : commonRatios) {
        if (std::abs(ratio - static_cast<double>(ratioWidth) / ratioHeight) < 0.005) {
            aspectRatio = QStringLiteral("%1:%2").arg(ratioWidth).arg(ratioHeight);
            break;
        }
    }
    if (aspectRatio.isEmpty()) {
        const int divisor = std::gcd(width, height);
        aspectRatio = divisor > 0 ? QStringLiteral("%1:%2").arg(width / divisor).arg(height / divisor)
                                  : QStringLiteral("unknown");
    }
    if (refreshRate > 1) {
        return QStringLiteral("%1 × %2, %3 Hz (%4)").arg(width).arg(height).arg(refreshRate).arg(aspectRatio);
    }
    return QStringLiteral("%1 × %2 (%3)").arg(width).arg(height).arg(aspectRatio);
}

QVector<MonitorInfo> DisplayModeService::monitors() const {
    QVector<MonitorInfo> result;

    for (DWORD adapterIndex = 0;; ++adapterIndex) {
        DISPLAY_DEVICEW adapter{};
        adapter.cb = sizeof(adapter);
        if (!EnumDisplayDevicesW(nullptr, adapterIndex, &adapter, 0)) {
            break;
        }
        if ((adapter.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) == 0 ||
            (adapter.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) != 0) {
            continue;
        }

        for (DWORD monitorIndex = 0;; ++monitorIndex) {
            DISPLAY_DEVICEW monitor{};
            monitor.cb = sizeof(monitor);
            if (!EnumDisplayDevicesW(adapter.DeviceName, monitorIndex, &monitor, 0)) {
                break;
            }
            if ((monitor.StateFlags & DISPLAY_DEVICE_ACTIVE) == 0) {
                continue;
            }

            const QString monitorName = QString::fromWCharArray(monitor.DeviceString);
            const QString adapterName = QString::fromWCharArray(adapter.DeviceString);
            result.append({QString::fromWCharArray(adapter.DeviceName),
                           QStringLiteral("%1 (%2)").arg(monitorName, adapterName)});
        }
    }
    return result;
}

QVector<DisplayMode> DisplayModeService::modesFor(const QString &deviceName) const {
    QVector<DisplayMode> result;
    const std::wstring nativeName = deviceName.toStdWString();

    for (DWORD index = 0;; ++index) {
        DEVMODEW mode{};
        mode.dmSize = sizeof(mode);
        if (!EnumDisplaySettingsW(nativeName.c_str(), index, &mode)) {
            break;
        }
        if (!isTestableMode(nativeName, mode)) {
            continue;
        }

        const DisplayMode candidate{static_cast<int>(mode.dmPelsWidth),
                                    static_cast<int>(mode.dmPelsHeight),
                                    static_cast<int>(mode.dmDisplayFrequency)};
        const bool exists = std::any_of(result.cbegin(), result.cend(), [&candidate](const DisplayMode &item) {
            return item.width == candidate.width && item.height == candidate.height && item.refreshRate == candidate.refreshRate;
        });
        if (!exists) {
            result.append(candidate);
        }
    }

    std::sort(result.begin(), result.end(), [](const DisplayMode &left, const DisplayMode &right) {
        const int leftPixels = left.width * left.height;
        const int rightPixels = right.width * right.height;
        return leftPixels == rightPixels ? left.refreshRate < right.refreshRate : leftPixels < rightPixels;
    });
    return result;
}

std::optional<MonitorInfo> DisplayModeService::primaryMonitor() const {
    for (DWORD adapterIndex = 0;; ++adapterIndex) {
        DISPLAY_DEVICEW adapter{};
        adapter.cb = sizeof(adapter);
        if (!EnumDisplayDevicesW(nullptr, adapterIndex, &adapter, 0)) {
            break;
        }
        if ((adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) == 0 ||
            (adapter.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) == 0) {
            continue;
        }

        for (DWORD monitorIndex = 0;; ++monitorIndex) {
            DISPLAY_DEVICEW monitor{};
            monitor.cb = sizeof(monitor);
            if (!EnumDisplayDevicesW(adapter.DeviceName, monitorIndex, &monitor, 0)) {
                break;
            }
            if ((monitor.StateFlags & DISPLAY_DEVICE_ACTIVE) != 0) {
                return MonitorInfo{QString::fromWCharArray(adapter.DeviceName),
                                   QString::fromWCharArray(monitor.DeviceString)};
            }
        }
        return MonitorInfo{QString::fromWCharArray(adapter.DeviceName), QStringLiteral("Primary display")};
    }
    return std::nullopt;
}

std::optional<DisplayMode> DisplayModeService::currentMode(const QString &deviceName) const {
    if (deviceName.isEmpty()) {
        return std::nullopt;
    }

    const std::wstring nativeName = deviceName.toStdWString();
    DEVMODEW mode{};
    mode.dmSize = sizeof(DEVMODEW);
    if (!EnumDisplaySettingsW(nativeName.c_str(), ENUM_CURRENT_SETTINGS, &mode)) {
        return std::nullopt;
    }
    return DisplayMode{static_cast<int>(mode.dmPelsWidth),
                       static_cast<int>(mode.dmPelsHeight),
                       static_cast<int>(mode.dmDisplayFrequency)};
}

DisplayOperationResult DisplayModeService::captureCurrentMode(const QString &deviceName) {
    if (deviceName.isEmpty()) {
        return failed(QStringLiteral("Cannot save the display mode: no monitor is selected."));
    }
    if (savedMode && savedMode->deviceName != deviceName) {
        return failed(QStringLiteral("Cannot change monitors while another monitor's mode is saved."));
    }
    if (savedMode) {
        return {true, {}};
    }

    const std::wstring nativeName = deviceName.toStdWString();
    auto saved = std::make_unique<SavedDisplayMode>();
    saved->deviceName = deviceName;
    saved->mode.dmSize = sizeof(DEVMODEW);
    if (!EnumDisplaySettingsW(nativeName.c_str(), ENUM_CURRENT_SETTINGS, &saved->mode)) {
        return failed(QStringLiteral("Could not read the original display mode for %1 (Win32 error %2).")
                          .arg(deviceName)
                          .arg(GetLastError()));
    }

    writeDisplayLog(QStringLiteral("Saved original mode for %1: %2 × %3, %4 Hz.")
                        .arg(deviceName)
                        .arg(saved->mode.dmPelsWidth)
                        .arg(saved->mode.dmPelsHeight)
                        .arg(saved->mode.dmDisplayFrequency));
    savedMode = std::move(saved);
    return {true, {}};
}

DisplayOperationResult DisplayModeService::applyMode(const QString &deviceName, const DisplayMode &mode) {
    if (mode.width <= 0 || mode.height <= 0) {
        return failed(QStringLiteral("Cannot apply a mode with an invalid resolution."));
    }
    const DisplayOperationResult captureResult = captureCurrentMode(deviceName);
    if (!captureResult.succeeded) {
        return captureResult;
    }

    DEVMODEW requested{};
    requested.dmSize = sizeof(DEVMODEW);
    requested.dmPelsWidth = static_cast<DWORD>(mode.width);
    requested.dmPelsHeight = static_cast<DWORD>(mode.height);
    requested.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    if (mode.refreshRate > 1) {
        requested.dmDisplayFrequency = static_cast<DWORD>(mode.refreshRate);
        requested.dmFields |= DM_DISPLAYFREQUENCY;
    }

    const std::wstring nativeName = deviceName.toStdWString();
    const LONG testResult = ChangeDisplaySettingsExW(nativeName.c_str(), &requested, nullptr, CDS_TEST, nullptr);
    if (testResult != DISP_CHANGE_SUCCESSFUL) {
        return failed(QStringLiteral("Mode test for %1 failed: %2.")
                          .arg(mode.label(), displayChangeError(testResult)));
    }

    const LONG applyResult = ChangeDisplaySettingsExW(nativeName.c_str(), &requested, nullptr, 0, nullptr);
    if (applyResult != DISP_CHANGE_SUCCESSFUL) {
        return failed(QStringLiteral("Could not apply mode %1: %2.")
                          .arg(mode.label(), displayChangeError(applyResult)));
    }

    writeDisplayLog(QStringLiteral("Applied mode %1 to %2.").arg(mode.label(), deviceName));
    return {true, {}};
}

DisplayOperationResult DisplayModeService::restoreMode() {
    if (!savedMode) {
        return {true, {}};
    }

    const std::wstring nativeName = savedMode->deviceName.toStdWString();
    const LONG restoreResult = ChangeDisplaySettingsExW(nativeName.c_str(), &savedMode->mode, nullptr, 0, nullptr);
    if (restoreResult != DISP_CHANGE_SUCCESSFUL) {
        return failed(QStringLiteral("Could not restore the original display mode: %1.")
                          .arg(displayChangeError(restoreResult)));
    }

    writeDisplayLog(QStringLiteral("Restored original mode for %1.").arg(savedMode->deviceName));
    savedMode.reset();
    return {true, {}};
}

bool DisplayModeService::hasSavedMode() const {
    return static_cast<bool>(savedMode);
}
