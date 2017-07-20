#include "multimedia.h"

#include <QStringList>

#import <Foundation/Foundation.h>
#import <QTKit/QTKit.h>

namespace Darwin
{
	int getVideoDevicesCount()
	{
    	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    	NSArray *devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];
    	return [devices count];
    	[devices release];
    	[pool drain];
	}
	    
	int getAudioDevicesCount()
	{
    	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    	NSArray *devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeSound];
    	return [devices count];
    	[devices release];
    	[pool drain];
	}

}
