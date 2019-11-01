#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QThread>
#include <QNetworkReply>
#include <QTime>
#include <QDir>
#include <QFileInfo>
#include <QScopedPointer>
#include <QtGlobal>
#include <QPointer>
#include <QDebug>

#include <functional>

#include "utilities/errorcode.h"
#include "utilities/credsretriever.h"
#include "utilities/authentication_helper.h"
#include "utilities/filesystem_utils.h"

#include "download/detail/downloader_signal_catchers.h"
#include "download/detail/downloader_base.h"
#include "download/downloader_traits.h"
#include "utilities/errorcode.h"


// Holds states for download items
struct Downloadable
{
    enum State
    {
        kQueued,        // initial state, download is in queue to start
        kDownloading,    // download in progress
        kPaused,        // Paused means freeze, and able to be resumed, if supported by server
        kFinished,        // Final state of normal download. File should exist, network reply closed
        kFailed,        // Error happened, see error details. Can be redownloaded by placing in queue.
        kCanceled        // Cancelled by user, file deleted. Can be redownloaded by placing in queue.
    };
};

namespace download
{

struct SpeedCalculation
{
    QTime previous_time;
    qint64 previous_progress;
    double speed;
    SpeedCalculation() : previous_progress(0), speed(0) {}
}; // SpeedCalculation

/// \class DownloaderObserverInterface Observer interface for the Downloader
struct DownloaderObserverInterface
{
    virtual void onFinished() = 0;
    virtual void onError(utilities::ErrorCode::ERROR_CODES code, const QString& description) = 0;
    virtual void onProgress(qint64 bytes_downloaded) = 0;
    virtual void onSpeed(qint64 bytes_per_second) = 0;
    virtual void onFileCreated(const QString& filename) = 0;
    virtual void onNeedLogin(utilities::ICredentialsRetriever* retriever) = 0;
    virtual void onReplyInvalidated() = 0;
    virtual void onFileToBeReleased(const QString& filename) = 0;
    virtual void onStart(const QByteArray& data) = 0;
}; // DownloaderObserverInterface


/// \class    Downloader
///
/// \brief    Downloader.
///         Implements downloading mechanism as a couple of source (URL) and destination (file).
///    \param  SpeedControl set to speed_limitable_tag to support speed limitation
///    \param  delete_file_if_error set to true if you don't want to redownload file on each error (for instance network failure)
///

template <class SpeedControl = speed_readable_tag, bool delete_file_if_error = true>
class Downloader : public detail::DownloaderBase<SpeedControl>, public Downloadable
{
public:
    typedef Downloader<SpeedControl, delete_file_if_error> class_type;
    typedef SpeedControl speed_access_category;

    /// \enum    DuplicateDownloadNamePolicy
    /// \brief    kReplaceFile does not change name;
    ///         kGenerateNewName downloads to "filename(<N+1>).ext", where N=0..MAX_INT, until file exists;
    ///
    enum DuplicateDownloadNamePolicy {kGenerateNewName, kReplaceFile};
    enum { DOWNLOAD_MAX_REDIRECTS_ALLOWED = 10 };

    explicit Downloader(QObject* parent = 0)
        : state_(kQueued), paused_download_size_(0), expected_size_(0), current_download_(nullptr),
          current_header_(nullptr), redirect_count_(0), header_checked_(false), size_checked_(false), total_file_size_(0),
          download_from_url_(false), current_header_catcher_(nullptr), current_download_catcher_(nullptr), observer_(nullptr),
          download_name_policy_(kGenerateNewName), network_manager_(nullptr),
          default_filename_(QStringLiteral("download.avi")),
          network_manager_catcher_(new detail::NetworkAccessManagerCatcher(network_manager_)),
          authentication_helper_(new utilities::AuthenticationHelper(1, 1, parent)),
          authentication_helper_catcher_(new detail::AuthenticationHelperCatcher(authentication_helper_.data()))
    {
        authentication_helper_catcher_->connectOnAuthNeedLogin(
            std::bind(&class_type::AuthNeedLogin, this, std::placeholders::_1));
    }

