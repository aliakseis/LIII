#include "test-Utils.h"

#include <iterator>
#include "utilities/utils.h"

#include <QtTest/QtTest>
#include <QString>
#include "utilities/filesystem_utils.h"


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


QTEST_MAIN(Test_Utils)
