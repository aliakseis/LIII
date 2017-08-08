#include "peersinfomodel.h"

PeersInfoModel::PeersInfoModel(libtorrent::torrent_handle torrentHandle, QObject* parent) :
    QAbstractListModel(parent),
    m_torrentHandle(torrentHandle)
{
    torrentHandle.resolve_countries(true);
    updatePeersInfo();
}

int PeersInfoModel::rowCount(const QModelIndex& parent) const
{
    return m_peersInfo.size();
}

int PeersInfoModel::columnCount(const QModelIndex& parent) const
{
    return static_cast<int>(ColumnsCount);
}

libtorrent::peer_info PeersInfoModel::getItem(int row)
{
    libtorrent::peer_info result;
    if (m_peersInfo.size() > (size_t)row)
    {
        result = m_peersInfo[row];
    }
    return result;
}

QVariant PeersInfoModel::data(const QModelIndex& index, int role) const
{
    PeersColumns column = static_cast<PeersColumns>(index.column());
    const libtorrent::peer_info& peerInfo = m_peersInfo[index.row()];
    if (role == Qt::DisplayRole)
    {
        switch (column)
        {
        case ClientName:
            return QString::fromUtf8(peerInfo.client.c_str());
        case Country:
            return QString::fromLatin1(peerInfo.country, 2);
        case IpAddress:
            return QString::fromStdString(peerInfo.ip.address().to_string());
        case PeerProgress:
            return QString("%1 %").arg(peerInfo.progress * 100, 0, 'f', 0);
        case SpeedUpload:
        case SpeedDownload:
            {
                const float l_speed = ((column == SpeedUpload) ? peerInfo.up_speed : peerInfo.down_speed) / 1024.f;
                return (l_speed <= std::numeric_limits<float>::epsilon())
                    ? QString() : tr("%1KB/s").arg(l_speed, 0, 'f', 1);
            }
        }
    }

    return QVariant();
}

void PeersInfoModel::updatePeersInfo()
{
    beginResetModel();
    m_torrentHandle.get_peer_info(m_peersInfo);
    endResetModel();
}

QVariant PeersInfoModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        PeersColumns column = static_cast<PeersColumns>(section);
        switch (column)
        {
        case ClientName:
            return tr("Client name");
        case Country:
            return tr("Country");
        case IpAddress:
            return tr("IP address");
        case PeerProgress:
            return tr("Progress");
        case SpeedUpload:
            return tr("Speed up");
        case SpeedDownload:
            return tr("Speed down");
        }
    }

    return QVariant();
}
