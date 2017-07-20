#pragma once

#include <QObject>

class QHttpNetworkReply;
class QHttpNetworkReplyPrivate;
class QHttpThreadDelegatePublic;

namespace traffic_limitation
{

class NetworkReplyAdaptor : public QObject
{
    Q_OBJECT

public:
    NetworkReplyAdaptor(QHttpNetworkReply* parent);

signals:
    void readyRead();

public slots:
    void readyReadSlot();
private:
    QHttpNetworkReplyPrivate* replyPrivate;
    QHttpThreadDelegatePublic* threadDelegate;
};

} // namespace traffic_limitation