    ~Downloader()
    {
        delete current_header_catcher_;
        delete current_download_catcher_;
    }

    State state() const {return state_;}
    qint64 totalFileSize() const {return total_file_size_;}
    void setTotalFileSize(qint64 value) {total_file_size_ = value;}
    QString resultFileName() const {return output_.fileName();}
    void setObserver(DownloaderObserverInterface* observer) {observer_ = observer;}

    /// \fn    bool Downloader::setDestinationPath(const QString& destination_path)
    ///
    /// \brief    Sets file path to save downloaded files, creates it if it does not exist.
    ///
    /// \param    destination_path    Full pathname of the destination directory.
    ///
    /// \return    true if it succeeds, false if it fails.

    bool setDestinationPath(const QString& destination_path)
    {
        destination_path_ = destination_path;
        if (destination_path_.isEmpty())
        {
            return false;
        }
        QDir path(destination_path_);
        if (!path.exists(destination_path_) && !path.mkpath(destination_path_))
        {
            destination_path_.clear();
            return false;
        }
        return true;
    }

    /// \fn    const QString& Downloader::destinationPath() const
    ///
    /// \brief    Gets the destination path where files are downloaded.
    ///
    /// \return    .

    const QString& destinationPath() const { return destination_path_; }

    void setDefaultFilename(const QString& default_filename) {default_filename_ = default_filename;}

    /// \fn    void Downloader::setExpectedFileSize(qint64 expected_size)
    ///
    /// \brief    Sets expected file size (e.g. if reported externally). Needed for progress reporting.
    ///
    /// \param    expected_size    Size of the expected.
    void setExpectedFileSize(qint64 expected_size)
    {
        expected_size_ = expected_size;
    }

    /// \fn    void Downloader::setDownloadNamePolicy(DuplicateDownloadNamePolicy download_name_policy)
    ///
    /// \brief    Rewrite file if exists, or create with new name.
    ///
    /// \see    DuplicateDownloadNamePolicy for details
    ///
    /// \param    download_name_policy    The download name policy.
    void setDownloadNamePolicy(DuplicateDownloadNamePolicy download_name_policy) {download_name_policy_ = download_name_policy;}
    DuplicateDownloadNamePolicy downloadNamePolicy() const {return download_name_policy_;}

    /// \fn    void Downloader::Start(QNetworkReply* reply, QNetworkAccessManager* network_manager,
    ///     const QString& filename = QString())
    ///
    /// \brief    Starts Download from existing network reply. All settings should be applied before this call.
    ///
    /// \param [in,out]    reply               If non-null, the reply.
    /// \param [in,out]    network_manager    If non-null, manager for network.
    /// \param    filename                   (Optional) Desired filename. May change depending on filename policy. If empty filename from reply will be used.
    /// \see            DuplicateDownloadNamePolicy

    void Start(QNetworkReply* reply, QNetworkAccessManager* network_manager, const QString& filename = QString())
    {
        Q_ASSERT(reply);
        Q_ASSERT(network_manager);
        filename_ = filename;
        download_from_url_ = false;
        doStart(reply->url(), utilities::GetFileName(reply), reply, network_manager);
    }

    /// \fn    void Downloader::Start(const QUrl& url, QNetworkAccessManager* network_manager,
    ///     const QString& filename = QString())
    ///
    /// \brief    Starts download from URL.
    ///
    /// \param    url                           URL of the file to be downloaded.
    /// \param [in,out]    network_manager    If non-null, manager for network.
    /// \param    filename                   (Optional) Desired filename. May change depending on filename policy. If empty filename from reply will be used.
    /// \see            DuplicateDownloadNamePolicy
    void Start(const QUrl& url, QNetworkAccessManager* network_manager, const QString& filename = QString())
    {
        Q_ASSERT(network_manager);
        filename_ = filename;
        download_from_url_ = true;
        doStart(url, QFileInfo(url.path()).fileName(), network_manager->get(QNetworkRequest(url)), network_manager);
    }

