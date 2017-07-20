#pragma once

#include <QObject>

namespace traffic_limitation
{

class QConnectionObjectEx : public QObject
{
    Q_DECLARE_PRIVATE(QObject)
public:
    inline bool isSender(const QObject* receiver, const char* signal) const
    { return d_func()->isSender(receiver, signal); }
    inline QObjectList receiverList(const char* signal) const
    { return d_func()->receiverList(signal); }
    inline QObjectList senderList() const
    { return d_func()->senderList(); }
    QObjectPrivate* dFunc() { return d_func(); }
    const QObjectPrivate* dFunc() const { return d_func(); }
};

template<typename T>
inline decltype((*(T*)0)((QObject*)0)) findFirstThat(const QObject* obj, const char* signal, T fun)
{
    if (0 == obj)
    {
        return 0;
    }

    QObjectList receivers = static_cast<const QConnectionObjectEx*>(obj)->receiverList(signal);
    Q_FOREACH(QObject * item, receivers)
    {
        if (auto result = fun(item))
        {
            return result;
        }
    }

    return 0;
}

template<typename T>
inline decltype((*(T*)0)((QObject*)0)) findFirstThat(const QObject* obj, T fun)
{
    if (0 == obj)
    {
        return 0;
    }

    QObjectList senders = static_cast<const QConnectionObjectEx*>(obj)->senderList();
    Q_FOREACH(QObject * item, senders)
    {
        if (auto result = fun(item))
        {
            return result;
        }
    }

    return 0;
}

} // namespace traffic_limitation
