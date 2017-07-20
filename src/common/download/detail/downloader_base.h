#pragma once

#include <QSocketNotifier>
#include <QEvent>
#include <QWeakPointer>

#ifdef ALLOW_TRAFFIC_CONTROL
#include "traffic_limitation/TrafficControl.h"
#include "traffic_limitation/InterceptTimer.h"
#endif // #ifdef ALLOW_TRAFFIC_CONTROL

#include "download/downloader_traits.h"

#include <memory>
#include <atomic>

namespace download
{

namespace detail
{

template <class SpeedControl>
class DownloaderBase
{
public:
    void setSpeedLimit(int) {}
    int speedLimit() const { return 0; }
protected:
    void resetInterceptor() {}
};


#ifdef ALLOW_TRAFFIC_CONTROL

template <>
class DownloaderBase<speed_limitable_tag>
{
public:
    DownloaderBase() : m_socketReadInterceptor(std::make_shared<SocketReadInterceptorImpl>()) {}

    // We can use negative speed limits to hold disabled control values
    void setSpeedLimit(int speed_limit) { m_socketReadInterceptor->speed_limit_ = speed_limit; }
    int speedLimit() const { return m_socketReadInterceptor->speed_limit_; }

    std::shared_ptr<traffic_limitation::ISocketReadInterceptor> getInterceptor() const
    {
        return m_socketReadInterceptor;
    }

protected:
    void resetInterceptor()
    {
        int speedLimit = m_socketReadInterceptor->speed_limit_;
        m_socketReadInterceptor = std::make_shared<SocketReadInterceptorImpl>();
        m_socketReadInterceptor->speed_limit_ = speedLimit;
    }

private:

    class SocketReadInterceptorImpl : public traffic_limitation::ISocketReadInterceptor
    {
    public:
        SocketReadInterceptorImpl() : speed_limit_(0) {}
        virtual bool intercept(QSocketNotifier* notifier, QEvent* event) override
        {
            const int speed_limit = speed_limit_;
            if (speed_limit <= 0)
            {
                if (intercept_timer_)
                {
                    delete intercept_timer_.data();
                    intercept_timer_ = nullptr;
                }
                return false;
            }
            if (!intercept_timer_)
                try
                {
                    intercept_timer_ = new traffic_limitation::InterceptTimer(notifier, event, speed_limit);
                    intercept_timer_->handleEvent();
                }
                catch (...)
                {
                    intercept_timer_ = nullptr;
                }
            else
            {
                intercept_timer_.data()->onIntercept(speed_limit);
            }

            return true;
        }

        std::atomic<int> speed_limit_;
        QPointer<traffic_limitation::InterceptTimer> intercept_timer_;
    };

    std::shared_ptr<SocketReadInterceptorImpl> m_socketReadInterceptor;
};

#endif // #ifdef ALLOW_TRAFFIC_CONTROL

} // namespace detail

} // namespace download
