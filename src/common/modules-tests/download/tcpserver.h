#pragma once

#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>
#include <QDebug>

#include "tcpclient.h"
////////////////////////////////////////////////////////////////////
class TcpServer : public QTcpServer
{
public:
	explicit TcpServer(QObject* parent = 0)
		: QTcpServer(parent)
	{
	}

protected:
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
	void incomingConnection(int socketDescriptor) override
#else
	void incomingConnection(qintptr socketDescriptor) override
#endif
	{
		qDebug() << __FUNCTION__ << "socketDescriptor:" << socketDescriptor;
		TcpClient* tcpCl = new TcpClient(socketDescriptor);
		QThreadPool::globalInstance()->start(tcpCl);
	}
};