    void Cancel()
    {
        if (state() == kDownloading)
        {
            speed_calculation_.previous_progress = 0;
            KillReply();
            KillFile();
            state_ = kCanceled;
            qDebug() << "download::Cancel url=" << current_url_;
        }
    }
    void Stop()
    {
        speed_calculation_.previous_progress = 0;
        KillReply();
        KillFile();
        state_ = kQueued;
        qDebug() << "download::Stop url=" << current_url_;
    }
    void Pause()
    {
        if (state_ != kDownloading) { return; }
        state_ = kPaused;
        if (output_.isOpen())
        {
            FlushOutput();
        }
        KillReply();
        output_.close();
        qDebug() << "download::Pause url=" << current_url_;
    }

    /// \fn    void Downloader::Resume(const QUrl& url, QNetworkAccessManager* network_manager,
    ///     const QString& filename = QString())
    ///
    /// \brief    Resumes the given download. If resume is impossible, will start new download with given filename.
    ///
    /// \param    url                           URL of the file.
    /// \param [in,out]    network_manager    If non-null, manager for network.
    /// \param    filename                   (Optional) filename have to coincide with previous.

    void Resume(const QUrl& url, QNetworkAccessManager* network_manager, const QString& filename = QString())
    {
        Q_ASSERT(network_manager);
        filename_ = filename;
        output_.setFileName(SaveFileName(QFileInfo(url.path()).fileName()));
        download_from_url_ = true;
        doResume(url, QNetworkRequest(url), network_manager);
    }

    void Resume(QNetworkReply* reply, QNetworkAccessManager* network_manager, const QString& filename = QString())
    {
        Q_ASSERT(reply);
        Q_ASSERT(network_manager);
        filename_ = filename;
        output_.setFileName(SaveFileName(utilities::GetFileName(reply)));
        download_from_url_ = false;
        if (doResume(reply->url(), reply->request(), network_manager))
        {
            if (!reply->isFinished())
            {
                reply->abort();
            }
            reply->deleteLater();
            if (observer_)
            {
                observer_->onReplyInvalidated();
            }
        }
    }

    void FlushOutput()
    {
        if (current_download_ != 0)
        {
            qint64 avail = current_download_->bytesAvailable();
            if (avail > 0)
            {
                const QByteArray data = current_download_->read(avail);
                if (observer_ && output_.pos() == 0)
                {
                    observer_->onStart(data);
                }
                output_.write(data);
            }
        }
    }

    bool isRunning() const { return current_download_ != 0; }

private:
    // simplified adapter to void ProcessNetworkReply(QNetworkReply* reply, QNetworkAccessManager* network_manager, bool seekToTheEnd, speed_readable_tag)
    void ProcessNetworkReply(QNetworkReply* reply, QNetworkAccessManager* network_manager, bool seekToTheEnd = true)
    {
        Q_ASSERT(reply);
        Q_ASSERT(network_manager);
        ProcessNetworkReply(reply, network_manager, seekToTheEnd, speed_access_category());
    }

    /// \fn    void Downloader::ProcessNetworkReply(QNetworkReply* reply,
    ///     QNetworkAccessManager* network_manager, bool seekToTheEnd, speed_limitable_tag)
    ///
    /// \brief    Initializes download speed limitation mechanism and calls ProcessNetworkReply(QNetworkReply* reply, QNetworkAccessManager* network_manager, bool seekToTheEnd, speed_readable_tag)
    /// \see    ProcessNetworkReply(QNetworkReply* reply, QNetworkAccessManager* network_manager, bool seekToTheEnd, speed_readable_tag)
    ///
    /// \param [in,out]    reply               If non-null, the network reply.
    /// \param [in,out]    network_manager    If non-null, manager for network.
    /// \param    seekToTheEnd               true to append file, false to rewrite.

