#pragma once

#include <QString>

#import <Foundation/NSString.h>


inline NSString* nsStringFromQString(const QString& s)
{
	const char* utf8String = s.toUtf8().constData();
return [[NSString alloc] initWithUTF8String: utf8String];
}

inline QString nsStringToQString(const NSString* nsstr)
{
	NSRange range;
	range.location = 0;
	range.length = [nsstr length];
	QString result(range.length, QChar(0));

	unichar* chars = new unichar[range.length];
[nsstr getCharacters: chars range: range];
	QString osVer = QString::fromUtf16(chars, range.length);
	delete[] chars;
	return osVer;
}
