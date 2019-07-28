#pragma once

#include <QSocketNotifier>
#include <QNetworkReply>

#include <memory>

namespace traffic_limitation
{

struct ISocketReadInterceptor
{
    virtual ~ISocketReadInterceptor() = default;

    virtual bool intercept(QSocketNotifier* notifier, QEvent* event) = 0;

    bool isInterceptorSet = false;
};


bool handleSocketReadNotify(QObject* receiver, QEvent* event);

void subscribeSocketReadInterceptor(QNetworkReply* reply, const std::shared_ptr<ISocketReadInterceptor>& interceptor);

} // namespace traffic_limitation
