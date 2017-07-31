#pragma once

#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <QString>
#include <QObject>
#include <QMap>
#include <QTimer>

#include <memory>

#include "downloadtype.h"
#include "treeitem.h"


static const char TORRENTS_SUB_FOLDER[] = "torrents";

QString toQString(const libtorrent::sha1_hash& hash);

class TorrentManager : public QObject
{
    Q_OBJECT
public:
    ~TorrentManager();

    static TorrentManager* Instance();
    static void dispose();
    static bool isSessionExists();

    void close();

    void setListeningPort(int firstPosrt, int lastPort);
    int port() const;

    libtorrent::torrent_handle torrentByModelId(int id);
    libtorrent::torrent_handle addTorrent(
        const QString& torrOrMagnet, 
        int id, 
        bool interactive = false, 
        const QString& savePath = "",
        const std::vector<boost::uint8_t>* file_priorities = nullptr);
    bool resumeTorrent(int id);
    bool restartTorrent(int id);

    QString torrentRootItemPath(int itemId);

    void setUploadLimit(int limit);
    void setDownloadLimit(int limit);

public Q_SLOTS:
    void on_deleteTaskWithID(int a_id, DownloadType::Type type, int deleteWithFiles);
    void on_pauseTaskWithID(int a_id, DownloadType::Type type);

private Q_SLOTS:
    int cacheResumeTorrentsData(bool fully_data_save = false);

private:
    explicit TorrentManager();
    TorrentManager(const TorrentManager&) = delete;
    TorrentManager& operator =(const TorrentManager&) = delete;

    static std::unique_ptr<TorrentManager> m_instance;

    QMap<int, libtorrent::torrent_handle> m_idToHandle;
    QTimer m_resumeDataTimer;

    bool m_closed;

    std::unique_ptr<libtorrent::session> m_session;
};
