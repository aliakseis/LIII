#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QPointer>

#include "utilities/credsretriever.h"

#include "download/downloader.h"

#include "treeitem.h"

class DownloadCollectionModel;
class ExtractThread;

class DownloadTask : public QObject, public download::DownloaderObserverInterface
{
    Q_OBJECT

public:
#ifdef ALLOW_TRAFFIC_CONTROL
    typedef download::Downloader<download::speed_limitable_tag, false> DownloaderType;
#else
    typedef download::Downloader<download::speed_readable_tag, false> DownloaderType;
#endif // ALLOW_TRAFFIC_CONTROL

    DownloadTask(
        int task_id,
        QString  url,
        QObject* parent = nullptr);
    ~DownloadTask();
    DownloadTask(const DownloadTask&) = delete;
    DownloadTask& operator =(const DownloadTask&) = delete;

    void start();
    void cancelTask();
    void interruptTask();
    void download();
    float getSpeed() const;

    QString fileName() const { return filename_; }
    QString url() const { return url_; }
    int priority_level() const;
    int task_id() const { return task_id_; }
    bool ready_to_download() const { return ready_to_download_; }
    bool is_torrent_file() const { return is_torrent_file_; }
#ifdef ALLOW_TRAFFIC_CONTROL
    int speedLimit() const { return downloader_ ? downloader_->speedLimit() : 0; }
    void setSpeedLimit(int kbps);
#endif

private:
    void notifyIfFinished();
    void setStatusInModel(ItemDC::eSTATUSDC a_status, int arg = 0);
    void on_download();

    void onProgress(qint64 bytes_downloaded) override;
    void onSpeed(qint64 bytes_per_second) override;
    void onFinished() override;
    void onFileCreated(const QString& filename) override;
    void onError(utilities::ErrorCode::ERROR_CODES code, const QString& err) override;
    void onFileToBeReleased(const QString& filename) override;
    void onNeedLogin(utilities::ICredentialsRetriever* retriever) override;
    void onReplyInvalidated() override;
    void onStart(const QByteArray& data) override;

    QScopedPointer<DownloaderType> downloader_;
    QScopedPointer<QNetworkAccessManager> network_manager_;
    QString url_, filename_;
    qint64 total_file_size_;
    int task_id_;
    QPointer<TreeItem> tree_item_;
    bool ready_to_download_;
    bool is_torrent_file_;

Q_SIGNALS:
    void signalDownloadFinished(int ID);
    void signalTryNewtask();
    void readyToDownload(int id);
};
