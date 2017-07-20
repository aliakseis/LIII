#pragma once

#include <QObject>


class Test_Resources
	: public QObject
{
	Q_OBJECT

public:
	Test_Resources() {}

private Q_SLOTS:
	void initTestCase();

	void test_icon();
	void test_version();
	void test_StringFileInfo();

	void cleanupTestCase();

private:
	QString filePath;
};
