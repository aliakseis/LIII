#pragma once

#include <QAbstractListModel>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_info.hpp>

class PeersInfoModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum PeersColumns
    {
        ClientName,
        Country,
        IpAddress,
        PeerProgress,
        SpeedUpload,
        SpeedDownload,
        ColumnsCount
    };
    explicit PeersInfoModel(libtorrent::torrent_handle torrentHandle, QObject* parent = 0);
    libtorrent::peer_info getItem(int row);
public slots:
    void updatePeersInfo(const std::vector<libtorrent::peer_info>& peersInfo);

protected:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    libtorrent::torrent_handle m_torrentHandle;
    std::vector<libtorrent::peer_info> m_peersInfo;
};
