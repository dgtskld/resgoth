#include "app_config.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

QString AppConfigStore::filePath() const {
    return QDir(QCoreApplication::applicationDirPath()).filePath("resgoth.ini");
}

AppConfig AppConfigStore::read() const {
    QSettings settings(filePath(), QSettings::IniFormat);
    AppConfig config;
    config.gamePath = settings.value("game/path").toString();
    config.steamAppId = settings.value("steam/appId").toString();
    config.monitorDeviceName = settings.value("display/monitor").toString();
    config.width = settings.value("display/width", 0).toInt();
    config.height = settings.value("display/height", 0).toInt();
    config.refreshRate = settings.value("display/refreshRate", 0).toInt();
    return config;
}

void AppConfigStore::write(const AppConfig &config) const {
    QSettings settings(filePath(), QSettings::IniFormat);
    settings.setValue("game/path", config.gamePath);
    settings.setValue("steam/appId", config.steamAppId);
    settings.setValue("display/monitor", config.monitorDeviceName);
    settings.setValue("display/width", config.width);
    settings.setValue("display/height", config.height);
    settings.setValue("display/refreshRate", config.refreshRate);
    settings.sync();
}
