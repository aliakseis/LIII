#include "torrentmanager.h"

#include <string>
#include <climits>
#include <queue>
#include <vector>
#include <utility>
#include <algorithm>
#include <functional>
#include <stdio.h>
#include <boost/function_output_iterator.hpp>
#include <libtorrent/version.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/file.hpp>
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/lazy_entry.hpp>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QByteArray>
#include <QFile>
#include <QMessageBox>

#include "settings_declaration.h"
#include "application.h"

#include "addtorrentform.h"
#include "torrentslistener.h"
#include "downloadtype.h"

#include "mainwindow.h"
#include "settings_declaration.h"
#include "utilities/translation.h"
#include "version.hxx"
#include "branding.hxx"

#include "global_functions.h"

namespace {

libtorrent::fingerprint getFingerprint()
{
    int version[4] {};
    sscanf(PROJECT_VERSION, "%d.%d.%d.%d", version, version + 1, version + 2, version + 3);
    const char torrentClientId[] = "53";
    return { torrentClientId, version[0], version[1], version[2], version[3] };
}

bool loadFastResumeData(const QString& hash, std::vector<char>& buf)
{
    const QString fastresume_path = utilities::PrepareCacheFolder(TORRENTS_SUB_FOLDER) + hash + ".fastresume";

    qDebug() << "Trying to load fastresume data: " << fastresume_path;

    QFile fastresume_file(fastresume_path);
    if (fastresume_file.size() <= 0 || !fastresume_file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open a file " << fastresume_path;
        return false;
    }

    const QByteArray content = fastresume_file.readAll();

    const int content_size = content.size();
    Q_ASSERT(content_size > 0);

    buf.reserve(content_size);
    std::copy(content.constBegin(), content.constEnd(), std::back_inserter(buf));

    fastresume_file.close();
    return content_size > 0;
}

libtorrent::sha1_hash hashFromQString(const QString& hash)
{
    libtorrent::sha1_hash hex;
    libtorrent::from_hex(hash.toLatin1().constData(), 40, (char*)&hex);
    return hex;
}

QString torrentRootItemPath(const libtorrent::torrent_handle& handle)
{
    QString result;
    Q_ASSERT(handle.is_valid());
    if (handle.is_valid())
    {
        libtorrent::torrent_info torrInfo = handle.get_torrent_info();

        if (torrInfo.num_files() > 0)
        {
            const std::string& firstFile = torrInfo.file_at(0).path;
            auto end = firstFile.cend();
            for (int i = 1; i < torrInfo.num_files(); ++i)
            {
                const libtorrent::file_entry& fentry = torrInfo.file_at(i);
                auto loc = std::mismatch(firstFile.cbegin(), end, fentry.path.cbegin(), fentry.path.cend());
                end = loc.first;
            }

            auto minLength = std::distance(firstFile.cbegin(), end);

            auto lastSlash = firstFile.find_last_of("/\\", minLength);
            if (lastSlash != std::string::npos && lastSlash > 0)
            {
                minLength = lastSlash;
            }

            auto lhs = handle.save_path();
            if (!lhs.empty() && lhs[lhs.size() - 1] != '\\' && lhs[lhs.size() - 1] != '/')
                lhs += QDir::separator().toLatin1();
            result = QString::fromStdString(lhs + firstFile.substr(0, minLength));
        }
    }
    return result;
}

QString btihFromMaget(const QString& magnet)
{
    QRegExp btihRx("magnet:(?:.*)(?:\\?|&)xt=urn:btih:([a-z0-9]+)[&\\n\\r]?", Qt::CaseInsensitive);
    if (btihRx.indexIn(magnet) != -1)
    {
        QString btih = btihRx.cap(1);
        return btih.size() == 40 ? btih : QString();
    }

    return QString();
}

std::vector<libtorrent::announce_entry> parseTrackersList(const QString& torrOrMagnet)
{
    std::vector<libtorrent::announce_entry> trackers_new;
    QRegExp trackersRx("&tr(?:.[0-9]+)?=([^&\\n\\r]+)", Qt::CaseInsensitive);
    int pos = 0;
    while ((pos = trackersRx.indexIn(torrOrMagnet, pos)) != -1)
    {
        trackers_new.push_back(libtorrent::announce_entry(trackersRx.cap(1).toUtf8().constData()));
        pos += trackersRx.matchedLength();
    }
    return trackers_new;
}

auto makeAddTrackerIterator(libtorrent::torrent_handle& h)
{
    return boost::make_function_output_iterator(
        std::bind(&libtorrent::torrent_handle::add_tracker, &h, std::placeholders::_1));
}

bool mergeTrackers()
{
    QMessageBox msgBox(
        QMessageBox::NoIcon,
        utilities::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
        utilities::Tr::Tr(DUPLICATE_TORRENT_TEXT),
        QMessageBox::Yes | QMessageBox::No,
        utilities::getMainWindow());
    msgBox.setDefaultButton(QMessageBox::No);

    return msgBox.exec() == QMessageBox::Yes;
}

}  // namespace

