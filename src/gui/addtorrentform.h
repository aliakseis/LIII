#pragma once

#include <QDialog>
#include <QString>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>

#include <vector>

class TorrentContentFilterModel;

namespace Ui
{
class AddTorrentForm;
}

// TODO: think about renaming this class
class AddTorrentForm : public QDialog
{
    Q_OBJECT
public:
    AddTorrentForm(const libtorrent::torrent_handle& handle, QWidget* parent = 0);
    AddTorrentForm(libtorrent::add_torrent_params* params, QWidget* parent = 0);
    ~AddTorrentForm();

    std::vector<int> filesPriorities() const;

    void accept() override;

    QString savePath() const;
    void setSavePath(const QString& savePath);

    libtorrent::torrent_handle torrentHandle() const { return m_torrentHandle; }

    int exec() override;
private Q_SLOTS:
    void updateDiskSpaceLabel();
    void checkAcceptAvailable();
    void browseSavePath();
    void savePathEdited(const QString& sPath);
    void selectAll();
    void selectNone();

private:
    void initialize();

private:
    Ui::AddTorrentForm* ui;
    TorrentContentFilterModel* m_contentModel;

    const libtorrent::torrent_info* m_torrentInfo;
    libtorrent::add_torrent_params* m_torrentAddParams;
    libtorrent::torrent_handle m_torrentHandle;
    QString m_savePath;
    Q_PROPERTY(QString m_savePath READ savePath WRITE setSavePath)
};
