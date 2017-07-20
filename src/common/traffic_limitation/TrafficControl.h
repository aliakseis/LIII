#pragma once

#include <QSocketNotifier>
#include <QNetworkReply>

#include <memory>

namespace traffic_limitation
{

struct ISocketReadInterceptor
{
    ISocketReadInterceptor()
        : isInterceptorSet(false)
    {
    }

    virtual ~ISocketReadInterceptor()
    {
    }

    virtual bool intercept(QSocketNotifier* notifier, QEvent* event) = 0;

    bool isInterceptorSet;
};


bool handleSocketReadNotify(QObject* receiver, QEvent* event);

void subscribeSocketReadInterceptor(QNetworkReply* reply, const std::shared_ptr<ISocketReadInterceptor>& interceptor);

} // namespace traffic_limitation