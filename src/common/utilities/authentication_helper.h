#pragma once

#include <QObject>

class QAuthenticator;

namespace utilities
{

class ICredentialsRetriever;

class AuthenticationHelper    : public QObject
{
    Q_OBJECT

public:
    explicit AuthenticationHelper(int proxyChecks = 1, int networkChecks = 1, QObject* parent = nullptr);
    virtual ~AuthenticationHelper() {}

    virtual void getProxyAuthentication(QAuthenticator* authenticator);
    virtual void getWebAuthentication(QAuthenticator* authenticator);

Q_SIGNALS:
    void authNeedLogin(utilities::ICredentialsRetriever* icr);

protected:
	void getAuthentication(int& counter, QAuthenticator* authenticator);

private:
    int m_proxyCounter;
    int m_networkCounter;
};

} // namespace utilities