#include "darwin/DarwinSingleton.h"

DarwinSingleton::DarwinSingleton() : QObject()
{

}

DarwinSingleton& DarwinSingleton::Instance()
{
	static DarwinSingleton ds;
	return ds;
}

void DarwinSingleton::on_preferencesClicked()
{
	emit showPreferences();
}

void DarwinSingleton::on_aboutClicked()
{
	emit showAbout();
}

void DarwinSingleton::on_friendCWClicked(QString friendName)
{
	emit showFriendCW(friendName);
}

void DarwinSingleton::on_addTorrent(QString url)
{
	emit addTorrent(QStringList() << url);
}

void DarwinSingleton::on_captureDevicesChanged(QString message)
{
	emit captureDevicesChanged(message);
}
