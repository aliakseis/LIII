#pragma once

#include "qglobal.h"
#include "utilities/translation.h"

#ifndef PROJECT_NAME
#define PROJECT_NAME    "LIII"
#endif

#ifndef PROJECT_DOMAIN
#define    PROJECT_DOMAIN    "LIII.com";
#endif


namespace
{

const char PROJECT_ICON[]           =    ":/icon.ico";

const char MODEL_STATE_FILE_NAME[]  =    "modelState.xml";

#ifdef Q_OS_WIN32
const char EXPLORER_LABEL[]         = "explorer";
#else
const char EXPLORER_LABEL[]         = "open";
const char APPLESCRIPT_BIN[]        = "osascript";
#endif

// translations

namespace Tr = utilities::Tr;

Tr::Translation CLEANUP_TEXT                         = Tr::translate("MainWindow", "Are you sure you want to remove completed downloads?");

Tr::Translation DELETE_DOWNLOAD_TEXT                 = Tr::translate("MainWindow", "Are you sure you wish to delete this download?<br/>Deleting will stop the download and remove the<br style=\"font-size:20px\"/>item from the disk.");
Tr::Translation CANCEL_DOWNLOAD_TEXT                 = Tr::translate("MainWindow", "Are you sure you wish to cancel this download?<br/>Canceling will stop the download and remove the<br style=\"font-size:20px\"/>item from the list.");
Tr::Translation EXIT_TEXT                            = Tr::translate("MainWindow", "<span style=\"margin-bottom:14px;\">Are you sure you wish to exit %1?</span><br/>Video playback and download will be stopped when <br style=\"font-size:20px\"/>closing this window.");
Tr::Translation ASSOCIATE_TORRENT_TEXT               = Tr::translate("MainWindow", "Would you like to make %1 the default torrent application?");
Tr::Translation DUPLICATE_TORRENT_HEADER             = Tr::translate("MainWindow", "Torrent is already in the list");
Tr::Translation DUPLICATE_TORRENT_TEXT               = Tr::translate("MainWindow", "The Torrent you are trying to add is already in the list of Torrents.\nDo you want to load the trackers from it?");

Tr::Translation DOWNLOAD_TO_LABEL                    = Tr::translate("Preferences", "Download to:", "Download to dialog header");
Tr::Translation NO_LINKS_IN_CLIPBOARD                = Tr::translate("AddLinks", "No valid links were found in clipboard.<br/>Copy links and click 'Paste Link' again.", "No links in clipboard error message text");
Tr::Translation PAUSE_LABEL                          = Tr::translate("MainWindow", "Pause", "Pause label");
Tr::Translation RESET_WARNINGS_TEXT                  = Tr::translate("Preferences", "All warning dialogs you've once hidden will now appear again.\nAfter returning them you will still be able to hide them back.\nAre you sure you wish to continue?", "Reset warnings dialog text");
#ifdef Q_OS_WIN
Tr::Translation START_LIII_ON_STARTING_WINDOWS_LABEL = Tr::translate("Preferences", "Start %1 when Windows starts up", "start LIII on starting Windows label");
#endif

Tr::Translation START_LABEL                          = Tr::translate("MainWindow", "Start", "Start label");
Tr::Translation TORRENT_DETAILS_INFO                 = Tr::translate("MainWindow", "Torrent details");

Tr::Translation PASTE_LINKS_CTRLV                    = Tr::translate("MainWindow", "Paste links here to start downloading them (Ctrl+V)");
Tr::Translation TREEVIEW_UNKNOWN_SIZE                = Tr::translate("MainWindow", "Unknown");
Tr::Translation TREEVIEW_DOWNLOADING_STATUS          = Tr::translate("MainWindow", "Downloading");
Tr::Translation TREEVIEW_CONNECTING_STATUS           = Tr::translate("MainWindow", "Connecting...");
Tr::Translation TREEVIEW_QUEUED_STATUS               = Tr::translate("MainWindow", "Queued");
Tr::Translation TREEVIEW_PAUSED_STATUS               = Tr::translate("MainWindow", "Paused");
Tr::Translation TREEVIEW_STOPPED_STATUS              = Tr::translate("MainWindow", "Stopped");
Tr::Translation TREEVIEW_FAILED_STATUS               = Tr::translate("MainWindow", "Failed");
Tr::Translation TREEVIEW_COMPLETE_STATUS             = Tr::translate("MainWindow", "Complete");

Tr::Translation TREEVIEW_STALLED_STATUS              = Tr::translate("MainWindow", "Stalled");

Tr::Translation TREEVIEW_SEEDING_STATUS              = Tr::translate("MainWindow", "Seeding");
Tr::Translation TREEVIEW_STARTING_STATUS             = Tr::translate("MainWindow", "Starting");

Tr::Translation TREEVIEW_TITLE_HEADER                = Tr::translate("MainWindow", "Name");
Tr::Translation TREEVIEW_STATUS_HEADER               = Tr::translate("MainWindow", "Status");
Tr::Translation TREEVIEW_SPEED_HEADER                = Tr::translate("MainWindow", "Speed down");
Tr::Translation TREEVIEW_SPEED_UPLOAD_HEADER         = Tr::translate("MainWindow", "Speed up");
Tr::Translation TREEVIEW_PROGR_HEADER                = Tr::translate("MainWindow", "Progress");
Tr::Translation TREEVIEW_SIZE_HEADER                 = Tr::translate("MainWindow", "Size");
Tr::Translation TREEVIEW_SOURCE_HEADER               = Tr::translate("MainWindow", "Source");

Tr::Translation TREEVIEW_MENU_MOVEUP                 = Tr::translate("AddLinks",   "High Priority");
Tr::Translation TREEVIEW_MENU_MOVEDOWN               = Tr::translate("AddLinks",   "Low Priority");
Tr::Translation TREEVIEW_MENU_OPENFOLDER             = Tr::translate("MainWindow", "Open in folder");
Tr::Translation TREEVIEW_MENU_CANCEL                 = Tr::translate("MainWindow", "Cancel");
Tr::Translation TREEVIEW_MENU_REMOVE                 = Tr::translate("MainWindow", "Remove");

Tr::Translation ABOUT_TITLE                          = Tr::translate("MainWindow", "About %1");

Tr::Translation DONT_SHOW_THIS_AGAIN                 = Tr::translate("MainWindow", "Don't show this again");

} // namespace
