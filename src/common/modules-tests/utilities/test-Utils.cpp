#include "test-Utils.h"

#include <iterator>
#include "utilities/utils.h"

#include <QtTest/QtTest>
#include <QString>
#include "utilities/filesystem_utils.h"


QString dump(const QVariant& data)
{
	QString result = data.toString();
	if (result.isEmpty())
	{
		QDebug(&result) << data;
		result = result.trimmed();
	}

	return result;
}


/********************** Tests for functions from utils.h ****************************/
void Test_Utils::IsAsyncUrl_data()
{
	QTest::addColumn<QString>("string");
	QTest::addColumn<bool>("result");

	QTest::newRow("http") << "http://test.com" << true;
	QTest::newRow("https") << "https://test.com" << true;
	QTest::newRow("qrc") << "qrc://test.com" << true;

	QTest::newRow("HTTP") << "HTTP://TEST.COM" << true;
	QTest::newRow("HttpS") << "HttpS://test.COM" << true;
	QTest::newRow("QRC") << "QRC://TEST.COM" << true;

	QTest::newRow("begins from space http(s)") << " http://test.com" << true;
	QTest::newRow("begins from space qrc") << "  qrc://test.com" << true;

	QTest::newRow("without protocol") << " test.com" << false;
	QTest::newRow("begins qrc") << "  qrc_test.com" << false;

	QTest::newRow("empty string") << "" << false;
	QTest::newRow("string of spaces") << "  " << false;
}

#ifdef Q_OS_WIN
#include <windows.h>
#endif

void Test_Utils::GetOSName()
{
	QString sExpected;

#if defined(Q_OS_LINUX)
	sExpected = "linux";
#elif defined(Q_OS_MAC)
	sExpected = "mac";
#elif defined(Q_OS_WIN)
#if 0
#if defined(_WIN64)
	sExpected = "win64";// 64-bit programs run only on Win64
#elif defined(_WIN32)
	BOOL f64 = FALSE;
	sExpected = (IsWow64Process(GetCurrentProcess(), &f64) && f64 ? "win64" : "win32");
#else
	sExpected = "win32";// Win64 does not support Win16
#endif
#else
	sExpected = "win";
#endif


#else
	sExpected = "unknown";
#endif

	QCOMPARE(utilities::GetOSName(), sExpected);
}


void Test_Utils::GetFileName_data()
{
	QTest::addColumn<QString>("string");
	QTest::addColumn<QString>("result");

	QTest::newRow("file only") << "test.txt" << "test.txt";
	QTest::newRow("file with space only") << "test data.txt" << "test data.txt";
	QTest::newRow("empty string") << "" << "";
#ifdef Q_OS_WIN
	QTest::newRow("normal path") << "c:\\test.txt" << "test.txt";
	QTest::newRow("path with space1") << "c:\\test\\1 2.txt" << "1 2.txt";
	QTest::newRow("path with space2") << "c:\\test data\\1 2.txt" << "1 2.txt";
	QTest::newRow("directory only") << "c:\\test\\" << "c:\\test\\";
	QTest::newRow("many extensions") << "c:\\test\\data.test.ext" << "data.test.ext";
#else
	QTest::newRow("normal path") << "/tmp/test.txt" << "test.txt";
	QTest::newRow("path with space1") << "/test/1 2.txt" << "1 2.txt";
	QTest::newRow("path with space2") << "/test data/1 2.txt" << "1 2.txt";
	QTest::newRow("many extensions") << "/test/data.test.ext" << "data.test.ext";
#endif
}

void Test_Utils::GetFileName()
{
	QFETCH(QString, string);
	QFETCH(QString, result);

	QCOMPARE(utilities::GetFileName(string), result);
}

void Test_Utils::IsFolderName_data()
{
	QTest::addColumn<QString>("folder");
	QTest::addColumn<QString>("name");
	QTest::addColumn<bool>("result");

	QTest::newRow("folder & name are same case strings") << "test" << "test" << true;
	QTest::newRow("folder & name are different case strings") << "test" << "TEST" << true;

	QTest::newRow("1) folder ends with /") << "test/" << "test" << true;
	QTest::newRow("1) folder ends with \\") << "test\\" << "test" << true;

	QTest::newRow("1) test/test") << "test/test" << "test/test" << true;
#ifdef Q_OS_WIN
	QTest::newRow("1) test\\test") << "test\\test" << "test\\test" << true;
#endif

	QTest::newRow("2) test/test") << "test/test\\" << "test/test" << true;
#ifdef Q_OS_WIN
	QTest::newRow("2) test\\test") << "test\\test/" << "test\\test" << true;
#endif

	QTest::newRow("3) test/test") << "test/test" << "test" << false;
	QTest::newRow("3) test\\test") << "test\\test" << "test" << false;

	QTest::newRow("folder shorter then name") << "test" << "test test" << false;
	QTest::newRow("folder does not starts from name") << "test" << "data" << false;

	QTest::newRow("2) folder ends with /") << "test/" << "data" << false;
	QTest::newRow("2) folder ends with \\") << "test\\" << "data" << false;

	QTest::newRow("1) folder with / at the middle") << "test/data" << "test" << false;
	QTest::newRow("1) folder with \\  at the middle") << "test\\data" << "test" << false;

	QTest::newRow("2) folder with / at the middle") << "test/data/test" << "test" << false;
	QTest::newRow("2) folder with \\  at the middle") << "test\\data\\test" << "test" << false;


}

void Test_Utils::IsFolderName()
{
	QFETCH(QString, folder);
	QFETCH(QString, name);
	QFETCH(bool, result);

	QCOMPARE(utilities::IsFolderName(folder, name), result);
}


void Test_Utils::SizeToString_data()
{
	QTest::addColumn<quint64>("size");
	QTest::addColumn<int>("precision");
	QTest::addColumn<int>("fieldWidth");
	QTest::addColumn<QString>("result");

	QTest::newRow("0")		<< quint64(0)		<< 0 << 0 << "0 B";
	QTest::newRow("1")		<< quint64(1)		<< 0 << 0 << "1 B";
	QTest::newRow("1024")	<< quint64(1024)	<< 0 << 0 << "1 kB";
	QTest::newRow("1024*1024")	<< quint64(1024 * 1024)	<< 0 << 0 << "1 MB";
	QTest::newRow("1024*1024*1024")	<< quint64(1024 * 1024 * 1024)	<< 0 << 0 << "1 GB";

	QTest::newRow("1024")	<< quint64(1024)	<< 1 << 1 << "1.0 kB";
	QTest::newRow("1024")	<< quint64(1024)	<< 2 << 2 << "1.00 kB";
	QTest::newRow("1125")	<< quint64(1125)	<< 1 << 1 << "1.1 kB";

	QTest::newRow("1) 1024*1024-25")	<< quint64(1024 * 1024 - 25)	<< 2 << 1 << "1023.98 kB";
	QTest::newRow("2) 1024*1024-25")	<< quint64(1024 * 1024 - 25)	<< 1 << 1 << "1024.0 kB";

	QTest::newRow("1024*1024*1024")	<< quint64(1024 * 1024 * 1024)	<< 1 << 1 << "1.0 GB";
}

void Test_Utils::SizeToString()
{
	QFETCH(quint64, size);
	QFETCH(int, precision);
	QFETCH(int, fieldWidth);
	QFETCH(QString, result);

	QCOMPARE(utilities::SizeToString(size, precision, fieldWidth), result);
}

/********************** DECLARE TEST MAIN ****************************/
QTEST_MAIN(Test_Utils)
