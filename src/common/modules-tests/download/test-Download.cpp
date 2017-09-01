#include "test-Download.h"

#include "../AutoTest.h"

#include "tcpserver.h"

#include "download/downloader.h"

#include "downloadtestcommon.h"

#include <stdlib.h>

#include <QEventLoop>
#include <QDebug>


/********************** DECLARE_TEST LIST ****************************/
QTEST_MAIN(Test_Download)


typedef download::Downloader<download::speed_readable_tag, false> DownloaderType;


QByteArray getFileContents(const QString& fileName)
{
	QByteArray ba;
	QFile f(fileName);
	if (f.open(QIODevice::ReadOnly))
	{
		ba = f.readAll();
		f.close();
	}

	return ba;
}

bool isContentsValid(const QByteArray& ba)
{
	if (TEST_DATA_SIZE != ba.size())
	{
		return false;
	}

	const char* data = ba.data();
	unsigned long next = QApplication::applicationPid();
	for (unsigned int i = 0; i < TEST_DATA_SIZE; ++i)
	{
		next = testRand(next);
		if (data[i] != *((const char*)(const void*) &next))
		{
			return false;
		}
	}

	return true;
}


#define LIST_STATES DECL_STATE(None) DECL_STATE(Running) DECL_STATE(Finished) DECL_STATE(Error)

class TestDownloaderClient
	: protected QEventLoop
	, public download::DownloaderObserverInterface
{
public:

#define DECL_STATE(state) state,
	enum State { LIST_STATES };
#undef DECL_STATE

private:
	// DownloaderObserverInterface interface
	void onProgress(qint64 bytes_downloaded) override
	{
		m_bytesDownloaded += bytes_downloaded;
		if (m_partial && m_partial >= m_bytesDownloaded)
		{
			m_state = Finished;
			processEvents();
			exit(0);
		}
	}
	void onSpeed(qint64 bytes_per_second) override
	{
	}
	void onFinished() override
	{
		m_state = Finished;
		processEvents();
		exit(0);
	}
	void onFileCreated(const QString& filename) override
	{
		m_fileCreated = filename;
	}
	void onError(utilities::ErrorCode::ERROR_CODES code, const QString& err) override
	{
		qDebug() << __FUNCTION__ << "code:" << code << "description:" << err;
		m_state = Error;
		processEvents();
		exit(0);
	}
	void onFileToBeReleased(const QString& filename) override
	{
		m_fileReleased = filename;
	}
	void onNeedLogin(utilities::ICredentialsRetriever* retriever) override {}
	void onReplyInvalidated() override {}
    void onStart(const QByteArray&) override {}

	QUrl setupUrl(const QString& fileName, const QMap<QString, QVariant>& params)
	{
		QUrl url;
		url.setScheme("http");
		url.setHost("localhost");
		url.setPort(TEST_PORT);
		url.setPath('/' + fileName);
		if (!params.isEmpty())
		{
			QByteArray arr;
			{
				QDataStream ds(&arr, QIODevice::WriteOnly);
				ds << params;
			}
			arr = arr.toBase64();
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
			url.setEncodedQuery(arr);
#else
			url.setQuery(arr);
#endif
		}
		Q_ASSERT(url.isValid());
		return url;
	}


	State m_state;

public:
	TestDownloaderClient() : m_state(None), m_partial(0), m_bytesDownloaded(0)
	{
		downloader.setDownloadNamePolicy(DownloaderType::kReplaceFile);
		downloader.setObserver(this);
		QVERIFY(downloader.setDestinationPath(QDir(QDir::tempPath()).absolutePath()));
	}


	typedef QMap<QString, QVariant> VarMap;
	void start(const QString& fileName, const VarMap& params = VarMap())
	{
		m_bytesDownloaded = 0;
		downloader.Start(setupUrl(fileName, params), &networkAccessManager, fileName);

		run();
	}

	void resume(const QString& fileName, const VarMap& params = VarMap())
	{
		downloader.Resume(setupUrl(fileName, params), &networkAccessManager, fileName);

		run();
	}

	void run()
	{
		m_state = Running;
		exec();
	}

	State state() const { return m_state; }

	QNetworkAccessManager networkAccessManager;
	DownloaderType downloader;
	unsigned int m_partial;
	qint64 m_bytesDownloaded;

	QString m_fileCreated;
	QString m_fileReleased;
};


namespace QTest
{

template<>
inline char* toString<TestDownloaderClient::State>(const TestDownloaderClient::State& state)
{
	switch (state)
	{
#define DECL_STATE(s) case TestDownloaderClient::s: return qstrdup(#s);
		LIST_STATES
#undef DECL_STATE
	default: return 0;
	}

	return 0;
}

}



