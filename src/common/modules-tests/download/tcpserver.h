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
	void incomingConnection(qintptr socketDescriptor) override
	{
		qDebug() << __FUNCTION__ << "socketDescriptor:" << socketDescriptor;
		TcpClient* tcpCl = new TcpClient(socketDescriptor);
		QThreadPool::globalInstance()->start(tcpCl);
	}
};
