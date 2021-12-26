#pragma once

#include <memory>
#include <QObject>
#include <QReadWriteLock>

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

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


#define ALERTS_OF_INTEREST(macro) \
    macro(save_resume_data_alert)\
    macro(storage_moved_alert)\
    macro(metadata_received_alert)\
    macro(file_error_alert)\
    macro(torrent_paused_alert)\
    macro(torrent_resumed_alert)\
    macro(torrent_removed_alert)\
    macro(stats_alert)\
    macro(state_changed_alert)\
    macro(session_stats_alert)


#define ALERT_TYPE_DEF(elem) struct elem;

namespace libtorrent
{

class alert;
class session;

ALERTS_OF_INTEREST(ALERT_TYPE_DEF)
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

#define HANDLER_DEF(elem) void handler(const libtorrent::elem&);

    ALERTS_OF_INTEREST(HANDLER_DEF)

#undef HANDLER_DEF

private:
    QMap<libtorrent::torrent_handle, int> m_handleToId;
    mutable QReadWriteLock m_handleMapWriteDataLock;
    bool m_askAboutFilesChoose;
};
