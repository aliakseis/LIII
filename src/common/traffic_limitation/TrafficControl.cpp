#include "TrafficControl.h"

#include <QEvent>
#include <QWeakPointer>
#include <QReadWriteLock>
#include "NetworkReplyAdaptor.h"


#include <QTcpSocket>

//#ifdef Q_OS_WIN
QT_BEGIN_INCLUDE_NAMESPACE
#include <private/qobject_p.h>
#include <private/qauthenticator_p.h>
#include <private/qhttpnetworkconnectionchannel_p.h>
#include <private/qhttpthreaddelegate_p.h>
QT_END_INCLUDE_NAMESPACE
//#endif

#include "QConnectionObjectEx.h"
#include "utilities/utils.h"

#include <functional>
#include <string.h>
#include <unordered_map>

namespace {

QReadWriteLock receiverMapLock;

std::unordered_map<QObject*, std::shared_ptr<traffic_limitation::ISocketReadInterceptor> > receiverMap;

void removeObjectFromReceiverMap(QObject* obj)
{
    QWriteLocker locker(&receiverMapLock);
    VERIFY(1 == receiverMap.erase(obj));
}

struct IsHttpNetworkConnectionChannel
{
    QHttpNetworkConnectionChannel* operator()(QObject* item) const
    {
        if (nullptr == item || 0 != strcmp("QHttpNetworkConnectionChannel", item->metaObject()->className()))
        {
            return nullptr;
        }
        return static_cast<QHttpNetworkConnectionChannel*>(item);
    }
};

}  // namespace

namespace traffic_limitation
{

bool handleSocketReadNotify(QObject* receiver, QEvent* e)
{
    if (QEvent::SockAct == e->type())
    {
        auto notifier = qobject_cast<QSocketNotifier*>(receiver);
        if (notifier != nullptr && notifier->isEnabled())
        {
            QSocketNotifier::Type type = notifier->type();
            if (QSocketNotifier::Read == type && receiver->parent() != nullptr)
            {
                if (auto tcpSocket = qobject_cast<QTcpSocket*>(receiver->parent()->parent()))
                {

                    QHttpNetworkConnectionChannel* channel 
                        = findFirstThat(tcpSocket, "readyRead()", IsHttpNetworkConnectionChannel());

                    if (nullptr == channel)
                    {
                        // presumably SSL stuff
                        auto sslSocket = qobject_cast<QTcpSocket*>(tcpSocket->parent());
                        channel = findFirstThat(sslSocket, "readyRead()", IsHttpNetworkConnectionChannel());
                    }

                    if (channel != nullptr && channel->reply != nullptr && notifier->isEnabled())
                    {
                        if (QObject* delegate = channel->reply->parent())
                        {
                            std::shared_ptr<traffic_limitation::ISocketReadInterceptor> interceptor;
                            {
                                const QObjectList receivers 
                                    = static_cast<const QConnectionObjectEx*>(delegate)->receiverList("downloadData(QByteArray)");
                                QReadLocker locker(&receiverMapLock);
                                for (QObject* receiver : receivers)
                                {
                                    auto it = receiverMap.find(receiver);
                                    if (it != receiverMap.end())
                                    {
                                        interceptor = it->second;
                                        break;
                                    }
                                }
                            }
                            if (interceptor)
                            {
                                auto replyPrivate
                                    = static_cast<QHttpNetworkReplyPrivate*>(static_cast<QConnectionObjectEx*>(static_cast<QObject*>(channel->reply))->dFunc());

                                if (!replyPrivate->downstreamLimited)
                                {
                                    VERIFY(QObject::disconnect(channel->reply, SIGNAL(readyRead()), delegate, SLOT(readyReadSlot())));
                                    auto networkReplyAdaptor = new NetworkReplyAdaptor(channel->reply);
                                    VERIFY(QObject::connect(tcpSocket, SIGNAL(readyRead()), networkReplyAdaptor, SLOT(readyReadSlot())));

                                    replyPrivate->downstreamLimited = true;
                                }

                                return interceptor->intercept(notifier, e);
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}


void subscribeSocketReadInterceptor(QNetworkReply* reply, const std::shared_ptr<ISocketReadInterceptor>& interceptor)
{
    if (nullptr == reply || nullptr == interceptor || interceptor->isInterceptorSet)
    {
        return;
    }

    QObject* receiver = reply;

    QObject::connect(receiver, &QObject::destroyed, removeObjectFromReceiverMap);

    receiverMapLock.lockForWrite();
    receiverMap[receiver] = interceptor;
    receiverMapLock.unlock();

    interceptor->isInterceptorSet = true;
}

} // namespace traffic_limitation
