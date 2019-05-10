#include "torrentslistener.h"
#include "torrentmanager.h"

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent.hpp>
#include <libtorrent/extensions.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/announce_entry.hpp>

#include "utilities/utils.h"
#include "utilities/notify_helper.h"
#include "addtorrentform.h"
#include "mainwindow.h"

#include <QMetaType>
#include <QThread>
#include <QApplication>
#include <QDebug>
#include <QPointer>

#include <functional>
#include <utility>

namespace {

class AddTorrentFormHelper : public NotifyHelper
{
public:
    explicit AddTorrentFormHelper(
        TorrentsListener* listener, const libtorrent::torrent_handle& handle)
        : m_listener(listener), m_handle(handle)
    {
        moveToThread(QApplication::instance()->thread());
    }

    void slotNoParams() override
    {
        auto* addDialog = new AddTorrentForm(m_handle, utilities::getMainWindow());

        const int status = addDialog->exec();
        if (!m_listener.isNull())
        {
            if (status == QDialog::Rejected) // main thread
            {
                DownloadCollectionModel::instance().deleteURLFromModel(
                    m_listener->getItemID(m_handle), libtorrent::session::delete_files);
            }
            else
            {
                m_listener->handleItemMetadata(m_handle);
                DownloadCollectionModel::instance().setTorrentFilesPriorities(
                    m_listener->getItemID(m_handle), addDialog->filesPriorities());
            }
        }

        addDialog->deleteLater();

        deleteLater();
    }

private:
    QPointer<TorrentsListener> m_listener;
    libtorrent::torrent_handle m_handle;
};


class TorrentsListenerExtension : public libtorrent::plugin
{
public:
    TorrentsListenerExtension(std::function<void(libtorrent::torrent_handle, void*)> callback) : m_callback(std::move(callback)) {}

    boost::shared_ptr<libtorrent::torrent_plugin> new_torrent(libtorrent::torrent_handle const& h, void* user) override
    {
        m_callback(h, user);
        return boost::shared_ptr<libtorrent::torrent_plugin>();
    }
private:
    std::function<void(libtorrent::torrent_handle, void*)> m_callback;
};

} // namespace


TorrentsListener::TorrentsListener(QObject* parent /* = 0 */)
    : QObject(parent)
    , m_askAboutFilesChoose(false)
{
}

TorrentsListener::~TorrentsListener()
{
}


#define ALERT_MASK_DEF(r, data, elem) | libtorrent::elem::static_category

void TorrentsListener::setAlertDispatch(libtorrent::session* s)
{
    using namespace std::placeholders;

    const boost::uint32_t alertMask
        = 0
          BOOST_PP_SEQ_FOR_EACH(ALERT_MASK_DEF, _, ALERTS_OF_INTEREST)
          ;

    static_assert(alertMask != 0, "alertMask should not be equal to zero");

    s->set_alert_mask(alertMask);

    s->set_alert_dispatch(std::bind(&TorrentsListener::alertDispatch, this, _1));
    s->add_extension(boost::shared_ptr<libtorrent::plugin>(new TorrentsListenerExtension(
        std::bind(&TorrentsListener::onTorrentAdded, this, _1, _2))));
}

#undef ALERT_MASK_DEF


#define CASE_DEF(r, data, elem) \
    if (typeid_ == typeid(libtorrent::elem)) \
        handler(*static_cast<libtorrent::elem*>(p.get())); \
    else


void TorrentsListener::alertDispatch(std::auto_ptr<libtorrent::alert> p)
{
    const std::type_info& typeid_ = typeid(*p);
    BOOST_PP_SEQ_FOR_EACH(CASE_DEF, _, ALERTS_OF_INTEREST)
    ;
}

#undef CASE_DEF

///////////////////////////////////////////////////////////////////////////////

#ifdef DEVELOPER_TORRENTS_LOGS
#define TRACE_ALERT qDebug() << typeid(a).name() << a.message().c_str();
#else
#define TRACE_ALERT
#endif

