#pragma once

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QAuthenticator>
#include <QNetworkProxy>

#include "utilities/credsretriever.h"
#include "utilities/authentication_helper.h"
#include "utilities/utils.h"


namespace download
{

namespace detail
{

template <class CatcheeType>
class CatcherSource
{
public:
    explicit CatcherSource(CatcheeType* source) : source_(source) {}

    void setSource(CatcheeType* source) { source_ = source; }
    CatcheeType* source() const { return source_; }

private:
    CatcheeType* source_;
};


class NetworkReplyCatcher : public CatcherSource<QNetworkReply>
{
public:
    typedef CatcherSource<QNetworkReply> BaseType;

    explicit NetworkReplyCatcher(QNetworkReply* source);
    ~NetworkReplyCatcher();

    template <typename T>
    void connectOnDownloadProgress(T receiver)
    {
        on_downloadProgress_signal_ = QObject::connect(source(), &QNetworkReply::downloadProgress, receiver);
    }
    template <typename T>
    void connectOnFinished(T receiver)
    {
        on_finished_signal_ = QObject::connect(source(), &QNetworkReply::finished, receiver);
    }
    template <typename T>
    void connectOnReadyRead(T receiver)
    {
        on_readyRead_signal_ = QObject::connect(source(), &QNetworkReply::readyRead, receiver);
    }
    template <typename T>
    void connectOnError(T receiver)
    {
        on_error_signal_ = QObject::connect(
            source(), static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), receiver);
    }

    void disconnectOnDownloadProgress()
    {
        QObject::disconnect(on_downloadProgress_signal_);
    }
    void disconnectOnFinished()
    {
        QObject::disconnect(on_finished_signal_);
    }
    void disconnectOnReadyRead()
    {
        QObject::disconnect(on_readyRead_signal_);
    }
    void disconnectOnError()
    {
        QObject::disconnect(on_error_signal_);
    }

private:
    QMetaObject::Connection on_downloadProgress_signal_;
    QMetaObject::Connection on_finished_signal_;
    QMetaObject::Connection on_readyRead_signal_;
    QMetaObject::Connection on_error_signal_;
};


class NetworkAccessManagerCatcher : public CatcherSource<QNetworkAccessManager>
{
public:
    typedef CatcherSource<QNetworkAccessManager> BaseType;

    explicit NetworkAccessManagerCatcher(QNetworkAccessManager* source);
    ~NetworkAccessManagerCatcher();

    template <typename T>
    void connectOnAuthenticationRequired(T receiver)
    {
        on_authenticationRequired_signal_ = QObject::connect(source(), &QNetworkAccessManager::authenticationRequired, receiver);
    }
    template <typename T>
    void connectOnProxyAuthenticationRequired(T receiver)
    {
        on_proxyAuthenticationRequired_signal_ = QObject::connect(source(), &QNetworkAccessManager::proxyAuthenticationRequired, receiver);
    }

    void disconnectOnAuthenticationRequired()
    {
        QObject::disconnect(on_authenticationRequired_signal_);
    }
    void disconnectOnProxyAuthenticationRequired()
    {
        QObject::disconnect(on_proxyAuthenticationRequired_signal_);
    }

private:
    QMetaObject::Connection on_authenticationRequired_signal_;
    QMetaObject::Connection on_proxyAuthenticationRequired_signal_;
};

class AuthenticationHelperCatcher : public CatcherSource<utilities::AuthenticationHelper>
{
public:
    typedef CatcherSource<utilities::AuthenticationHelper> BaseType;

    explicit AuthenticationHelperCatcher(utilities::AuthenticationHelper* source);
    ~AuthenticationHelperCatcher();

    template <typename T>
    void connectOnAuthNeedLogin(T receiver)
    {
        on_authNeedLogin_signal_ = QObject::connect(source(), &utilities::AuthenticationHelper::authNeedLogin, receiver);
    }

    void disconnectOnAuthNeedLogin()
    {
        VERIFY(QObject::disconnect(on_authNeedLogin_signal_));
    }

private:
    QMetaObject::Connection on_authNeedLogin_signal_;
};

} // namespace detail

} // namespace download
