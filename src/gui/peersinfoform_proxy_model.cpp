#include "peersinfoform_proxy_model.h"
#include "peersinfomodel.h"


bool PeersSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    if (PeersInfoModel* model = qobject_cast<PeersInfoModel*>(sourceModel()))
    {
        const libtorrent::peer_info& leftPeerInfo = model->getItem(left.row());
        const libtorrent::peer_info& rightPeerInfo = model->getItem(right.row());
        PeersInfoModel::PeersColumns column = static_cast<PeersInfoModel::PeersColumns>(left.column());
        switch (column)
        {
        case  PeersInfoModel::ClientName:
            return leftPeerInfo.client > rightPeerInfo.client;
        case  PeersInfoModel::Country:
            return leftPeerInfo.country[0] > rightPeerInfo.country[0];
        case  PeersInfoModel::IpAddress:
            return leftPeerInfo.ip.address() > rightPeerInfo.ip.address();
        case  PeersInfoModel::PeerProgress:
            return leftPeerInfo.progress > rightPeerInfo.progress;
        case  PeersInfoModel::SpeedDownload:
            return leftPeerInfo.down_speed > rightPeerInfo.down_speed;
        case  PeersInfoModel::SpeedUpload:
            return leftPeerInfo.up_speed > rightPeerInfo.up_speed;
        }
    }
    return QSortFilterProxyModel::lessThan(left, right);
}