#include "AppDelegate.h"

#include <QWidget>
#import <QTKit/QTKit.h>

#include "QtSingleApplication/QtSingleApplication.h"
#include "NS2QtImpl.h"
#include "DarwinSingleton.h"


@implementation AppDelegate

NSMutableArray *menuItems;

- (id)initWithHandler:(QWidget*)handler
{		 
	if ((self = [super init]))
	{
		applicationHandler = handler;
		m_menu = [[[NSMenu alloc] init] retain];
		menuItems = [NSMutableArray array];
		[self initDockProgessBar];
		[[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
		
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(captureDevicesChanged:)
													 name:QTCaptureDeviceWasConnectedNotification
												   object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(captureDevicesChanged:)
													 name:QTCaptureDeviceWasDisconnectedNotification
												   object:nil];
	}

	return self;
}

- (void) initDockProgessBar
{
	docTile = [[NSApplication sharedApplication] dockTile];
	NSImageView *iv = [[NSImageView alloc] init];
	[iv setImage:[[NSApplication sharedApplication] applicationIconImage]];
	[docTile setContentView:iv];
	
	progressIndicator = [[NSProgressIndicator alloc]
						 initWithFrame:NSMakeRect(0.0f, 3.0f, docTile.size.width, 12.)];
	[progressIndicator setStyle:NSProgressIndicatorBarStyle];
	[progressIndicator setIndeterminate:NO];
	[iv addSubview:progressIndicator];
	
	[progressIndicator setBezeled:YES];
	[progressIndicator setMinValue:0];
	[progressIndicator setMaxValue:100];
	[progressIndicator release];
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
	return YES;
}


- (BOOL) application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	NSArray *allowedExtensions = [[NSArray alloc] initWithObjects:@"torrent", nil];
	NSMutableArray *allowedUTIForOpen = [[NSMutableArray alloc] init];
	
	for (NSString *fileExtension in allowedExtensions)
	{
		NSString *UTIContentType = (NSString *)UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension,
																					 (CFStringRef)fileExtension,
																					 NULL);
		[allowedUTIForOpen addObject:UTIContentType];
	}
	
	NSString *UTI = (NSString *)UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, (CFStringRef)[filename pathExtension], NULL);
	
	for (NSString *allowedUTI in allowedUTIForOpen)
	{
		if ([allowedUTI isEqualToString:UTI])
		{
			DarwinSingleton::Instance().on_addTorrent(nsStringToQString(filename));
			return YES;
		}
	}
	return NO;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)app hasVisibleWindows:(BOOL)flag
{
	if (applicationHandler)
	{
		applicationHandler->show();
	}
	return YES;
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	[m_menu removeAllItems];
	for (NSString *value in menuItems)
	{
		[m_menu addItem: [[NSMenuItem alloc] initWithTitle:value
													action:@selector(openFriendConversation:)
											 keyEquivalent:@""]];
	}
	if ([m_menu numberOfItems])
	{
		[m_menu addItem:[NSMenuItem separatorItem]];
	}
	
	NSString * preferencesAction = nsStringFromQString(QApplication::translate("CMainWindow", "Preferences"));
	NSString * aboutAction = nsStringFromQString(QApplication::translate("CMainWindow", "About"));
	
	[m_menu addItem:[[NSMenuItem alloc] initWithTitle:preferencesAction
											   action:@selector(showPreferences:)
										keyEquivalent:@""]];
	[m_menu addItem:[[NSMenuItem alloc] initWithTitle:aboutAction
											   action:@selector(showAbout:)
										keyEquivalent:@""]];
	
	return m_menu;
}

- (void)openFriendConversation: (id) sender
{
	NSString* nsFriendName = [sender title];
	DarwinSingleton::Instance().on_friendCWClicked(nsStringToQString(nsFriendName));
}

- (void) showPreferences: (id) sender
{
	DarwinSingleton::Instance().on_preferencesClicked();
}

- (void) showAbout: (id) sender
{
	DarwinSingleton::Instance().on_aboutClicked();
}

-(NSMutableArray *) dockMenuCWList
{
	return menuItems;
}

- (void) setDockMenuCWList: (NSMutableArray *) newMenuItems
{
	menuItems = newMenuItems;
}

- (void)setProgress:(NSNumber *)fraction
{
	if ( [fraction integerValue] >= [progressIndicator minValue]
		&& [fraction integerValue] < [progressIndicator maxValue])
	{
		[progressIndicator setDoubleValue:[fraction integerValue]];
		[progressIndicator setHidden:NO];
	}
	else
	{
		[progressIndicator setHidden:YES];
	}
	[docTile display];
}

- (void)captureDevicesChanged: (id) sender
{
	DarwinSingleton::Instance().on_captureDevicesChanged(nsStringToQString([sender name]));
}

@end