QString toQString(const libtorrent::sha1_hash& hash)
{
    char out[41];
    libtorrent::to_hex((char const*)&hash[0], libtorrent::sha1_hash::size, out);
    return QString(out);
}

using namespace app_settings;

std::unique_ptr<TorrentManager> TorrentManager::m_instance;

TorrentManager* TorrentManager::Instance()
{
    if (!m_instance)
    {
        m_instance.reset(new TorrentManager());
    }

    return m_instance.get();
}

void TorrentManager::dispose()
{
    m_instance.reset();
}

bool TorrentManager::isSessionExists()
{
    return m_instance && m_instance->m_session != nullptr;
}

TorrentManager::TorrentManager()
    : m_closed(false)
    , m_session(std::make_unique<libtorrent::session>(getFingerprint()))
{
    m_session->set_settings(libtorrent::session_settings(PROJECT_NAME " " PROJECT_VERSION
#if defined(Q_OS_DARWIN)
        " Mac"
#elif defined(Q_OS_UNIX)
        " Linux"
#endif
    ));

    DownloadCollectionModel* dlcModel = &DownloadCollectionModel::instance();

    QByteArray in = QByteArray::fromBase64(dlcModel->getTorrentSessionState().toLatin1());
    // bdecode
    libtorrent::lazy_entry e;
    libtorrent::error_code ec;
    lazy_bdecode(in.constData(), in.constData() + in.size(), e, ec);
    if (!ec)
    {
        m_session->load_state(e);
    }

    TorrentsListener::instance().setAlertDispatch(m_session.get());

    VERIFY(connect(dlcModel, SIGNAL(signalDeleteURLFromModel(int, DownloadType::Type, int)), this, SLOT(on_deleteTaskWithID(int, DownloadType::Type, int))));
    VERIFY(connect(dlcModel, SIGNAL(signalPauseDownloadItemWithID(int, DownloadType::Type)), this, SLOT(on_pauseTaskWithID(int, DownloadType::Type))));

    if (!QSettings().value(TorrentsPortIsAuto, TorrentsPortIsAuto_Default).toBool())
    {
        int port = QSettings().value(TorrentsPort, TorrentsPort_Default).toInt();
        Q_ASSERT(port > 0 && port <= 65535);
        setListeningPort(port, port);
    }
    else
    {
        setListeningPort(6881, 6889);
    }


    // Speed control
    setUploadLimit(QSettings().value(IsTrafficUploadLimited, IsTrafficUploadLimited_Default).toBool()
        ? QSettings().value(TrafficUploadLimitKbs, TrafficUploadLimitKbs_Default).toInt() * 1024 : 0);

    setDownloadLimit(QSettings().value(IsTrafficLimited, IsTrafficLimited_Default).toBool()
        ? QSettings().value(TrafficLimitKbs, TrafficLimitKbs_Default).toInt() * 1024 : 0);

    // enable dht by default for magnets
    Q_ASSERT(m_session->is_dht_running());

    for (auto node : {
        "router.bittorrent.com",
        "router.utorrent.com",
        "dht.transmissionbt.com",
        "dht.aelitis.com"
    })
    {
        m_session->add_dht_router(std::make_pair(std::string(node), 6881));
    }

    // Regular saving of fastresume data
    VERIFY(connect(&m_resumeDataTimer, SIGNAL(timeout()), this, SLOT(cacheResumeTorrentsData())));
    m_resumeDataTimer.setSingleShot(false);
    m_resumeDataTimer.start(200000); // 3.3min
}