    void ProcessNetworkReply(QNetworkReply* reply, QNetworkAccessManager* network_manager, bool seekToTheEnd, speed_limitable_tag)
    {
        Q_ASSERT(reply);
        Q_ASSERT(network_manager);
#ifdef ALLOW_TRAFFIC_CONTROL
        std::shared_ptr<traffic_limitation::ISocketReadInterceptor> interceptor = this->getInterceptor();
        interceptor->isInterceptorSet = false;
        traffic_limitation::subscribeSocketReadInterceptor(reply, interceptor);
#endif//ALLOW_TRAFFIC_CONTROL
        ProcessNetworkReply(reply, network_manager, seekToTheEnd, speed_readable_tag());
    }

    /// \fn    void Downloader::ProcessNetworkReply(QNetworkReply* reply,
    ///     QNetworkAccessManager* network_manager, bool seekToTheEnd, speed_readable_tag)
    ///
    /// \brief    Actually starts downloading process with network reply.
    ///
    /// \param [in,out]    reply               If non-null, the reply.
    /// \param [in,out]    network_manager    If non-null, manager for network.
    /// \param    seekToTheEnd               true to seek to the end.

    void ProcessNetworkReply(QNetworkReply* reply, QNetworkAccessManager* network_manager, bool seekToTheEnd, speed_readable_tag)
    {
        using namespace std::placeholders;

        qDebug() << __FUNCTION__;
        Q_ASSERT(reply);
        Q_ASSERT(network_manager);
        // download initialization
        reply->ignoreSslErrors();
        state_ = kDownloading;
        Q_ASSERT(0 == current_download_);
        current_download_ = reply;
        header_checked_ = false;
        size_checked_ = false;
        redirect_count_ = 0;
        network_manager_ = network_manager;
        network_manager_catcher_->setSource(network_manager);
        network_manager_catcher_->connectOnAuthenticationRequired(
            std::bind(&class_type::AuthenticationRequired, this, _1, _2));
        network_manager_catcher_->connectOnProxyAuthenticationRequired(
            std::bind(&class_type::ProxyAuthenticationRequired, this, _1, _2));
        // error check
        if (!CheckHeader() || !CheckSize())
        {
            return;
        }
        // create or open file to download to
        if (!output_.open(QIODevice::ReadWrite))
        {
            qDebug() << "ProcessNetworkReply() can't open file for writing url=" << current_url_ << ", filename=\"" << output_.fileName() << '"';
            DownloadError(utilities::ErrorCode::eDOWLDOPENFILERR);
            return;
        }
        output_.seek(seekToTheEnd ? output_.size() : 0);
        // notify on download start
        if (observer_)
        {
            observer_->onFileCreated(output_.fileName());
        }
        // initialize progress reporting mechanism
        if (!expected_size_)
        {
            //Q_ASSERT(0 == downloader->current_header_);
            current_header_ = network_manager->head(current_download_->request());
            if (current_header_catcher_)
            {
                current_header_catcher_->setSource(current_header_);
            }
            else
            {
                current_header_catcher_ = new detail::NetworkReplyCatcher(current_header_);
            }
            current_header_catcher_->connectOnFinished(
                std::bind(&class_type::HeaderFinished, this));
        }
        // report if all data already downloaded
        FlushOutput();
        if (current_download_->isFinished())
        {
            qDebug() << "Reply is already finished";
            DownloadFinished();
        }
        else
        {
            // connect observer to progress reporting and return
            if (current_download_catcher_)
            {
                current_download_catcher_->setSource(current_download_);
            }
            else
            {
                current_download_catcher_ = new detail::NetworkReplyCatcher(current_download_);
            }
            current_download_catcher_->connectOnDownloadProgress(
                std::bind(&class_type::DownloadProgress, this, _1, _2));
            current_download_catcher_->connectOnFinished(
                std::bind(&class_type::DownloadFinished, this));
            current_download_catcher_->connectOnReadyRead(
                std::bind(&class_type::ReadyRead, this));
            current_download_catcher_->connectOnError(
                std::bind(&class_type::RemoteError, this, _1));
        }
        speed_calculation_.previous_time = QTime::currentTime();
    }

