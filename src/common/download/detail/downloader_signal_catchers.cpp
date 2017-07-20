#include "downloader_signal_catchers.h"


namespace download
{

namespace detail
{

NetworkReplyCatcher::NetworkReplyCatcher(QNetworkReply* source) : BaseType(source) {}
NetworkReplyCatcher::~NetworkReplyCatcher()
{
    disconnectOnDownloadProgress();
    disconnectOnFinished();
    disconnectOnReadyRead();
    disconnectOnError();
}


NetworkAccessManagerCatcher::NetworkAccessManagerCatcher(QNetworkAccessManager* source) : BaseType(source) {}
NetworkAccessManagerCatcher::~NetworkAccessManagerCatcher()
{
    disconnectOnAuthenticationRequired();
    disconnectOnProxyAuthenticationRequired();
}


AuthenticationHelperCatcher::AuthenticationHelperCatcher(utilities::AuthenticationHelper* source) : BaseType(source) {}
AuthenticationHelperCatcher::~AuthenticationHelperCatcher()
{
    disconnectOnAuthNeedLogin();
}


} // namespace detail

} // namespace download

