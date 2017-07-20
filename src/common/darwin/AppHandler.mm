#include "AppHandler.h"
#include "AppDelegate.h"
#include "DarwinSingleton.h"
#include "NS2QtImpl.h"
//#include "QtSingleApplication/QtSingleApplication.h"

#include <QStringList>
#include <QWidget>
#include <QFileOpenEvent>
#include <QUrl>

#include <algorithm>



namespace Darwin
{
	AppDelegate *mainDelegate;
	
	void SetApplicationHandler(QWidget* handler)
	{
		[[NSAutoreleasePool alloc] init];
		mainDelegate = [[AppDelegate alloc] initWithHandler:handler];
		[[NSApplication sharedApplication] setDelegate:mainDelegate];
	}
    
	void setDockBadge(int badgeInt)
	{
		NSString * badgeString;
		if(badgeInt != 0)
		{
			badgeString = [NSString stringWithFormat:@"%i", badgeInt];
		}
		else
		{
			badgeString = @"";
		}
		[[NSApp dockTile] setBadgeLabel: badgeString];
		[badgeString release];
	}
    
	unsigned long long getVolumeFreeSpace(QString path)
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSString* volumePath = nsStringFromQString(path);
		NSDictionary* fileAttributes = [[NSFileManager defaultManager] fileSystemAttributesAtPath:volumePath];
		[volumePath release];
		return [[fileAttributes objectForKey:NSFileSystemFreeSize] longLongValue];
		[pool drain];
	}
    
	QString getOsVersion()
	{
		SInt32 major, minor, bugfix;
		Gestalt(gestaltSystemVersionMajor, &major);
		Gestalt(gestaltSystemVersionMinor, &minor);
		Gestalt(gestaltSystemVersionBugFix, &bugfix);
		
		NSString *systemVersion = [NSString stringWithFormat:@"%d.%d.%d",
								   major, minor, bugfix];
		return nsStringToQString(systemVersion);
	}
	
	void requestAttention()
	{
		[NSApp requestUserAttention: NSInformationalRequest];
	}
	
	void updateDockMenu(QStringList userCWTitleList)
	{
		NSMutableArray *menuItems = [mainDelegate dockMenuCWList];
		[menuItems removeAllObjects];
		std::for_each(userCWTitleList.begin(), userCWTitleList.end(),
					  [menuItems](QString title)
					  {
						  [menuItems addObject:nsStringFromQString(title)];
					  });
		[mainDelegate setDockMenuCWList:menuItems];
		
		[[[NSApplication sharedApplication] delegate] applicationDockMenu:[NSApplication sharedApplication]];
	}
	
	void setAsDefaultForExtension(QString extension)
	{
		NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
		NSString *fileExtension = nsStringFromQString(extension);
		NSString *UTIContentType = (NSString *)UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension,
																					 (CFStringRef)fileExtension,
																					 NULL);
		
		LSSetDefaultRoleHandlerForContentType((CFStringRef)UTIContentType, kLSRolesAll, (CFStringRef)bundleID);
	}
	
	void setAsDefaultForURI(QString URI)
	{
		NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
		NSString *URIScheme = nsStringFromQString(URI);
		LSSetDefaultHandlerForURLScheme((CFStringRef)URIScheme, (CFStringRef)bundleID);
	}
	
	bool isDefaultForExtension(QString extension)
	{
		NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
		
		NSString* UTIContentType = (NSString *)UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension,
																					 (CFStringRef)nsStringFromQString(extension),
																					 NULL);

		NSString *idForExtension = [(NSString *)LSCopyDefaultRoleHandlerForContentType((CFStringRef)UTIContentType,
																					  kLSRolesAll) autorelease];
		return ([idForExtension caseInsensitiveCompare:bundleID] == NSOrderedSame) ? true : false;
	}
	
	bool isDefaultForURI(QString URI)
	{
		NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
		
		NSString *idForURI = [(NSString *)LSCopyDefaultHandlerForURLScheme ((CFStringRef)nsStringFromQString(URI)) autorelease];
		return ([idForURI caseInsensitiveCompare:bundleID] == NSOrderedSame) ? true : false;
	}
	
	bool OpenEventFilter::eventFilter(QObject* obj, QEvent* event)
	{
		if (event->type() == QEvent::FileOpen)
		{
			QFileOpenEvent* fileEvent = static_cast<QFileOpenEvent*>(event);
			if (!fileEvent->url().isEmpty())
			{
				DarwinSingleton::Instance().on_addTorrent(fileEvent->url().toString());
			}
			
			return false;
		}
		else
		{
			// standard event processing
			return QObject::eventFilter(obj, event);
		}
	}
	
	void setOverallProgress(int progress)
	{
		[mainDelegate setProgress:[NSNumber numberWithInt:progress]];
	}
	
	void showNotification(QString title, QString message)
	{
		NSUserNotification *notification = [[NSUserNotification alloc] init];
		notification.title = nsStringFromQString(title);
		notification.informativeText = nsStringFromQString(message);
		notification.soundName = NSUserNotificationDefaultSoundName;
		
		[[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
	}
	
}  // namespace Darwin