    void PrepareToFlush(QNetworkReply* current_download, speed_readable_tag) {Q_UNUSED(current_download)}
    void PrepareToFlush(QNetworkReply* current_download, speed_limitable_tag)
    {
#ifdef ALLOW_TRAFFIC_CONTROL
        traffic_limitation::subscribeSocketReadInterceptor(current_download, this->getInterceptor());
#endif//ALLOW_TRAFFIC_CONTROL
    }

    void doStart(const QUrl& url, const QString& remoteFileName, QNetworkReply* reply, QNetworkAccessManager* network_manager)
    {
        qDebug() << "download::doStart " << url;
        paused_download_size_ = 0;
        QString path = SaveFileName(remoteFileName);
        output_.setFileName(ApplyDownloadNamePolicy(path));
        current_url_ = url;
        ProcessNetworkReply(reply, network_manager);
    }

    /// \fn    bool Downloader::doResume(const QUrl& url, QNetworkRequest req,
    ///     QNetworkAccessManager* network_manager)
    ///
    /// \brief    Resumes download if possible, start new otherwise. If returns true network reply must be stopped and deleted!
    ///
    /// \param    url                           URL of the document.
    /// \param    req                           The request.
    /// \param [in,out]    network_manager    If non-null, manager for network.
    ///
    /// \return    true if reply used (if any) is to be deleted, false otherwise.

    bool doResume(const QUrl& url, QNetworkRequest req, QNetworkAccessManager* network_manager)
    {
        qDebug() << "download::doResume " << url;
        current_url_ = url;
        if (output_.exists())
        {
            if ((paused_download_size_ = output_.size()) > 0)
            {
                if (total_file_size_ == paused_download_size_)
                {
                    state_ = kFinished;
                    if (observer_)
                    {
                        observer_->onFinished();
                    }
                }
                else
                {
                    qDebug() << "Trying to download from size " << paused_download_size_;
                    QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(paused_download_size_) + "-";
                    req.setRawHeader("Range", rangeHeaderValue);
                    ProcessNetworkReply(network_manager->get(req), network_manager);
                }
                return true;
            }
            Start(url, network_manager, filename_);
        }
        else
        {
            DownloadError(utilities::ErrorCode::eDOWLDOPENFILERR, "File's not found");
            Cancel();
        }
        return false;
    }

    // error check of reply header
    bool CheckHeader()
    {
        if (!header_checked_ && current_download_->rawHeaderList().size())
        {
            qDebug() << __FUNCTION__;
            header_checked_ = true;
            if (current_download_->error())
            {
                qDebug() << "Downloader: network error: " << current_download_->error();
                DownloadError(utilities::ErrorCode::eDOWLDNETWORKERR,
                    QString("Downloader network error: ") + QString::number(current_download_->error()));
                return false;
            }
            int http_code = current_download_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "HTTP status: "<< http_code;
            if (http_code >= 400)
            {
                Restart(utilities::ErrorCode::eDOWLDHTTPCODERR,
                    utilities::Tr::Tr(utilities::NETWORK_ERROR_HTTP_STATUS).arg(http_code));
                return false;
            }
            QString redirect_src = current_download_->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
            if (!redirect_src.isEmpty())
            {
                if (redirect_count_ >= DOWNLOAD_MAX_REDIRECTS_ALLOWED)
                {
                    KillReply();
                    KillFile();
                    DownloadError(utilities::ErrorCode::eENDLESSREDIRECT, redirect_src);
                }
                else
                {
                    redirect_count_++;
                    QUrl redirect_url(redirect_src);
                    if (redirect_url.isRelative())
                    {
                        qDebug() << "Downloader::CheckHeader() Url is relative";
                        redirect_src = current_url_.resolved(redirect_url).toString();
                    }
                    qDebug() << "Downloader::checkHeader() Redirect to: " << redirect_src;
                    KillReply();
                    if (0 == paused_download_size_) // we are not resuming
                    {
                        KillFile();
                        Start(redirect_src, network_manager_, filename_);
                    }
                    else
                    {
                        output_.close();
                        Resume(redirect_src, network_manager_, filename_);
                    }
                    return false;
                }
            }
            // Headers differ for content and error returns
            if (download_from_url_ && !current_download_->hasRawHeader("Content-Disposition") &&
                    current_download_->header(QNetworkRequest::ContentTypeHeader).toString().startsWith("text/"))
            {
                qDebug() << "Downloader: response containing error description detected";
                QString replyText = current_download_->read(512);
                qDebug() << replyText;
                int endlPos = replyText.indexOf('\n');
                if (endlPos > 0)
                {
                    replyText.truncate(endlPos);
                }

                Restart(utilities::ErrorCode::eDOWLDCONTENTLENGTHERR, replyText);
                return false;
            }
        }
        return true;
    }

