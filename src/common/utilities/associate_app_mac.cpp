#include "associate_app.h"
#include "darwin/AppHandler.h"

#include <QString>

// TODO: implement

bool utilities::isDefaultTorrentApp()
{
	return Darwin::isDefaultForExtension(QString("torrent"));
}

void utilities::setDefaultTorrentApp(WId)
{
	Darwin::setAsDefaultForExtension(QString("torrent"));
}

void utilities::unsetDefaultTorrentApp(WId)
{

}

bool utilities::isDefaultMagnetApp()
{
    return Darwin::isDefaultForURI(QString("magnet"));
}

void utilities::setDefaultMagnetApp(WId)
{
    Darwin::setAsDefaultForURI(QString("magnet"));
}

void utilities::unsetDefaultMagnetApp(WId)
{

}

void runWithPrivileges()
{
}