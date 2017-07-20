#include "downloadtask.h"

#include "utilities/utils.h"

#include "downloadcollectionmodel.h"
#include "settings_declaration.h"
#include "global_functions.h"


using namespace download;
using global_functions::IsContentFile;


DownloadTask::DownloadTask(DownloadCollectionModel* download_collection_model, int task_id, const QString& url, QObject* parent)
    :    QObject(parent),
        download_collection_model_(download_collection_model),
        downloader_(new DownloaderType(this)),
        reply_(nullptr),
        network_manager_(new QNetworkAccessManager(this)),
        url_(url),
        total_file_size_(0),
        task_id_(task_id), time_to_wait_(0), priority_level_(0),
        player_started_(false), ready_to_download_(false),
        free_download_(true), download_direct_link_(true)
{
    VERIFY(connect(download_collection_model_, SIGNAL(signalModelUpdated()), SLOT(updatePriority())));
    downloader_->setObserver(this);
}

void DownloadTask::start()
{
    auto it = download_collection_model_->getItemByID(task_id_);
    priority_level_ = it.priority();
    update_model_time_ = QTime::currentTime();

    downloader_->setDestinationPath(global_functions::GetVideoFolder());
    downloader_->setExpectedFileSize(it.getSize());
    applyStrategy();
}

DownloadTask::~DownloadTask()
{
    qDebug() << "DLTask::~DLTask() id=" << task_id_;
}

void DownloadTask::applyStrategy()
{
    on_download(url_, true);
}

void DownloadTask::on_download(const QString& url, bool isPremium)
{
    qDebug() << QString("%1(%2)  id=%3").arg(__FUNCTION__).arg(url).arg(task_id_);
    free_download_ = !isPremium;
    download_direct_link_ = true;
    direct_link_ = url;
    ready_to_download_ = true;

    emit readyToDownload(task_id_);

    setActualURLinModel();
}

void DownloadTask::on_download(QNetworkReply* reply)
{
    qDebug() << QString("%1(QNetworkReply*) id=%2").arg(__FUNCTION__).arg(task_id_);
    download_direct_link_ = false;
    reply_ = reply;
    ready_to_download_ = true;

    emit readyToDownload(task_id_);

    setActualURLinModel();
}

void DownloadTask::setActualURLinModel()
{
}

void DownloadTask::download()
{
    qDebug() << QString("%1 of task %2 (url=%3)").arg(__FUNCTION__).arg(task_id_).arg(url_);

    ready_to_download_ = false;
    setStatusInModel(ItemDC::eDOWNLOADING);
    auto it = download_collection_model_->getItemByID(task_id_);
    QFileInfo file(it.downloadedFileName());
    if (file.exists() && file.size() > 0)
    {
        if (download_direct_link_)
        {
            downloader_->Resume(direct_link_, network_manager_.data(), file.fileName());
        }
        else
        {
            downloader_->Resume(reply_, network_manager_.data(), file.fileName());
        }
    }
    else
    {
        if (filename_.isEmpty())
        {
            filename_ = QFileInfo(QUrl(it.initialURL()).path()).fileName();
            bool hasCandidate = -1 != filename_.lastIndexOf('.');
            if (!hasCandidate || !IsContentFile(filename_))
            {
                if (!hasCandidate)
                {
                    filename_.clear();
                }
                // Put it here since it seems to be an app specific behavior
                if (reply_ != 0)
                {
                    for (const auto& fn : { utilities::GetFileName(reply_), QFileInfo(reply_->url().path()).fileName() })
                    {
                        if (IsContentFile(fn))
                        {
                            filename_ = fn;
                            break;
                        }
                        else if (!hasCandidate && -1 != fn.lastIndexOf('.'))
                        {
                            filename_ = fn;
                            hasCandidate = true;
                        }
                    }
                }
            }
        }
        if (download_direct_link_)
        {
            downloader_->Start(direct_link_, network_manager_.data(), filename_);
        }
        else
        {
            downloader_->Start(reply_, network_manager_.data(), filename_);
        }
    }
    emit signalTryNewtask();
}

void DownloadTask::on_RemainingTime(int remainingSeconds)
{
    time_to_wait_ = std::max(0, remainingSeconds);
    ItemDC it;
    it.setID(task_id_);
    it.setWaitingTime(remainingSeconds);
    it.setStatus(ItemDC::eWAITING);
    download_collection_model_->on_statusChange(it);
    download_collection_model_->on_waitingTimeChange(it);
}

void DownloadTask::onError(utilities::ErrorCode::ERROR_CODES code, const QString& err)
{
    QString errorDescription  = (err.isEmpty()) ? ErrorCode::instance().getDescription(code).key : err;
    qDebug() << QString("%1 of task %2 (url=%3), DESCRIPTION: %4").arg(__FUNCTION__).arg(task_id_).arg(url_, errorDescription);
    auto it = download_collection_model_->getItemByID(task_id_);
    it.setStatus(ItemDC::eERROR);
    it.setSpeed(0.f);
    it.setWaitingTime(ErrorCode::instance().getTimeout(code));
    it.setErrorCode(code);
    it.setErrorDescription(err);
    download_collection_model_->on_ItemDCchange(it);
    cancelTask();
}

