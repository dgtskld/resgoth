#pragma once

#include <QString>
#include <QVector>

struct SteamGameInfo {
    QString appId;
    QString name;
    QString installDirectory;
};

class SteamLibraryService {
public:
    [[nodiscard]] QVector<SteamGameInfo> installedGames() const;
};