    // implements file size checking
    bool CheckSize()
    {
        if (!size_checked_ && expected_size_)
        {
            qDebug() << __FUNCTION__;
            size_checked_ = true;
            if (expected_size_ > 0)
            {
                qint64 filesize = current_download_->header(QNetworkRequest::ContentLengthHeader).toLongLong();
                if (filesize && expected_size_ != filesize + paused_download_size_ &&
                        (!current_header_ || current_download_->header(QNetworkRequest::ContentTypeHeader).toString() ==
                         current_header_->header(QNetworkRequest::ContentTypeHeader).toString()))
                {
                    qDebug() << "Downloader: file size mismatch";
                    Restart(utilities::ErrorCode::eDOWLDCONTENTLENGTHERR);
                    return false;
                }
            }
            if (current_header_)
            {
                current_header_->deleteLater();
                current_header_ = nullptr;
            }
        }
        return true;
    }

    // unconnects observer and deleted reply
    void KillReply()
    {
        if (current_header_)
        {
            if (current_header_catcher_)
            {
                current_header_catcher_->disconnectOnFinished();
            }
            if (!current_header_->isFinished())
            {
                current_header_->abort();
            }
            current_header_->deleteLater();
            current_header_ = nullptr;
        }
        if (current_download_)
        {
            if (current_download_catcher_)
            {
                current_download_catcher_->disconnectOnDownloadProgress();
                current_download_catcher_->disconnectOnFinished();
                current_download_catcher_->disconnectOnReadyRead();
                current_download_catcher_->disconnectOnError();
            }
            if (!current_download_->isFinished())
            {
                current_download_->abort();
            }
            current_download_->deleteLater();
            current_download_ = nullptr;
        }
        this->resetInterceptor();
    }

    // gracefully deletes file
    void KillFile()
    {
        if (output_.isOpen())
        {
            output_.close();
        }
        if (!output_.exists())
        {
            return;
        }
        if (observer_)
        {
            observer_->onFileToBeReleased(output_.fileName());
        }
        utilities::DeleteFileWithWaiting(output_.fileName()); // invoking DownloadError must be incorrect here
    }
    // attempts to restart failed download in case of failure
    void Restart(utilities::ErrorCode::ERROR_CODES error_code, QString const& errText = QString())
    {
        if (paused_download_size_)
        {
            qDebug() << "Downloader: Will try to re-download from scratch";
            paused_download_size_ = 0;
            QUrl url = current_download_->url();
            KillReply();
            output_.close();
            download_from_url_ = true;
            current_url_ = url;
            QNetworkReply* new_reply = network_manager_->get(QNetworkRequest(current_url_));
            ProcessNetworkReply(new_reply, network_manager_, false);
        }
        else
        {
            DownloadError(error_code, errText);
        }
    }

    void DownloadError(utilities::ErrorCode::ERROR_CODES code, const QString& text = QString())
    {
        qDebug() << __FUNCTION__ << " code=" << utilities::ErrorCode::instance().getDescription(code).key << " url= " << current_url_;
        KillReply();
        if (delete_file_if_error)
        {
            KillFile();
        }
        else
        {
            output_.close();
        }
        state_ = kFailed;
        speed_calculation_.previous_progress = 0;
        if (observer_)
        {
            observer_->onError(code, text);
        }
    }