void DownloadTask::cancelTask()
{
    qDebug() << QString("%1 of task %2 (url=%3)").arg(__FUNCTION__).arg(task_id_).arg(url_);
    // changing status is unnecessary, as task is about to be removed from model
    if (downloader_.data())
    {
        downloader_->Cancel();
    }
    delStrategy();
    notifyIfFinished();
}

void DownloadTask::interruptTask()
{
    qDebug() << QString("%1 of task %2 (url=%3)").arg(__FUNCTION__).arg(task_id_).arg(url_);
    if (downloader_.data())
    {
        downloader_->Pause();
    }
    delStrategy();
}

void DownloadTask::notifyIfFinished()
{
    if (!downloader_.data() || downloader_->state() != DownloaderType::kDownloading)
    {
        emit signalDownloadFinished(task_id());
    }
    emit signalTryNewtask();
}

void DownloadTask::onProgress(qint64 downloadedSize)
{
    ItemDC it;
    it.setID(task_id_);

    if (downloader_->totalFileSize())
    {
        total_file_size_ = downloader_->totalFileSize();
        it.setSize(total_file_size_);
        download_collection_model_->on_sizeChange(it);
        const int percentage = downloadedSize * 100 / total_file_size_;
        it.setPercentDownload(percentage);
        it.setSizeCurrDownl(downloadedSize);
        download_collection_model_->on_sizeCurrDownlChange(it);
    }
}

void DownloadTask::onSpeed(qint64 bytesPerSeconds)
{
    ItemDC it;
    it.setID(task_id_);
    it.setSpeed(bytesPerSeconds / 1000.);
    download_collection_model_->on_speedChange(it);
}

void DownloadTask::onFinished()
{
    if (!downloader_.data())
    {
        return;
    }
    qDebug() << QString("%1 of task %2 (url=%3)").arg(__FUNCTION__).arg(task_id_).arg(url_);
    qint64 fsize = downloader_->totalFileSize();
    auto it = download_collection_model_->getItemByID(task_id());
    it.setStatus(ItemDC::eFINISHED);
    it.setPercentDownload(100);
    it.setSizeCurrDownl(fsize);
    it.setSize(fsize);
    it.setSpeed(0.f);
    download_collection_model_->on_ItemDCchange(it);

    free_download_ = false;

    delStrategy();

    notifyIfFinished();
}

void DownloadTask::onFileCreated(const QString& filename)
{
    filename_ = filename;

    ItemDC it;
    it.setID(task_id());
    it.setDownloadedFileName(filename);
    download_collection_model_->on_downloadedFileNameChange(it);

    on_setExtractedFilename(filename);
}

void DownloadTask::on_setExtractedFilename(const QString& filename)
{
    ItemDC it;
    it.setID(task_id());
    it.setExtractedFileName(filename);
    download_collection_model_->on_extractedFileNameChange(it);

    first_extracted_filename_ = filename;
}


void DownloadTask::setStatusInModel(ItemDC::eSTATUSDC a_status, int arg /* = 0*/)
{
    ItemDC it;
    it.setID(task_id_);
    it.setStatus(a_status);
    download_collection_model_->on_statusChange(it);

    if (a_status == ItemDC::eWAITING)
    {
        it.setWaitingTime(arg);
        download_collection_model_->on_waitingTimeChange(it);
    }
}

void DownloadTask::delStrategy()
{
    qDebug() << QString("%1() id=%2").arg(__FUNCTION__).arg(task_id_);
}

void DownloadTask::onFileToBeReleased(const QString& filename)
{
    qDebug() << QString("%1(%2)  for task id = %3; url=%4").arg(__FUNCTION__).arg(filename).arg(task_id_).arg(url_);
}


float DownloadTask::getSpeed() const
{
    auto it = download_collection_model_->getItemByID(task_id_);
    return it.getSpeed();
}

#ifdef ALLOW_TRAFFIC_CONTROL
void DownloadTask::setSpeedLimit(int kbps)
{
    if (downloader_.data())
    {
        downloader_->setSpeedLimit(kbps);
        ItemDC it;
        it.setID(task_id_);
        it.setSpeed(std::min(static_cast<float>(kbps), getSpeed()));
        download_collection_model_->on_speedChange(it);
    }
}
#endif // ALLOW_TRAFFIC_CONTROL

void DownloadTask::updatePriority()
{
    auto it = download_collection_model_->getItemByID(task_id_);
    priority_level_ = it.priority();
}

void DownloadTask::onNeedLogin(utilities::ICredentialsRetriever* retriever)
{
    emit needLogin(retriever);
}

void DownloadTask::on_setFileName(const QString& filename)
{
    filename_ = filename;
}

void DownloadTask::onReplyInvalidated()
{
}

void DownloadTask::on_error(utilities::ErrorCode::ERROR_CODES code, const QString& err)
{
    onError(code, err);
}

