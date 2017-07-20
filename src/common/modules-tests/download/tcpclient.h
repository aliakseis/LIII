#pragma once

#include <QtNetwork>
#include <QRunnable>

#include "downloadtestcommon.h"

#include <QApplication>
#include <QtGlobal>
#include <QDebug>

#include <cstddef>

////////////////////////////////////////////////////////////////////
class TcpClient : public QRunnable
{
	std::ptrdiff_t m_socketDescriptor;

public:
	explicit TcpClient(std::ptrdiff_t socketDescriptor)
	{
		m_socketDescriptor = socketDescriptor;
	}

protected:
	void run() override
	{
		qDebug() << __FUNCTION__ << "socketDescriptor:" << m_socketDescriptor;

		QTcpSocket tcpSocket;

		if (!tcpSocket.setSocketDescriptor(m_socketDescriptor))
		{
			qCritical() << "Socket error: " << tcpSocket.error();
			return;
		}

		tcpSocket.waitForReadyRead();

		QByteArray requestBytes(tcpSocket.read(4 * 1024));
		if (requestBytes.isEmpty())
		{
			qWarning() << __FUNCTION__ << "empty request";
			tcpSocket.disconnectFromHost();
			return;
		}
		QTextStream in(requestBytes);
		QString request, path;
		in >> request >> path;

		QMap<QString, QVariant> params;

		{
			QStringList split = path.split('?');
			if (2 == split.size())
			{
				path = split[0];
				QByteArray arr(QByteArray::fromBase64(split[1].toLatin1()));
				if (!arr.isEmpty())
				{
					QDataStream ds(arr);
					ds >> params;
				}
			}
		}

		unsigned int rangeStart = 0;
		while (!in.atEnd())
		{
			QString clause;
			in >> clause;
			if (clause.startsWith("bytes="))
			{
				rangeStart = clause.mid(6).split('-')[0].toUInt();
				break;
			}
		}

		QByteArray block;

		bool isPartial = false;

		auto itRedirect = params.find("redirect");
		if (itRedirect != params.end())
		{
			QString responseHeader
				= QString("HTTP/1.1 302 Found\r\nLocation: http://localhost:%1/%2\r\n\r\n")
				  .arg(TEST_PORT)
				  .arg(itRedirect.value().value<QString>());
			{
				qDebug() << __FUNCTION__ << "response header:\n" << responseHeader;
				QTextStream out(&block, QIODevice::WriteOnly);
				out << responseHeader;
			}
		}
		else
		{
			QString responseHeader;
			if (rangeStart > 0)
			{
				responseHeader
					= QString(
						  "HTTP/1.1 206 Partial Content\r\n"
						  "Date: Mon, 05 May 2008 00:37:54 GMT\r\n"
						  "Accept-Ranges: bytes\r\n"
						  "Content-Length: %1\r\n"
						  "Content-Range: bytes %2-%3/%4\r\n"
						  "Content-Type: video/3gpp\r\n\r\n"
					  )
					  .arg(TEST_DATA_SIZE - rangeStart)
					  .arg(rangeStart)
					  .arg(TEST_DATA_SIZE - 1)
					  .arg(TEST_DATA_SIZE);
			}
			else
			{
				responseHeader
					= QString(
						  "HTTP/1.1 200 OK\r\n"
						  "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n"
						  "Content-Type: video/3gpp\r\n"
						  "Content-Length: %1\r\n\r\n"
					  )
					  .arg(TEST_DATA_SIZE);
			}
			{
				qDebug() << __FUNCTION__ << "response header:\n" << responseHeader;
				QTextStream out(&block, QIODevice::WriteOnly);
				out << responseHeader;
			}

			if (0 == request.compare("GET", Qt::CaseInsensitive))
			{
				unsigned int num = TEST_DATA_SIZE;
				auto itPartial = params.find("partial");
				if (itPartial != params.end())
				{
					num = itPartial.value().value<unsigned int>();
					isPartial = true;
				}

				unsigned long next = QApplication::applicationPid();
				for (unsigned int i = 0; i < num; ++i)
				{
					next = testRand(next);
					if (i < rangeStart)
					{
						continue;
					}
					block.append((const char*)(const void*) &next, 1);
				}
			}
		}

		tcpSocket.write(block);

		tcpSocket.waitForBytesWritten();

		if (!isPartial)
		{
			tcpSocket.disconnectFromHost();
		}
		if (tcpSocket.state() != QAbstractSocket::UnconnectedState)
		{
			tcpSocket.waitForDisconnected();
		}
	}

};