    /// \brief returns filename of download, considering filename_ (set from reply headers), naming policy in case of file exists, and parameter (as backup filename).
    /// \param remote_filename uses if filename_ is empty (not set)
    ///
    /// filename_ (usually set from header) has maximum priority. If empty remote_filename is used.
    /// If both filename_ and remote_filename are empty, default_filename_ is used.
    ///
    /// destination_path_ is used to save file to; if filename has slashes or special signs, they are replaced with underscore.
    ///
    QString SaveFileName(const QString& remote_filename)
    {
        QString path = filename_;
        if (path.isEmpty())
        {
            path = remote_filename;
            if (path.isEmpty())
            {
                path = default_filename_;
            }
        }
        path.remove('\"').replace(QRegExp("[/\\\\:*?<>|]"), "_");
        if (!destination_path_.isEmpty())
        {
            QString destPath = destination_path_;
            if (!destPath.endsWith(QDir::separator()))
            {
                destPath += QDir::separator();
            }
            path = destPath + path;
        }
        return path;
    }

    /// \fn    QString Downloader::ApplyDownloadNamePolicy(const QString& path)
    ///
    /// \brief    Applies the download name policy described by path.
    /// \see    DuplicateDownloadNamePolicy
    ///
    /// \param    path    Desired full pathname of the file
    ///
    /// \return    path considering the policy
    ///
    ///         If     kReplaceFile policy is chosen and file exists, it will be deleted first.
    ///         If it is impossible to delete the file observer will be notified on Error with ErrorCode::eDOWLDUNKWNFILERR.
    ///
    ///         If kGenerateNewName is chosen and file exists new filename will be generated as filename(N).ext,
    ///         where N is integer, starting from 1 until file with such name exists.
    QString ApplyDownloadNamePolicy(const QString& path)
    {
        if (QFile::exists(path))
            switch (download_name_policy_)
            {
            case kGenerateNewName:
            {
                QFileInfo ff(path);
                QString left = destination_path_;
                if (!destination_path_.endsWith(QDir::separator()))
                {
                    left += QDir::separator();
                }
                left += ff.baseName() + '(';
                QString right = ")." + ff.completeSuffix();
                int i = 1;
                QString new_path;
                while (QFile::exists(new_path = left + QString::number(i) + right))
                {
                    ++i;
                }
                filename_ = QFileInfo(new_path).fileName();
                return new_path;
            }
            case kReplaceFile:
                if (!utilities::DeleteFileWithWaiting(path) && delete_file_if_error)
                {
                    DownloadError(utilities::ErrorCode::eDOWLDUNKWNFILERR, "Cannot delete file to replace");
                }
                break;
            default:
                DownloadError(utilities::ErrorCode::eDOWLDUNKWNFILERR, "Unknown duplicate file name policy");
            }
        return path;
    }

    // reports progress and speed to observer
    void DownloadProgress(qint64 current, qint64 total)
    {
        if (!observer_ || total <= 0)
        {
            return;
        }
        total_file_size_ = total + paused_download_size_;
        observer_->onProgress(current + paused_download_size_);
        const QTime current_time = QTime::currentTime();
        int calculation_interval = speed_calculation_.previous_time.msecsTo(current_time);
        if (calculation_interval > 1000)
        {
            speed_calculation_.speed = (current - speed_calculation_.previous_progress) * 1000. / calculation_interval;
            speed_calculation_.previous_progress = current;
            speed_calculation_.previous_time = current_time;

            observer_->onSpeed(speed_calculation_.speed); // update speed only when changed
        }
    }

    // processes download finish, reports state to observer
    void DownloadFinished()
    {
        qDebug() << __FUNCTION__;
        if (!CheckHeader() || !CheckSize()) { return; }
        qDebug() << "Downloader::DownloadFinished url= " << current_url_;
        // deinitializing current_header_ and current_header_catcher_ in KillReply()
        FlushOutput();
        KillReply();
        output_.close();
        state_ = kFinished;
        speed_calculation_.previous_progress = 0;
        network_manager_ = nullptr;
        if (observer_) { observer_->onFinished(); }
    }