TorrentManager::~TorrentManager()
{
    close();
}

void TorrentManager::close()
{
    if (m_closed)
    {
        return;
    }

    m_closed = true;

    DownloadCollectionModel* dlcModel = &DownloadCollectionModel::instance();
    libtorrent::entry session_state;
    m_session->save_state(session_state);
    QByteArray out;
    bencode(std::back_inserter(out), session_state);
    dlcModel->setTorrentSessionState(out.toBase64());

    //
    m_resumeDataTimer.stop();

    // Avoid setting model items' states to paused
    TorrentsListener::instance().disconnect(dlcModel);
    // Pause session
    m_session->pause();

    cacheResumeTorrentsData(true);
}

int TorrentManager::cacheResumeTorrentsData(bool fully_data_save /* = false */)
{
    int cached = 0;

    for (const auto& torrent : m_session->get_torrents())
    {
        try
        {
            const libtorrent::torrent_status status = torrent.status();

            if (!torrent.is_valid() || !status.has_metadata)
            {
                continue;
            }

            // Skipping datasave only in party-saving mode
            if (!fully_data_save && !torrent.need_save_resume_data())
            {
                continue;
            }

            const libtorrent::torrent_status::state_t state = status.state;
            if (state == libtorrent::torrent_status::checking_files || state == libtorrent::torrent_status::queued_for_checking)
            {
                continue;
            }

            qDebug() << "Saving fastresume data for " << QString::fromStdString(torrent.name());
            torrent.save_resume_data();

            cached++;
        }
        catch (std::exception const& e)
        {
            qWarning() << Q_FUNC_INFO << "caught exception:" << e.what();
        }
    }

    return cached;
}

void TorrentManager::setListeningPort(int firstPort, int lastPort)
{
    libtorrent::error_code ec;
    m_session->listen_on(std::make_pair(firstPort, lastPort), ec);
    if (ec)
    {
        qWarning() << QString("failed to open listen socket: %1").arg(QString::fromStdString(ec.message()));
        if (!QSettings().value(TorrentsPortIsAuto, TorrentsPortIsAuto_Default).toBool())
        {
            QSettings().setValue(TorrentsPortIsAuto, true);
            setListeningPort(6881, 6889);
        }
    }
}

