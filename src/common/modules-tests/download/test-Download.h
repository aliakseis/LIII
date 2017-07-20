#pragma once

#include <QObject>
#include "tcpserver.h"

class Test_Download: public QObject
{
	Q_OBJECT
private slots:
	void initTestCase();

	void testRegularDownload();
	void testRedirect();
	void testResume();
	void testResumeRedirect();

	void cleanupTestCase();

private:
	QScopedPointer<TcpServer> m_tcpServer;
};