void TorrentsListener::handler(libtorrent::stats_alert const& a)
{
    //TRACE_ALERT
    libtorrent::torrent_status status = a.handle.status(libtorrent::torrent_handle::query_accurate_download_counters);

    if (status.state == libtorrent::torrent_status::downloading)
    {
        ItemDC item;
        item.setID(getItemID(a.handle));
        item.setSize(status.total_wanted);
        item.setSizeCurrDownl(status.total_wanted_done);

        float downloadSpeed = status.download_payload_rate / 1024.0;
        item.setSpeed(downloadSpeed);
        float uploadSpeed = status.upload_payload_rate / 1024.0;
        item.setSpeedUpload(uploadSpeed);
        item.setStatus(downloadSpeed > 0 ? ItemDC::eDOWNLOADING : ItemDC::eSTALLED);
        emit statusChange(item);
        emit speedChange(item);
        emit sizeCurrDownlChange(item);

        emit signalTryNewtask(); // TODO fine tune
    }
    else if (status.state == libtorrent::torrent_status::seeding
        || status.state == libtorrent::torrent_status::finished)
    {
        float uploadSpeed = status.upload_payload_rate / 1024.0;
        ItemDC item;
        item.setID(getItemID(a.handle));
        item.setSpeedUpload(uploadSpeed);
        emit speedChange(item);
    }
}

void TorrentsListener::handler(libtorrent::torrent_removed_alert const& a)
{
    TRACE_ALERT
    QWriteLocker locker(&m_handleMapWriteDataLock);
    m_handleToId.remove(a.handle);
}

void TorrentsListener::handler(libtorrent::torrent_paused_alert const& a)
{
    TRACE_ALERT
    ItemDC item;
    item.setID(getItemID(a.handle));
    const auto newStatus 
        = (a.handle.is_seed() || a.handle.is_finished()) ? ItemDC::eFINISHED : ItemDC::ePAUSED;
    item.setStatus(newStatus);
    emit speedChange(item);
    emit uploadSpeedChange(item);
    emit statusChange(item);
}

void TorrentsListener::handler(libtorrent::torrent_resumed_alert const& a)
{
    TRACE_ALERT
    if (a.handle.is_seed() || a.handle.is_finished())
    {
        ItemDC item;
        item.setStatus(ItemDC::eSEEDING);
        item.setID(getItemID(a.handle));
        emit statusChange(item);
    }
}

void TorrentsListener::handler(libtorrent::file_error_alert const& a)
{
    TRACE_ALERT
    ItemDC item;
    item.setID(getItemID(a.handle));
    item.setStatus(ItemDC::eERROR);
    item.setErrorDescription(QString::fromLocal8Bit(a.error.message().c_str()));
    emit statusChange(item);
}

void TorrentsListener::handler(libtorrent::metadata_received_alert const& a)
{
    TRACE_ALERT
    libtorrent::torrent_handle handle = a.handle;

    if (m_askAboutFilesChoose)
    {
        askOpentorrentUser(handle);
    }
    else
    {
        handleItemMetadata(handle);
    }
}

void TorrentsListener::handler(libtorrent::storage_moved_alert const& a)
{
    TRACE_ALERT
    ItemDC item;
    item.setID(getItemID(a.handle));
    item.setTorrentSavePath(QString::fromStdString(a.handle.save_path()));
    emit torrentMoved(item);
}

void TorrentsListener::handler(libtorrent::save_resume_data_alert const& a)
{
    TRACE_ALERT
    if (!a.resume_data)
    {
        return;
    }

    if (!a.handle.is_valid())
    {
        return;
    }

    try
    {
        std::vector<char> out;
        libtorrent::bencode(std::back_inserter(out), *a.resume_data);
        const QString filepath = toQString(a.handle.info_hash()) + ".fastresume";
        QFile resume_file(utilities::PrepareCacheFolder(TORRENTS_SUB_FOLDER) + filepath);
        if (!out.empty() && resume_file.open(QIODevice::WriteOnly))
        {
            resume_file.write(&out[0], out.size());
            resume_file.close();
            qDebug() << "Fast resume data successfully saved";
        }
    }
    catch (libtorrent::libtorrent_exception const& e)
    {
        qDebug() << Q_FUNC_INFO << " caught " << e.what();
    }
}