libtorrent::torrent_handle TorrentManager::addTorrent(
    const QString& torrOrMagnet, 
    int id, 
    bool interactive /*= false*/, 
    const QString& savePath,
    const std::vector<boost::uint8_t>* file_priorities)
{
    qDebug() << __FUNCTION__ << " adding file: " << torrOrMagnet;

    libtorrent::add_torrent_params torrentParams;
    torrentParams.save_path = 
        (savePath.isEmpty() ? global_functions::GetVideoFolder() : savePath).toUtf8().constData();
    torrentParams.flags = libtorrent::add_torrent_params::flag_paused | libtorrent::add_torrent_params::flag_override_resume_data;// | libtorrent::add_torrent_params::flag_update_subscribe;
    if (QSettings().value(TorrentsSequentialDownload, TorrentsSequentialDownload_Default).toBool())
        torrentParams.flags |= libtorrent::add_torrent_params::flag_sequential_download;
    torrentParams.userdata = reinterpret_cast<void*>(id);

    torrentParams.storage_mode = libtorrent::storage_mode_allocate;

    const bool enable_file_dialog = interactive 
        && QSettings().value(ShowAddTorrentDialog, ShowAddTorrentDialog_Default).toBool();
    // TODO: may be two different functions
    const bool is_adding_from_file = DownloadType::determineType(torrOrMagnet) != DownloadType::MagnetLink;
    if (!is_adding_from_file)
    {
        torrentParams.url = torrOrMagnet.toStdString();
        TorrentsListener::instance().setFileDialogEnabled(enable_file_dialog);
    }
    else
    {
        libtorrent::error_code err;
        torrentParams.ti = boost::make_shared<libtorrent::torrent_info>(torrOrMagnet.toUtf8().constData(), err);
        if (!torrentParams.ti->is_valid() || err)
        {
            qDebug() << QString("Unable to decode torrent file: '%1', ERROR:%2").arg(torrOrMagnet).arg(err.message().c_str());
            // TODO: handle error according to http://www.rasterbar.com/products/libtorrent/manual.html#error-code
            return {};
        }

        // Set default priory to NORMAL. You can find similar code to magnets in metadata receiver
        torrentParams.file_priorities.resize(torrentParams.ti->num_files(), 2);
    }

    if (file_priorities)
    {
        torrentParams.file_priorities = *file_priorities;
    }

    QString targetInfoHash = is_adding_from_file ? toQString(torrentParams.ti->info_hash()) : btihFromMaget(torrOrMagnet);
    if (loadFastResumeData(targetInfoHash, torrentParams.resume_data))
    {
        qDebug("Successfully loaded fast resume data");
    }

    // Check firewall state
    if (interactive)
    {
        if (Application* myApp = dynamic_cast<Application*>(qApp))
        {
            myApp->checkFirewallException(utilities::getMainWindow());
        }
    }

    libtorrent::torrent_handle duplicate_tor = m_session->find_torrent(hashFromQString(targetInfoHash));
    if (duplicate_tor.is_valid()) // errors::duplicate_torrent?
    {
        // Merge trackers list
        if (interactive && mergeTrackers())
        {
            std::vector<libtorrent::announce_entry> trackers_new
                = is_adding_from_file? torrentParams.ti->trackers() : parseTrackersList(torrOrMagnet);
            std::vector<libtorrent::announce_entry> trackers_current = duplicate_tor.trackers();

            auto trackerPr = [](libtorrent::announce_entry const & l, libtorrent::announce_entry const & r) {return l.url < r.url;};
            std::sort(trackers_current.begin(), trackers_current.end(), trackerPr);
            std::sort(trackers_new.begin(), trackers_new.end(), trackerPr);
            std::set_difference(trackers_new.begin(), trackers_new.end(), 
                trackers_current.begin(), trackers_current.end(),
                makeAddTrackerIterator(duplicate_tor), trackerPr);
        }
        return {};
    }

    // Add torrent dialog
    if (is_adding_from_file && enable_file_dialog)
    {
        AddTorrentForm controlForm(&torrentParams, utilities::getMainWindow());
        if (controlForm.exec() == QDialog::Rejected)
        {
            return {};
        }
    }

    libtorrent::error_code err;
    auto handle = m_session->add_torrent(torrentParams, err);

    if (err)
    {
        qDebug() << err.message().c_str();
    }
    else
    {
        m_idToHandle[id] = handle;

        // Saving torrent
        if (is_adding_from_file)
        {
            QFile::copy(torrOrMagnet, utilities::PrepareCacheFolder(TORRENTS_SUB_FOLDER) + toQString(handle.info_hash()) + ".torrent");
        }
    }

    return handle;
}