void Test_Download::initTestCase()
{
	m_tcpServer.reset(new TcpServer());
	if (!m_tcpServer->listen(QHostAddress::LocalHost, TEST_PORT))
	{
		exit(EXIT_FAILURE);
	}
}

void Test_Download::cleanupTestCase()
{
	m_tcpServer.reset(nullptr);
}

void Test_Download::testRegularDownload()
{
	TestDownloaderClient testDownloaderClient;

	testDownloaderClient.start(FILE_NAME_REGULAR);

	QCOMPARE(testDownloaderClient.state(), TestDownloaderClient::Finished);
	QCOMPARE(testDownloaderClient.downloader.resultFileName(), testDownloaderClient.m_fileCreated);
	QVERIFY(testDownloaderClient.m_fileReleased.isEmpty());

	QString fileName(testDownloaderClient.downloader.resultFileName());
	QVERIFY(isContentsValid(getFileContents(fileName)));

	QVERIFY(utilities::DeleteFileWithWaiting(fileName));
}

void Test_Download::testRedirect()
{
	TestDownloaderClient testDownloaderClient;

	QMap<QString, QVariant> params;
	params["redirect"] = FILE_NAME_REGULAR;
	testDownloaderClient.start(FILE_NAME_REGULAR, params);

	QCOMPARE(testDownloaderClient.state(), TestDownloaderClient::Finished);
	QCOMPARE(testDownloaderClient.downloader.resultFileName(), testDownloaderClient.m_fileCreated);
	//QCOMPARE(testDownloaderClient.downloader.resultFileName(), testDownloaderClient.m_fileReleased);

	QString fileName(testDownloaderClient.downloader.resultFileName());
	QVERIFY(isContentsValid(getFileContents(fileName)));

	QVERIFY(utilities::DeleteFileWithWaiting(fileName));
}

void Test_Download::testResume()
{
	TestDownloaderClient testDownloaderClient;

	QMap<QString, QVariant> params;
	params["partial"] = testDownloaderClient.m_partial = PARTIAL_DATA_SIZE;
	testDownloaderClient.start(FILE_NAME_REGULAR, params);
	testDownloaderClient.m_partial = 0;

	QCOMPARE(testDownloaderClient.state(), TestDownloaderClient::Finished);
	QCOMPARE(testDownloaderClient.downloader.resultFileName(), testDownloaderClient.m_fileCreated);
	QVERIFY(testDownloaderClient.m_fileReleased.isEmpty());
	testDownloaderClient.m_fileCreated.clear();

	testDownloaderClient.downloader.Pause();
	testDownloaderClient.resume(FILE_NAME_REGULAR);

	QCOMPARE(testDownloaderClient.state(), TestDownloaderClient::Finished);
	QCOMPARE(testDownloaderClient.downloader.resultFileName(), testDownloaderClient.m_fileCreated);
	QVERIFY(testDownloaderClient.m_fileReleased.isEmpty());

	QString fileName(testDownloaderClient.downloader.resultFileName());
	QVERIFY(isContentsValid(getFileContents(fileName)));

	QVERIFY(utilities::DeleteFileWithWaiting(fileName));
}

void Test_Download::testResumeRedirect()
{
	TestDownloaderClient testDownloaderClient;

	{
		QMap<QString, QVariant> params;
		params["partial"] = testDownloaderClient.m_partial = PARTIAL_DATA_SIZE;
		testDownloaderClient.start(FILE_NAME_REGULAR, params);
		testDownloaderClient.m_partial = 0;
	}

	QCOMPARE(testDownloaderClient.state(), TestDownloaderClient::Finished);
	QCOMPARE(testDownloaderClient.downloader.resultFileName(), testDownloaderClient.m_fileCreated);
	QVERIFY(testDownloaderClient.m_fileReleased.isEmpty());
	testDownloaderClient.m_fileCreated.clear();

	testDownloaderClient.downloader.Pause();

	{
		QMap<QString, QVariant> params;
		params["redirect"] = FILE_NAME_REGULAR;
		testDownloaderClient.resume(FILE_NAME_REGULAR, params);
	}

	QCOMPARE(testDownloaderClient.state(), TestDownloaderClient::Finished);
	QCOMPARE(testDownloaderClient.downloader.resultFileName(), testDownloaderClient.m_fileCreated);
	QVERIFY(testDownloaderClient.m_fileReleased.isEmpty());

	QString fileName(testDownloaderClient.downloader.resultFileName());
	QVERIFY(isContentsValid(getFileContents(fileName)));

	QVERIFY(utilities::DeleteFileWithWaiting(fileName));
}
