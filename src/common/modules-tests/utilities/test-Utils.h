#pragma once

#include <QObject>
class Test_Utils: public QObject
{
	Q_OBJECT
private slots:
	void IsAsyncUrl_data();

	void GetOSName();

	void GetFileName_data();
	void GetFileName();

	void IsFolderName_data();
	void IsFolderName();

	void SizeToString_data();
	void SizeToString();
};
