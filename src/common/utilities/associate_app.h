#pragma once

#include <qwindowdefs.h>

namespace utilities
{

bool isDefaultTorrentApp();
void setDefaultTorrentApp(WId parent = {});
void unsetDefaultTorrentApp(WId parent = {});

bool isDefaultMagnetApp();
void setDefaultMagnetApp(WId parent = {});
void unsetDefaultMagnetApp(WId parent = {});

}
