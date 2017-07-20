#include "autorun_utils.h"

#include <QStringList>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <algorithm>

#ifdef Q_OS_DARWIN
#import <CoreServices/CoreServices.h>
#endif //Q_OS_MAC

namespace utilities
{
namespace
{
#ifdef Q_OS_WIN
const char runKey[] = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
#endif

#ifdef Q_OS_DARWIN

LSSharedFileListItemRef FindLoginItemForCurrentBundle(CFArrayRef currentLoginItems)
{
    CFURLRef mainBundleURL = CFBundleCopyBundleURL(CFBundleGetMainBundle());

    for (int i = 0, end = CFArrayGetCount(currentLoginItems); i < end; ++i)
    {
        //OSStatus status;
        LSSharedFileListItemRef item = (LSSharedFileListItemRef)CFArrayGetValueAtIndex(currentLoginItems, i);

        UInt32 resolutionFlags = kLSSharedFileListNoUserInteraction | kLSSharedFileListDoNotMountVolumes;
        CFURLRef url = NULL;
        OSStatus err = LSSharedFileListItemResolve(item, resolutionFlags, &url, NULL);

        if (err == noErr)
        {
            bool foundIt = CFEqual(url, mainBundleURL);
            CFRelease(url);

            if (foundIt)
            {
                CFRelease(mainBundleURL);
                return item;
            }
        }
    }

    CFRelease(mainBundleURL);
    return NULL;
}

#endif
}

// check if application is set to run with operating system
bool isAutorunEnabled()
{
#ifdef Q_OS_WIN

    return !QSettings(runKey, QSettings::NativeFormat).value(PROJECT_NAME).toString().isEmpty();

#elif defined(Q_OS_DARWIN)

    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);

    if (!loginItems)
    {
        return false;
    }

    UInt32 seed = 0U;
    CFArrayRef currentLoginItems = LSSharedFileListCopySnapshot(loginItems, &seed);
    LSSharedFileListItemRef existingItem = FindLoginItemForCurrentBundle(currentLoginItems);

    bool isAutoRun = existingItem != NULL;

    CFRelease(currentLoginItems);
    CFRelease(loginItems);

    return isAutoRun;

#endif
}

// register / unregister application to autostart with OS, according to runWithOS argument
// returns true is operation succeeded
bool setAutorun(bool runWithOS)
{
#if defined(Q_OS_WIN)
    QSettings runSettings(runKey, QSettings::NativeFormat);
    if (runWithOS)
    {
        runSettings.setValue(PROJECT_NAME, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + " -autorun");
    }
    else
    {
        runSettings.remove(PROJECT_NAME);
    }

    return runSettings.status() == QSettings::NoError;

#elif defined(Q_OS_DARWIN)

    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);

    if (!loginItems)
    {
        return false;
    }

    UInt32 seed = 0U;
    CFArrayRef currentLoginItems = LSSharedFileListCopySnapshot(loginItems, &seed);
    LSSharedFileListItemRef existingItem = FindLoginItemForCurrentBundle(currentLoginItems);

    if (runWithOS && (existingItem == NULL))
    {
        CFURLRef mainBundleURL = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        LSSharedFileListInsertItemURL(loginItems, kLSSharedFileListItemBeforeFirst, NULL, NULL, mainBundleURL, NULL, NULL);
        CFRelease(mainBundleURL);
    }
    else if (!runWithOS && (existingItem != NULL))
    {
        LSSharedFileListItemRemove(loginItems, existingItem);
    }

    CFRelease(currentLoginItems);
    CFRelease(loginItems);

    return true;
#endif
}

// checks if launched in autorun mode
bool isAutorunMode()
{
    const QStringList args = QApplication::arguments();
    QStringList::const_iterator autostartParam = std::find(args.constBegin(), args.constEnd(), "-autorun");
    return autostartParam != args.end();
}

}

