#include "NetworkReplyAdaptor.h"

#include "private/qhttpnetworkreply_p.h"
#include "private/qhttpthreaddelegate_p.h"
#include "QConnectionObjectEx.h"

#include "utilities/utils.h"


class QHttpThreadDelegatePublic : public QHttpThreadDelegate
{
public:
    QHttpNetworkReply* getHttpReply() { return httpReply; }
};


namespace traffic_limitation
{

NetworkReplyAdaptor::NetworkReplyAdaptor(QHttpNetworkReply* parent)
    : QObject(parent)
    , replyPrivate(static_cast<QHttpNetworkReplyPrivate*>(static_cast<QConnectionObjectEx*>(static_cast<QObject*>(parent))->dFunc()))
    , threadDelegate(static_cast<QHttpThreadDelegatePublic*>(parent->parent()))
{
    VERIFY(connect(this, SIGNAL(readyRead()), threadDelegate, SLOT(readyReadSlot())));
}

void NetworkReplyAdaptor::readyReadSlot()
{
    if (replyPrivate->statusCode != 401 && replyPrivate->statusCode != 407 && //replyPrivate->shouldEmitSignals()
            threadDelegate->getHttpReply() != 0)
    {
        emit readyRead();
    }
}

} // namespace traffic_limitation