void TorrentManager::on_deleteTaskWithID(int id, DownloadType::Type type, int deleteWithFiles)
{
    if (DownloadType::isTorrentDownload(type))
    {
        auto it = m_idToHandle.find(id);
        if (it != m_idToHandle.end() && it->is_valid())
        {
            const auto fileName = utilities::PrepareCacheFolder(TORRENTS_SUB_FOLDER) + toQString(it.value().info_hash());
            utilities::DeleteFileWithWaiting(fileName + ".torrent");
            utilities::DeleteFileWithWaiting(fileName + ".fastresume");
            m_session->remove_torrent(it.value(), deleteWithFiles);
        }
    }
}

void TorrentManager::on_pauseTaskWithID(int id, DownloadType::Type type)
{
    if (DownloadType::isTorrentDownload(type))
    {
        auto it = m_idToHandle.find(id);
        if (it != m_idToHandle.end() && it->is_valid())
        {
            qDebug() << __FUNCTION__ << "pausing:" << it.value().name().c_str();
            it.value().pause();
        }
    }
}

bool TorrentManager::resumeTorrent(int id)
{
    auto it = m_idToHandle.find(id);
    if (it != m_idToHandle.end())
    {
        qDebug() << __FUNCTION__ << "resuming:" << it.value().name().c_str();
        it.value().resume();
        return true;
    }
    return false;
}

bool TorrentManager::restartTorrent(int id)
{
    auto it = m_idToHandle.find(id);
    if (it != m_idToHandle.end())
    {
        libtorrent::torrent_handle handle = it.value();
        auto priorities = handle.file_priorities();

        QString hash = toQString(handle.info_hash());
        const QString torrentPath = utilities::PrepareCacheFolder(TORRENTS_SUB_FOLDER) + hash + ".torrent";
        if (QFile::exists(torrentPath))
        {
            libtorrent::error_code err;
            libtorrent::add_torrent_params torrentParams;
            torrentParams.save_path = handle.save_path();
            torrentParams.flags = libtorrent::add_torrent_params::flag_paused | libtorrent::add_torrent_params::flag_override_resume_data;
            if (QSettings().value(TorrentsSequentialDownload, TorrentsSequentialDownload_Default).toBool())
                torrentParams.flags |= libtorrent::add_torrent_params::flag_sequential_download;
            torrentParams.userdata = reinterpret_cast<void*>(id);
            torrentParams.ti = boost::make_shared<libtorrent::torrent_info>(torrentPath.toUtf8().constData(), err);

            torrentParams.storage_mode = libtorrent::storage_mode_allocate;

            torrentParams.file_priorities = std::vector<uint8_t>(priorities.begin(), priorities.end());

            m_session->remove_torrent(handle);
            libtorrent::torrent_handle newHandle = m_session->add_torrent(torrentParams, err);
            Q_ASSERT(newHandle.is_valid());
            m_idToHandle[id] = newHandle;

            return true;
        }
    }
    return false;
}

libtorrent::torrent_handle TorrentManager::torrentByModelId(int id)
{
    auto it = m_idToHandle.find(id);
    return (it != m_idToHandle.end()) ? it.value() : libtorrent::torrent_handle();
}


QString TorrentManager::torrentRootItemPath(int itemId)
{
    libtorrent::torrent_handle handle = torrentByModelId(itemId);
    return ::torrentRootItemPath(handle);
}

int TorrentManager::port() const
{
    return m_session->listen_port();
}

void TorrentManager::setUploadLimit(int limit)
{
    Q_ASSERT(limit >= 0);
    libtorrent::session_settings settings = m_session->settings();
    settings.upload_rate_limit = limit;
    m_session->set_settings(settings);
}

void TorrentManager::setDownloadLimit(int limit)
{
    Q_ASSERT(limit >= 0);
    libtorrent::session_settings settings = m_session->settings();
    settings.download_rate_limit = limit;
    m_session->set_settings(settings);
}
