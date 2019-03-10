#pragma once

#include <QDialog>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>

#include <QSortFilterProxyModel>

class TorrentContentFilterModel;

namespace Ui
{
class TorrentDetailsForm;
}

class PeersInfoModel;

// TODO: think about renaming this class
class TorrentDetailsForm : public QDialog
{
    Q_OBJECT
public:
    TorrentDetailsForm(const libtorrent::torrent_handle& handle, QWidget* parent = 0);
    ~TorrentDetailsForm();

    QStringList filesPriorities() const;

    void accept() override;

    QString savePath() const;
    void setSavePath(const QString& savePath);

    libtorrent::torrent_handle torrentHandle() const { return m_torrentHandle; }

    void onProgressUpdated();

Q_SIGNALS:
    void updateFilesProgress(const std::vector<boost::int64_t>& fp);

private Q_SLOTS:
    void updateDiskSpaceLabel();
    void browseSavePath();
    void savePathEdited(const QString& sPath);
    void adaptColumns(int col);
    void onItemExpanded(const QModelIndex& index);
    void onUpdateFilesProgress(const std::vector<boost::int64_t>& fp);

    void openPersistentEditors();
private:
    void initialize();
    void initTorrentContentTab();
    void initPeersInfoTab();

private:
    Ui::TorrentDetailsForm* ui;
    TorrentContentFilterModel* m_contentModel;

    const libtorrent::torrent_info* m_torrentInfo;
    libtorrent::add_torrent_params* m_torrentAddParams;
    libtorrent::torrent_handle m_torrentHandle;

    PeersInfoModel* m_PeersInfomodel;
    QSortFilterProxyModel* m_PeersInfoproxy;
    int m_updateTimeId;

    QString m_savePath;
    Q_PROPERTY(QString m_savePath READ savePath WRITE setSavePath)
protected:
    void timerEvent(QTimerEvent* event)override;
};
