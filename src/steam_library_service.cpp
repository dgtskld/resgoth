#include "steam_library_service.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSettings>
#include <QSet>

namespace {
QString readTextFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QString valueForKey(const QString &text, const QString &key) {
    const QRegularExpression expression(QStringLiteral("\\\"%1\\\"\\s*\\\"([^\\\"]*)\\\"").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch match = expression.match(text);
    return match.hasMatch() ? match.captured(1).replace(QStringLiteral("\\\\"), QStringLiteral("\\")) : QString{};
}

QString steamInstallPath() {
    const QList<QPair<QString, QString>> registryValues{
        {QStringLiteral("HKEY_CURRENT_USER\\Software\\Valve\\Steam"), QStringLiteral("SteamPath")},
        {QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Valve\\Steam"), QStringLiteral("InstallPath")},
        {QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Valve\\Steam"), QStringLiteral("InstallPath")},
    };
    for (const auto &[registryPath, valueName] : registryValues) {
        QSettings settings(registryPath, QSettings::NativeFormat);
        const QString path = settings.value(valueName).toString();
        if (QDir(path).exists()) {
            return path;
        }
    }

    const QStringList defaultPaths{
        QStringLiteral("C:/Program Files (x86)/Steam"),
        QStringLiteral("C:/Program Files/Steam"),
    };
    for (const QString &path : defaultPaths) {
        if (QDir(path).exists()) {
            return path;
        }
    }
    return {};
}

QSet<QString> libraryPaths(const QString &steamPath) {
    QSet<QString> paths;
    if (!steamPath.isEmpty()) {
        paths.insert(QDir::cleanPath(steamPath));
    }

    const QString librariesFile = QDir(steamPath).filePath(QStringLiteral("steamapps/libraryfolders.vdf"));
    const QString librariesText = readTextFile(librariesFile);
    const QRegularExpression pathExpression(QStringLiteral("\\\"path\\\"\\s*\\\"([^\\\"]+)\\\""));
    QRegularExpressionMatchIterator iterator = pathExpression.globalMatch(librariesText);
    while (iterator.hasNext()) {
        paths.insert(QDir::cleanPath(iterator.next().captured(1).replace(QStringLiteral("\\\\"), QStringLiteral("\\"))));
    }
    return paths;
}
} // namespace

QVector<SteamGameInfo> SteamLibraryService::installedGames() const {
    QVector<SteamGameInfo> games;
    const QSet<QString> libraries = libraryPaths(steamInstallPath());
    for (const QString &library : libraries) {
        const QDir steamApps(QDir(library).filePath(QStringLiteral("steamapps")));
        const QFileInfoList manifests = steamApps.entryInfoList({QStringLiteral("appmanifest_*.acf")}, QDir::Files);
        for (const QFileInfo &manifest : manifests) {
            const QString manifestText = readTextFile(manifest.filePath());
            const QString appId = manifest.baseName().mid(QStringLiteral("appmanifest_").size());
            const QString name = valueForKey(manifestText, QStringLiteral("name"));
            const QString installDirectory = valueForKey(manifestText, QStringLiteral("installdir"));
            const QString fullInstallDirectory = steamApps.filePath(QStringLiteral("common/%1").arg(installDirectory));
            if (!name.isEmpty() && QDir(fullInstallDirectory).exists()) {
                games.append({appId, name, fullInstallDirectory});
            }
        }
    }
    std::sort(games.begin(), games.end(), [](const SteamGameInfo &left, const SteamGameInfo &right) {
        return left.name.localeAwareCompare(right.name) < 0;
    });
    return games;
}
