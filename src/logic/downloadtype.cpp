#include "downloadtype.h"

#include <QRegExp>

DownloadType::Type DownloadType::determineType(const QString& linkOrPath)
{
    const QRegExp torrentRx(".*\\.torrent$");
    const QRegExp magnetRx("^magnet:");
    const QRegExp webUrlRx("^https?://");
    const QRegExp fileRx("^[a-z]:[\\\\/]|^/|^~/|^file://", Qt::CaseInsensitive);

    // first priority of this check is presence of "http",
    // then check for ".torrent"
    // after that may check the rest...
    if (linkOrPath.contains(webUrlRx))
    {
        return RemoteUrl;
    }
    if (linkOrPath.contains(torrentRx))
    {
        return TorrentFile;
    }
    if (linkOrPath.contains(magnetRx))
    {
        return MagnetLink;
    }
    if (linkOrPath.contains(fileRx))
    {
        return LocalFile;
    }

    return Unknown;
}
