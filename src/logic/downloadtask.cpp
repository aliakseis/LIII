#include "downloadtask.h"

#include <utility>

#include "utilities/utils.h"
#include "utilities/notify_helper.h"

#include "downloadcollectionmodel.h"
#include "settings_declaration.h"
#include "global_functions.h"
#include "logindialog.h"

#include <QApplication>
#include <QMainWindow>

namespace {

class LoginDialogHelper : public NotifyHelper
{
public:
    explicit LoginDialogHelper(utilities::ICredentialsRetriever* icr)
        : m_icr(icr)
    {
        moveToThread(QApplication::instance()->thread());
    }

    void slotNoParams() override
    {
        auto* dlg = new LoginDialog(utilities::getMainWindow());

        utilities::Credential cred;
        if (dlg->exec() == QDialog::Accepted)
        {
            cred.login = dlg->login();
            cred.password = dlg->password();
        }

        m_icr->SetCredentials(cred);
        dlg->deleteLater();
        deleteLater();
    }

private:
    utilities::ICredentialsRetriever* m_icr;
};


void needLogin(utilities::ICredentialsRetriever* icr)
{
    auto* helper = new LoginDialogHelper(icr);
    if (helper->thread() == QThread::currentThread())
    {
        helper->slotNoParams();
    }
    else
    {
        QMetaObject::invokeMethod(helper, "slotNoParams", Qt::BlockingQueuedConnection);
    }
}

} // namespace


using namespace download;


DownloadTask::DownloadTask(int task_id, QString  url, QObject* parent)
    : QObject(parent),
    downloader_(new DownloaderType(this)),
    network_manager_(new QNetworkAccessManager(this)),
    url_(std::move(url)),
    total_file_size_(0),
    task_id_(task_id),
    tree_item_(DownloadCollectionModel::instance().getRootItem()->findItemByID(task_id)),
    ready_to_download_(false),
    is_torrent_file_(false)
{
    downloader_->setObserver(this);
}

void DownloadTask::start()
{
    downloader_->setDestinationPath(global_functions::GetVideoFolder());
    auto it = DownloadCollectionModel::instance().getItemByID(task_id_);
    downloader_->setExpectedFileSize(it.size());
    on_download();
}

DownloadTask::~DownloadTask()
{
    qDebug() << "DLTask::~DLTask() id=" << task_id_;
}


void DownloadTask::on_download()
{
    qDebug() << QString("%1(%2)  id=%3").arg(__FUNCTION__).arg(url_).arg(task_id_);
    ready_to_download_ = true;

    emit readyToDownload(task_id_);
}

void DownloadTask::download()
{
    qDebug() << QString("%1 of task %2 (url=%3)").arg(__FUNCTION__).arg(task_id_).arg(url_);

    ready_to_download_ = false;
    setStatusInModel(ItemDC::eDOWNLOADING);
    auto it = DownloadCollectionModel::instance().getItemByID(task_id_);
    QFileInfo file(it.downloadedFileName());
    if (file.exists() && file.size() > 0)
    {
        downloader_->Resume(url_, network_manager_.data(), file.fileName());
    }
    else
    {
        if (filename_.isEmpty())
        {
            filename_ = QUrl(it.initialURL()).fileName();
            if (-1 == filename_.indexOf('.'))
            {
                filename_.clear();
            }
        }
        downloader_->Start(url_, network_manager_.data(), filename_);
    }
    emit signalTryNewtask();
}

void DownloadTask::onError(utilities::ErrorCode::ERROR_CODES code, const QString& err)
{
    QString errorDescription  = (err.isEmpty()) ? utilities::ErrorCode::instance().getDescription(code).key : err;
    qDebug() << QString("%1 of task %2 (url=%3), DESCRIPTION: %4").arg(__FUNCTION__).arg(task_id_).arg(url_, errorDescription);
    auto it = DownloadCollectionModel::instance().getItemByID(task_id_);
    it.setStatus(ItemDC::eERROR);
    it.setSpeed(0.f);
    it.setWaitingTime(utilities::ErrorCode::instance().getTimeout(code));
    it.setErrorCode(code);
    it.setErrorDescription(err);
    DownloadCollectionModel::instance().on_ItemDCchange(it);
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
    notifyIfFinished();
}

void DownloadTask::interruptTask()
{
    qDebug() << QString("%1 of task %2 (url=%3)").arg(__FUNCTION__).arg(task_id_).arg(url_);
    if (downloader_.data())
    {
        downloader_->Pause();
    }
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
        DownloadCollectionModel::instance().on_sizeChange(it);
        it.setSizeCurrDownl(downloadedSize);
        DownloadCollectionModel::instance().on_sizeCurrDownlChange(it);
    }
}

void DownloadTask::onSpeed(qint64 bytesPerSeconds)
{
    ItemDC it;
    it.setID(task_id_);
    it.setSpeed(bytesPerSeconds / 1000.);
    DownloadCollectionModel::instance().on_speedChange(it);
}

void DownloadTask::onFinished()
{
    if (!downloader_.data())
    {
        return;
    }
    qDebug() << QString("%1 of task %2 (url=%3)").arg(__FUNCTION__).arg(task_id_).arg(url_);
    qint64 fsize = downloader_->totalFileSize();
    auto it = DownloadCollectionModel::instance().getItemByID(task_id());
    it.setStatus(ItemDC::eFINISHED);
    it.setSizeCurrDownl(fsize);
    it.setSize(fsize);
    it.setSpeed(0.f);
    DownloadCollectionModel::instance().on_ItemDCchange(it);

    notifyIfFinished();
}

void DownloadTask::onFileCreated(const QString& filename)
{
    filename_ = filename;

    ItemDC it;
    it.setID(task_id());
    it.setDownloadedFileName(filename);
    DownloadCollectionModel::instance().on_downloadedFileNameChange(it);
}


void DownloadTask::setStatusInModel(ItemDC::eSTATUSDC a_status, int arg /* = 0*/)
{
    ItemDC it;
    it.setID(task_id_);
    it.setStatus(a_status);
    DownloadCollectionModel::instance().on_statusChange(it);
}

void DownloadTask::onFileToBeReleased(const QString& filename)
{
    qDebug() << QString("%1(%2)  for task id = %3; url=%4").arg(__FUNCTION__).arg(filename).arg(task_id_).arg(url_);
}


float DownloadTask::getSpeed() const
{
    auto it = DownloadCollectionModel::instance().getItemByID(task_id_);
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
        DownloadCollectionModel::instance().on_speedChange(it);
    }
}
#endif // ALLOW_TRAFFIC_CONTROL

int DownloadTask::priority_level() const
{
    if (tree_item_)
    {
        return tree_item_->priority();
    }
    return 0;
}

void DownloadTask::onNeedLogin(utilities::ICredentialsRetriever* retriever)
{
    needLogin(retriever);
}

void DownloadTask::onReplyInvalidated()
{
}

void DownloadTask::onStart(const QByteArray& data)
{
    const char TORRENT_IDENTIFYING_CHARACTERS[] = "d8:announce";
    if (data.startsWith(TORRENT_IDENTIFYING_CHARACTERS))
    {
        is_torrent_file_ = true;
    }
}
