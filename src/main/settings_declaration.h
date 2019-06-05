#pragma once

#include <QSettings>

namespace app_settings {

const char LoggingEnabled[] = "LoggingEnabled";

const char ln[] = "ln";
const char ln_Default[] = "en";

const char TorrentsPortIsAuto[] = "TorrentsPortIsAuto";
const bool TorrentsPortIsAuto_Default = true;

const char TorrentsPort[] = "TorrentsPort";
const int TorrentsPort_Default = 6881;

const char IsTrafficUploadLimited[] = "IsTrafficUploadLimited";
const bool IsTrafficUploadLimited_Default = false;

const char TrafficUploadLimitKbs[] = "TrafficUploadLimitKbs";
const int TrafficUploadLimitKbs_Default = 10;

const char IsTrafficLimited[] = "IsTrafficLimited";
const bool IsTrafficLimited_Default = false;

const char TrafficLimitKbs[] = "TrafficLimitKbs";
const int TrafficLimitKbs_Default = 1000;

const char TorrentsSequentialDownload[] = "TorrentsSequentialDownload";
const bool TorrentsSequentialDownload_Default = true;

const char VideoFolder[] = "VideoFolder";

const char UnlimitedLabel[] = "UnlimitedLabel";
const bool UnlimitedLabel_Default = false;

const char MaximumNumberLoads[] = "MaximumNumberLoads";
const int MaximumNumberLoads_Default = 5;

const char ShowAddTorrentDialog[] = "ShowAddTorrentDialog";

const char ShowSysTrayNotifications[] = "ShowSysTrayNotifications";

const char ShowCancelWarning[] = "ShowCancelWarning";

const char ShowCleanupWarning[] = "ShowCleanupWarning";

const char ShowExitWarning[] = "ShowExitWarning";

const char ShowAssociateTorrentDialog[] = "ShowAssociateTorrentDialog";

const char ShowAssociateMagnetDialog[] = "ShowAssociateMagnetDialog";

const char ShowSysTrayNotificationOnHide[] = "ShowSysTrayNotificationOnHide";

const char DeleteToRecycleBin[] = "DeleteToRecycleBin";
const bool DeleteToRecycleBin_Default = true;

const char TorrentDetailsAutoRefreshPeers[] = "TorrentDetailsAutoRefreshPeers";
const bool TorrentDetailsAutoRefreshPeers_Default = true;

const char UseProxy[] = "UseProxy";
const bool UseProxy_Default = false;

const char ProxyAddress[] = "ProxyAddress";

const char ProxyPort[] = "ProxyPort";
} // namespace app_settings