    // error handler
    void RemoteError(QNetworkReply::NetworkError download_error)
    {
        if (QNetworkReply::RemoteHostClosedError == download_error && expected_size_ > 0)
        {
            FlushOutput();
            output_.flush();
            qint64 result_size = output_.size();
            if (result_size > paused_download_size_ && result_size <= expected_size_)
            {
                // try to quickly restart download if it has occasionally interrupted
                if (result_size < expected_size_)
                {
                    QNetworkRequest req = current_download_->request();
                    KillReply();
                    paused_download_size_ = result_size;
                    output_.close();
                    qDebug() << "Retrying to download from size " << paused_download_size_;
                    QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(paused_download_size_) + "-";
                    req.setRawHeader("Range", rangeHeaderValue);
                    ProcessNetworkReply(network_manager_->get(req), network_manager_);
                }
                return;
            }
        }
        else if (paused_download_size_ > 0) // Let's try to download from scratch first if we are resuming
        {
            Restart(utilities::ErrorCode::eDOWLDHTTPCODERR,
                utilities::Tr::Tr(utilities::NETWORK_ERROR_NO_MSG).arg(current_download_->error()));
            return;
        }
        DownloadError(utilities::ErrorCode::eDOWLDNETWORKERR,
            utilities::Tr::Tr(utilities::NETWORK_ERROR_NO_MSG).arg(current_download_->error()));
    }

    // sets expected file size if reported in reply header
    void HeaderFinished()
    {
        if (!current_header_) { return; }
        qDebug() << "Downloader::on_headerFinished url= " << current_header_->url();
        if (current_header_catcher_)
        {
            current_header_catcher_->disconnectOnFinished();
        }
        expected_size_ = current_header_->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        if (!expected_size_)
        {
            expected_size_ = -1;
        }
    }

    void AuthenticationRequired(QNetworkReply* /*reply*/, QAuthenticator* authenticator)
    {
        qDebug() << "Authentication request";
        authentication_helper_->getWebAuthentication(authenticator);
    }
    void ProxyAuthenticationRequired(const QNetworkProxy& /*proxy*/, QAuthenticator* authenticator)
    {
        qDebug() << "Proxy authentication request";
        authentication_helper_->getProxyAuthentication(authenticator);
    }
    void AuthNeedLogin(utilities::ICredentialsRetriever* retriever)
    {
        Q_ASSERT(retriever);
        if (observer_ && retriever)
        {
            observer_->onNeedLogin(retriever);
        }
    }

    // the main downloading logic is periodically flush file to disk
    void ReadyRead()
    {
        PrepareToFlush(current_download_, speed_access_category());
        if (current_download_ && CheckHeader() && CheckSize())
        {
            FlushOutput();
        }
    }

    State state_;
    SpeedCalculation speed_calculation_;
    QString filename_;
    QFile output_;
    QUrl current_url_;
    qint64 paused_download_size_, expected_size_;
    QPointer<QNetworkReply> current_download_;
    QNetworkReply* current_header_;
    int redirect_count_;
    bool header_checked_, size_checked_;
    QString destination_path_;
    qint64 total_file_size_;
    bool download_from_url_;
    detail::NetworkReplyCatcher* current_header_catcher_;
    detail::NetworkReplyCatcher* current_download_catcher_;
    DownloaderObserverInterface* observer_;
    DuplicateDownloadNamePolicy download_name_policy_;
    QNetworkAccessManager* network_manager_;
    QString default_filename_;
    QScopedPointer<detail::NetworkAccessManagerCatcher> network_manager_catcher_;
    QScopedPointer<utilities::AuthenticationHelper> authentication_helper_;
    QScopedPointer<detail::AuthenticationHelperCatcher> authentication_helper_catcher_;
};

} // namespace download
