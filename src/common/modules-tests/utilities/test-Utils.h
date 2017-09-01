#pragma once

#include <QObject>
class Test_Utils: public QObject
{
	Q_OBJECT
private slots:

	void GetFileName_data();
	void GetFileName();

	void SizeToString_data();
	void SizeToString();
};
