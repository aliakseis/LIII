#include "InterceptTimer.h"

#include <QSocketNotifier>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <winnt.h>
#include <winbase.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/ioctl.h>
QT_BEGIN_INCLUDE_NAMESPACE
#include <../mkspecs/common/posix/qplatformdefs.h>
#include <private/qnet_unix_p.h>
QT_END_INCLUDE_NAMESPACE
#endif

namespace traffic_limitation
{

InterceptTimer::InterceptTimer(QSocketNotifier* receiver, QEvent* event, int speedLimit)
    : QObject(receiver)
    , m_receiver(receiver)
    , m_event(*event)
    , m_speedLimit(speedLimit)
    , plannedInterval(0)
    , m_isStopped(false)
{
    timer.start();
    //handleEvent();
}

void InterceptTimer::timerEvent(QTimerEvent* e)
{
    killTimer(e->timerId());
    if (m_receiver)
    {
        handleEvent();
    }
    else
    {
        m_isStopped = true;
    }
}

void InterceptTimer::onIntercept(int speedLimit)
{
    m_speedLimit = speedLimit;
    if (m_isStopped)
    {
        m_isStopped = false;
        handleEvent();
    }
}

#ifdef Q_OS_WIN

qint64 InterceptTimer::nativeBytesAvailable(int socketDescriptor)
{
    unsigned long  nbytes = 0;
    unsigned long dummy = 0;
    DWORD sizeWritten = 0;
    if (::WSAIoctl(socketDescriptor, FIONREAD, &dummy, sizeof(dummy), &nbytes, sizeof(nbytes), &sizeWritten, 0, 0) == SOCKET_ERROR)
    {
        //WS_ERROR_DEBUG(WSAGetLastError());
        return -1;
    }

    return nbytes;
}

#else

qint64 InterceptTimer::nativeBytesAvailable(int socketDescriptor)
{
    int nbytes = 0;
    // gives shorter than true amounts on Unix domain sockets.
    qint64 available = 0;
    if (qt_safe_ioctl(socketDescriptor, FIONREAD, (char*) &nbytes) >= 0)
    {
        available = (qint64) nbytes;
    }

#if defined (QNATIVESOCKETENGINE_DEBUG)
    qDebug("QNativeSocketEnginePrivate::nativeBytesAvailable() == %lli", available);
#endif
    return available;
}

#endif

enum { MAX_PLANNED_INTERVAL = 5000 }; // milliseconds

void InterceptTimer::handleEvent()
{
    QPointer<QSocketNotifier> receiver = m_receiver;

    while (receiver)
    {

        qint64 readBytes = InterceptTimer::nativeBytesAvailable(receiver.data()->socket());
        static_cast<QObject*>(receiver.data())->event(&m_event);

        if (!receiver)
        {
            // we are dead already
            break;
        }

        if (0 == readBytes)
        {
            m_isStopped = true;
            break;
        }
        else
        {
            int interval = (int) timer.restart();

            int desiredInterval = readBytes * 1000 / m_speedLimit / 1024;
            plannedInterval = (desiredInterval - (interval - plannedInterval));
            if (plannedInterval < 0)
            {
                plannedInterval = 0;
            }
            else
            {
                if (plannedInterval > MAX_PLANNED_INTERVAL)
                {
                    plannedInterval = MAX_PLANNED_INTERVAL;
                }

                const int id = startTimer(plannedInterval);
                Q_ASSERT(id != 0);
                break;
            }
        }
    }
}

} // namespace traffic_limitation
