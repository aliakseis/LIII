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
    void updatePeersInfo();

protected:
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    libtorrent::torrent_handle m_torrentHandle;
    std::vector<libtorrent::peer_info> m_peersInfo;
};
