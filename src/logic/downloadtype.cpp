#include "downloadtype.h"

#include <QRegExp>

DownloadType::Type DownloadType::determineType(const QString& linkOrPath)
{
    Type res = Unknown;
    QRegExp torrentRx(".*\\.torrent$");
    QRegExp magnetRx("^magnet:");
    QRegExp webUrlRx("^https?://");
    QRegExp fileRx("^[a-z]:[\\\\/]|^/|^~/|^file:///", Qt::CaseInsensitive);

    // first priority of this check is presence of "http",
    // then check for ".torrent"
    // after that may check the rest...
    if (linkOrPath.contains(webUrlRx))
    {
        res = RemoteUrl;
    }
    else if (linkOrPath.contains(torrentRx))
    {
        res = TorrentFile;
    }
    else if (linkOrPath.contains(magnetRx))
    {
        res = MagnetLink;
    }
    else if (linkOrPath.contains(fileRx))
    {
        res = LocalFile;
    }

    return res;
}
