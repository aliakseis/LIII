#include "test-Translation.h"

#include <iterator>
#include "utilities/translation.h"

#include <QtTest/QtTest>
#include <QString>



void Test_Translation::locationString_data()
{
	QTest::addColumn<QString>("file_name");
	QTest::addColumn<QString>("location_str");

	QTest::newRow("empty") << "" << "";

	QTest::newRow("LIII_en.qm") << "LIII_en.qm" << "en";
	QTest::newRow("c:/LIII_en.qm") << "c:/LIII_en.qm" << "en";
	QTest::newRow("C:/LIII_EN.QM") << "C:/LIII_EN.QM" << "en";

	QTest::newRow("LIII_en.ts") << "LIII_en.ts" << "";
	QTest::newRow("c:/LIII_en.ts") << "c:/LIII_en.ts" << "";

	QTest::newRow("LIII_en") << "LIII_en" << "";
	QTest::newRow("LIII_en.ts.qm") << "LIII_en.ts.qm" << "";
}

void Test_Translation::locationString()
{
	QFETCH(QString, file_name);
	QFETCH(QString, location_str);

	QCOMPARE(utilities::locationString(file_name), location_str);
}

/********************** DECLARE TEST MAIN ****************************/
QTEST_MAIN(Test_Translation)