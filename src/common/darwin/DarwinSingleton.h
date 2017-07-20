#ifndef DARWINSINGLETON_H
#define DARWINSINGLETON_H

#include <QObject>
#include <QStringList>

class DarwinSingleton : public QObject
{
	Q_OBJECT

public:
	static DarwinSingleton& Instance();
	void on_preferencesClicked();
	void on_aboutClicked();
	void on_friendCWClicked(QString friendName);
	void on_addTorrent(QString url);
	void on_captureDevicesChanged(QString message);

signals:
	void showPreferences();
	void showAbout();
	void showFriendCW(QString friendName);
	void addTorrent(QStringList url);
	void captureDevicesChanged(QString& message);

private:
	DarwinSingleton();

};

#endif //DARWINSINGLETON_H