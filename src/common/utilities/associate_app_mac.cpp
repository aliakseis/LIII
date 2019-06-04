#include "associate_app.h"
#include "darwin/AppHandler.h"

#include <QString>

// TODO: implement

bool utilities::isDefaultTorrentApp()
{
	return Darwin::isDefaultForExtension(QString("torrent"));
}

void utilities::setDefaultTorrentApp(WId parent)
{
	Darwin::setAsDefaultForExtension(QString("torrent"));
}

void utilities::unsetDefaultTorrentApp()
{

}

bool utilities::isDefaultMagnetApp()
{
    return Darwin::isDefaultForURI(QString("magnet"));
}

void utilities::setDefaultMagnetApp(WId parent)
{
    Darwin::setAsDefaultForURI(QString("magnet"));
}

void utilities::unsetDefaultMagnetApp()
{

}

void runWithPrivileges()
{
}