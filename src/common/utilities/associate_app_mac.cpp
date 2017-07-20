#include "associate_app.h"
#include "darwin/AppHandler.h"

#include <QString>

// TODO: implement

bool utilities::isDefaultTorrentApp()
{
	return Darwin::isDefaultForExtension(QString("torrent")) && Darwin::isDefaultForURI(QString("magnet"));
}

void utilities::setDefaultTorrentApp(WId parent)
{
	Darwin::setAsDefaultForExtension(QString("torrent"));
	Darwin::setAsDefaultForURI(QString("magnet"));
}

void utilities::unsetDefaultTorrentApp()
{

}

void runWithPrivileges()
{
}