void TorrentsListener::handler(libtorrent::state_changed_alert const& a)
{
    TRACE_ALERT
    ItemDC item;
    item.setID(getItemID(a.handle));

    switch (a.state)
    {
    case libtorrent::torrent_status::seeding:
        if (a.handle.status(0x0).paused)
        {
            return;
        }
        item.setStatus(ItemDC::eSEEDING);
        break;
    case libtorrent::torrent_status::finished:
        {
            item.setStatus(ItemDC::eFINISHED);
            item.setSpeed(0);
            libtorrent::torrent_status status = a.handle.status(libtorrent::torrent_handle::query_accurate_download_counters);
            item.setSize(status.total_wanted);
            item.setSizeCurrDownl(status.total_wanted_done);
            item.setDownloadType(DownloadType::TorrentFile);
            emit sizeCurrDownlChange(item);
            emit speedChange(item);
        }
        break;
    default:
        return;
    }

    emit statusChange(item);

    emit signalTryNewtask(); // TODO fine tune
}


void TorrentsListener::onTorrentAdded(libtorrent::torrent_handle handle, void* userData)
{
    const int id = (int)(intptr_t)userData;
    setAt(id, handle);
}

ItemID TorrentsListener::getItemID(const libtorrent::torrent_handle& h) const
{
    QReadLocker locker(&m_handleMapWriteDataLock);
    auto it = m_handleToId.find(h);
    ItemID result = (it != m_handleToId.end()) ? it.value() : nullItemID;
    return result;
}

void TorrentsListener::setAt(ItemID id, const libtorrent::torrent_handle& handle)
{
    QWriteLocker locker(&m_handleMapWriteDataLock);
    m_handleToId[handle] = id;
}


void TorrentsListener::saveTorrentFile(const libtorrent::torrent_handle& handle)
{
    if (handle.is_valid())
    {
        QString fileName = utilities::PrepareCacheFolder(TORRENTS_SUB_FOLDER) + toQString(handle.info_hash()) + ".torrent";
        const libtorrent::torrent_info& t = handle.get_torrent_info();
        libtorrent::entry meta = libtorrent::bdecode(t.metadata().get(),
                                 t.metadata().get() + t.metadata_size());
        libtorrent::entry torrent_entry(libtorrent::entry::dictionary_t);
        torrent_entry["info"] = meta;
        if (!handle.trackers().empty())
        {
            torrent_entry["announce"] = handle.trackers().front().url;
        }

        std::vector<char> out;
        bencode(back_inserter(out), torrent_entry);
        QFile torrent_file(fileName);
        if (!out.empty() && torrent_file.open(QIODevice::WriteOnly))
        {
            torrent_file.write(&out[0], out.size());
            torrent_file.close();
        }
    }
}

void TorrentsListener::setFileDialogEnabled(bool enabled)
{
    m_askAboutFilesChoose = enabled;
}

void TorrentsListener::askOpentorrentUser(const libtorrent::torrent_handle& handle)
{
    auto* helper = new AddTorrentFormHelper(this, handle);
    QMetaObject::invokeMethod(helper, "slotNoParams", Qt::QueuedConnection);
}


void TorrentsListener::handleItemMetadata(const libtorrent::torrent_handle& handle)
{
    try
    {
        libtorrent::torrent_status status = handle.status(0x0);
        ItemDC item;
        item.setID(getItemID(handle));
        item.setSize(status.total_wanted);
        item.setDownloadedFileName(QString::fromStdString(handle.get_torrent_info().name()));
        item.setSource("Torrent");

        // Save Path
        item.setTorrentSavePath(QString::fromStdString(handle.save_path()));

        emit itemMetadataReceived(item);

        saveTorrentFile(handle);
    }
    catch (libtorrent::libtorrent_exception const& e)
    {
        qDebug() << __FUNCTION__ << "exception: " << e.what();
    }
}
