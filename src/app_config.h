#pragma once

#include <QString>

struct AppConfig {
    QString gamePath;
    QString steamAppId;
    QString monitorDeviceName;
    int width = 0;
    int height = 0;
    int refreshRate = 0;
};

class AppConfigStore {
public:
    AppConfig read() const;
    void write(const AppConfig &config) const;

    [[nodiscard]] QString filePath() const;
};
