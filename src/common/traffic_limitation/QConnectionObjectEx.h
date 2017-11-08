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
inline auto findFirstThat(const QObject* obj, const char* signal, T fun) -> decltype(fun(nullptr))
{
    if (0 == obj)
    {
        return 0;
    }

    const QObjectList receivers = static_cast<const QConnectionObjectEx*>(obj)->receiverList(signal);
    for (QObject* item : receivers)
    {
        if (auto result = fun(item))
        {
            return result;
        }
    }

    return 0;
}

} // namespace traffic_limitation
