#pragma once

#include <QObject>
#include <QPointer>
#include <QEvent>
#include <QElapsedTimer>


class QSocketNotifier;

namespace traffic_limitation
{

class InterceptTimer : public QObject
{
    Q_OBJECT
public:
    InterceptTimer(QSocketNotifier* receiver, QEvent* event, int speedLimit);

    void timerEvent(QTimerEvent* e) override;

    void onIntercept(int speedLimit);

    static qint64 nativeBytesAvailable(int socketDescriptor);

    void handleEvent();

private:
    QPointer<QSocketNotifier> m_receiver;
    QEvent m_event;

    int m_speedLimit;
    QElapsedTimer timer;
    int plannedInterval;
    bool m_isStopped;
};

} // namespace traffic_limitation
