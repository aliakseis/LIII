#include "test-AuthenticationHelper.h"

#include <iterator>
#include <QtTest/QtTest>
#include <QString>
#include <QAuthenticator>

#include "utilities/credsretriever.h"
#include "utilities/authentication_helper.h"

void Test_AuthenticationHelper::initTestCase()
{

}

void Test_AuthenticationHelper::init()
{
	pAuth_ = new QAuthenticator;
	Q_ASSERT(pAuth_);

	pHelper_ = new utilities::AuthenticationHelper;
	QVERIFY(connect(pHelper_, SIGNAL(authNeedLogin(utilities::ICredentialsRetriever*)), this, SLOT(authNeedLogin(utilities::ICredentialsRetriever*))));
}

void Test_AuthenticationHelper::constructor()
{
	QVERIFY(pHelper_ != nullptr);
}

void Test_AuthenticationHelper::getProxyAuthentication_simple()
{
	pHelper_->getProxyAuthentication(pAuth_);
	QVERIFY(pAuth_->user() == "test-user");
	QVERIFY(pAuth_->password() == "test-password");
}

void Test_AuthenticationHelper::getWebAuthentication_simple()
{
	pHelper_->getWebAuthentication(pAuth_);
	QVERIFY(pAuth_->user() == "test-user");
	QVERIFY(pAuth_->password() == "test-password");
}

void Test_AuthenticationHelper::getProxyAuthentication_multi()
{
	// get authentication data 1st time (counter==1)
	pHelper_->getProxyAuthentication(pAuth_);
	QVERIFY(pAuth_->user() == "test-user");
	QVERIFY(pAuth_->password() == "test-password");

	// change authenticator data
	pAuth_->setUser("new-user");
	pAuth_->setPassword("new-password");

	// get authentication data next time(counter==0)
	pHelper_->getProxyAuthentication(pAuth_);
	QVERIFY(pAuth_->user() == "new-user");
	QVERIFY(pAuth_->password() == "new-password");
}

void Test_AuthenticationHelper::getWebAuthentication_multi()
{
	// get authentication data 1st time (counter==1)
	pHelper_->getWebAuthentication(pAuth_);
	QVERIFY(pAuth_->user() == "test-user");
	QVERIFY(pAuth_->password() == "test-password");

	// change authenticator data
	pAuth_->setUser("new-user");
	pAuth_->setPassword("new-password");

	// get authentication data next time(counter==0)
	pHelper_->getWebAuthentication(pAuth_);
	QVERIFY(pAuth_->user() == "new-user");
	QVERIFY(pAuth_->password() == "new-password");
}
void Test_AuthenticationHelper::cleanup()
{
	QVERIFY(disconnect(pHelper_, SIGNAL(authNeedLogin(utilities::ICredentialsRetriever*)), this, SLOT(authNeedLogin(utilities::ICredentialsRetriever*))));

	delete pHelper_;
	pHelper_ = nullptr;

	delete pAuth_;
	pAuth_ = nullptr;

}

void Test_AuthenticationHelper::cleanupTestCase()
{

}

void Test_AuthenticationHelper::authNeedLogin(utilities::ICredentialsRetriever* icr)
{
	utilities::Credential cred("test-host", "test-user", "test-password");
	icr->SetCredentials(cred);
}


/********************** DECLARE TEST MAIN ****************************/
QTEST_MAIN(Test_AuthenticationHelper)