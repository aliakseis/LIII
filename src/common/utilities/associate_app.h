#pragma once

#include <qwindowdefs.h>

namespace utilities
{

bool isDefaultTorrentApp();
void setDefaultTorrentApp(WId parent = NULL);
void unsetDefaultTorrentApp();

}
