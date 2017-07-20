#pragma once

#include <QObject>

class QAuthenticator;
namespace utilities
{
class ICredentialsRetriever;
class AuthenticationHelper;
};

class Test_AuthenticationHelper: public QObject
{
	Q_OBJECT

	QAuthenticator* pAuth_;
	utilities::AuthenticationHelper* pHelper_;

private slots:
	void initTestCase();
	void init();

	void constructor();
	void getProxyAuthentication_simple();
	void getWebAuthentication_simple();

	void getProxyAuthentication_multi();
	void getWebAuthentication_multi();

	void cleanup();
	void cleanupTestCase();

public slots:
	void authNeedLogin(utilities::ICredentialsRetriever* icr);
};