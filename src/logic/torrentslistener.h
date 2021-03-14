#pragma once

#include <memory>
#include <QObject>
#include <QReadWriteLock>

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include "utilities/singleton.h"
#include "treeitem.h"

#include <vector>

// status conversion
inline ItemDC::eSTATUSDC torrentStatus2ItemDCStatus(libtorrent::torrent_status::state_t state)
{
    switch (state)
    {
    case libtorrent::torrent_status::seeding:
        return ItemDC::eSEEDING;
    case libtorrent::torrent_status::finished:
        return ItemDC::eFINISHED;
    case libtorrent::torrent_status::downloading_metadata:
    case libtorrent::torrent_status::checking_files:
    case libtorrent::torrent_status::allocating:
    case libtorrent::torrent_status::checking_resume_data:
        return ItemDC::eSTARTING;
    case libtorrent::torrent_status::downloading:
        return ItemDC::eDOWNLOADING;
    default:
        return ItemDC::eUNKNOWN;
    }
}


#define ALERTS_OF_INTEREST \
    (save_resume_data_alert)\
    (storage_moved_alert)\
    (metadata_received_alert)\
    (file_error_alert)\
    (torrent_paused_alert)\
    (torrent_resumed_alert)\
    (torrent_removed_alert)\
    (stats_alert)\
    (state_changed_alert)\
    (session_stats_alert)

#if 0

(torrent_finished_alert)\
(save_resume_data_failed_alert)\
(file_renamed_alert)\
(torrent_deleted_alert)\
(storage_moved_failed_alert)\
(file_completed_alert)\
(tracker_error_alert)\
(tracker_reply_alert)\
(tracker_warning_alert)\
(portmap_error_alert)\
(portmap_alert)\
(peer_blocked_alert)\
(peer_ban_alert)\
(fastresume_rejected_alert)\
(url_seed_alert)\
(listen_succeeded_alert)\
(torrent_checked_alert)\
(add_torrent_alert)\
(block_downloading_alert)\
(block_timeout_alert)\

#endif

#define ALERT_TYPE_DEF(r, data, elem) struct elem;

namespace libtorrent
{

class alert;
class session;

BOOST_PP_SEQ_FOR_EACH(ALERT_TYPE_DEF, _, ALERTS_OF_INTEREST)
}

#undef ALERT_TYPE_DEF


class TorrentsListener : public QObject, public Singleton<TorrentsListener>
{
    Q_OBJECT

friend class Singleton<TorrentsListener>;

public:
    void setAlertDispatch(libtorrent::session* s);
    void setAt(ItemID id, const libtorrent::torrent_handle& handle);
    void setFileDialogEnabled(bool enabled);

    ItemID getItemID(const libtorrent::torrent_handle& h) const; // synchronized

    void handleItemMetadata(const libtorrent::torrent_handle& handle);

signals:
    void statusChange(const ItemDC& a_item);
    void sizeCurrDownlChange(const ItemDC& a_item);
    void sizeChange(const ItemDC& a_item);
    void speedChange(const ItemDC& a_item);
    void uploadSpeedChange(const ItemDC& a_item);
    void itemMetadataReceived(const ItemDC& item);
    void torrentMoved(const ItemDC& item);

    void signalTryNewtask();

    void sessionStats(long long unixTime, const std::vector<boost::uint64_t>& stats);

private:
    TorrentsListener(QObject* parent = 0);
    ~TorrentsListener();

    void alertDispatch(std::auto_ptr<libtorrent::alert> p);
    void onTorrentAdded(libtorrent::torrent_handle handl, void* userData);

    void saveTorrentFile(const libtorrent::torrent_handle& handle);
    void askOpentorrentUser(const libtorrent::torrent_handle& handle);

#define HANDLER_DEF(r, data, elem) void handler(const libtorrent::elem&);

    BOOST_PP_SEQ_FOR_EACH(HANDLER_DEF, _, ALERTS_OF_INTEREST)

#undef HANDLER_DEF

private:
    QMap<libtorrent::torrent_handle, int> m_handleToId;
    mutable QReadWriteLock m_handleMapWriteDataLock;
    bool m_askAboutFilesChoose;
};
