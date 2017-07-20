#ifndef APP_DELEGATE_H
#define APP_DELEGATE_H

#import <AppKit/NSApplication.h>
#import <Foundation/Foundation.h>
#import <AppKit/NSProgressIndicator.h>

class QWidget;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
{
	QWidget* applicationHandler;
	NSMenu* m_menu;
	NSProgressIndicator* progressIndicator;
	NSDockTile* docTile;
}

- (NSMutableArray*) dockMenuCWList;
- (void) setDockMenuCWList: (NSMutableArray*) newMenuItems;

- (id)initWithHandler: (QWidget*)handler;
- (void)setProgress: (NSNumber*)fraction;
- (void)initDockProgessBar;

#pragma mark NSApplicationDelegate methods

- (BOOL)applicationShouldHandleReopen: (NSApplication*)app hasVisibleWindows: (BOOL)flag;
- (BOOL)application: (NSApplication*)theApplication openFile: (NSString*)filename;
- (BOOL)userNotificationCenter: (NSUserNotificationCenter*)center shouldPresentNotification: (NSUserNotification*)notification;

@end


#endif // APP_DELEGATE_H