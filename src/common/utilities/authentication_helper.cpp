#include "authentication_helper.h"

#include <QAuthenticator>
#include "credsretriever.h"


namespace utilities
{

AuthenticationHelper::AuthenticationHelper(int proxyChecks, int networkChecks, QObject* parent)
    : QObject(parent)
    , m_proxyCounter(proxyChecks)
    , m_networkCounter(networkChecks)
{
    Q_ASSERT(m_proxyCounter);
    Q_ASSERT(m_networkCounter);
}

void AuthenticationHelper::getProxyAuthentication(QAuthenticator* authenticator)
{
    getAuthentication(m_proxyCounter, authenticator);
}

void AuthenticationHelper::getWebAuthentication(QAuthenticator* authenticator)
{
    getAuthentication(m_networkCounter, authenticator);
}

void AuthenticationHelper::getAuthentication(int& counter, QAuthenticator* authenticator)
{
    if (counter > 0)
    {
        Q_ASSERT(authenticator);
        --counter;
        utilities::CredentialsRetriever cr;
        emit authNeedLogin(&cr);
        authenticator->setUser(cr.cred().login);
        authenticator->setPassword(cr.cred().password);
    }
}

} // namespace utilities