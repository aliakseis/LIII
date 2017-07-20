#pragma once

#include <QObject>
class Test_Translation: public QObject
{
	Q_OBJECT
private slots:
	void locationString_data();
	void locationString();
};
