#pragma once

#include <QString>
#include <QObject>
#include <QMetaType>

class DownloadType
    : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)
public:
    enum Type
    {
        Unknown,
        RemoteUrl,
        TorrentFile,
        MagnetLink,
        LocalFile
    };

    static bool isDirectDownload(Type type)
    {
        return type != TorrentFile && type != MagnetLink;
    }
    static bool isTorrentDownload(Type type)
    {
        return type == TorrentFile || type == MagnetLink;
    }

    static Type determineType(const QString& linkOrPath);
};

Q_DECLARE_METATYPE(DownloadType::Type);
