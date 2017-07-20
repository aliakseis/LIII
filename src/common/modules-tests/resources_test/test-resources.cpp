#include "test-resources.h"
#include "../AutoTest.h"
#include <QtTest>
#include "version.hxx"


#ifdef Q_OS_WIN

#include <Windows.h>
#include <shellapi.h>
#include <utility>
#include "utilities/utils.h"
#include <QString>

namespace fromMSDN
{
// could not found where to include these structures, so copied them from msdn
typedef struct
{
	WORD  wLength;
	WORD  wValueLength;
	WORD  wType;
	WCHAR szKey;
	WORD  Padding;
	DWORD Value;
} Var;

typedef struct
{
	WORD  wLength;
	WORD  wValueLength;
	WORD  wType;
	WCHAR szKey;
	WORD  Padding;
	WORD  Value;
} String;

typedef struct
{
	WORD   wLength;
	WORD   wValueLength;
	WORD   wType;
	WCHAR  szKey;
	WORD   Padding;
	String Children;
} StringTable;
}

#include <QByteArray>
#include <QString>


#pragma comment(lib,"Version.lib")

void Test_Resources::initTestCase()
{
	filePath = QFileInfo(qApp->applicationDirPath(), PROJECT_NAME ".exe").absoluteFilePath();
	QVERIFY(QFile::exists(filePath));
}

void Test_Resources::test_icon()
{
	HICON icon = 0;
	UINT res = ExtractIconExW((LPCWSTR)filePath.utf16(), -1, NULL, NULL, 1); // gets number of icons
	QVERIFY2(res > 0, "no icon found in executable");
	if (res > 0)
	{
		res = ExtractIconExW((LPCWSTR)filePath.utf16(), 0, &icon, NULL, 1);

		QVERIFY(res == 1); // icon should be extracted

		QVERIFY(icon); // icon should be extracted

		// TODO: compare icon with main app icon

		if (res && icon)
		{
			DestroyIcon(icon);
		}
	}
}

void Test_Resources::test_version()
{
	DWORD size = GetFileVersionInfoSizeW((LPCWSTR)filePath.utf16(), NULL);

	QVERIFY(size > 0);
	if (size > 0)
	{
		QByteArray lpdata(size, '\0');
		BOOL res = GetFileVersionInfoW((LPCWSTR)filePath.utf16(), NULL, size, lpdata.data());
		QVERIFY(res != FALSE);
		if (res)
		{
			VS_FIXEDFILEINFO* fileinfo = 0;
			UINT infoSize = 0;
			res = VerQueryValueW(lpdata.data(), L"\\", (LPVOID*)&fileinfo, &infoSize);
			QVERIFY(res != FALSE);
			QVERIFY(fileinfo->dwFileVersionMS == fileinfo->dwProductVersionMS);
			QVERIFY(fileinfo->dwFileVersionLS == fileinfo->dwProductVersionLS);
			int version[] =
			{
				fileinfo->dwFileVersionMS >> 16,
				fileinfo->dwFileVersionMS & 0xffff,
				fileinfo->dwFileVersionLS >> 16,
				fileinfo->dwFileVersionLS & 0xffff
			};
			QString fileVer = QString("%1.%2.%3.%4").arg(version[0]).arg(version[1]).arg(version[2]).arg(version[3]);
			QCOMPARE(fileVer, QString(PROJECT_VERSION));
		}
	}
}

void Test_Resources::test_StringFileInfo()
{
	DWORD size = GetFileVersionInfoSizeW((LPCWSTR)filePath.utf16(), NULL);

	QVERIFY(size > 0);
	if (size > 0)
	{
		QByteArray lpdata(size, '\0');
		BOOL res = GetFileVersionInfoW((LPCWSTR)filePath.utf16(), NULL, size, lpdata.data());
		QVERIFY(res != FALSE);
		if (res)
		{
			struct LANGANDCODEPAGE
			{
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;
			UINT cbTranslate = 0;
			res = VerQueryValueW(lpdata.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate);
			QVERIFY(res != FALSE);
			QVERIFY(cbTranslate > 0);
			const uint numLangs = cbTranslate / sizeof(struct LANGANDCODEPAGE);

			std::pair<const QString, const QString> checkParam[] =
			{
				std::make_pair<const QString, const QString>("CompanyName"			,	PROJECT_COMPANYNAME),
				std::make_pair<const QString, const QString>("FileDescription"		,	PROJECT_FULLNAME),
				std::make_pair<const QString, const QString>("FileVersion"			,	PROJECT_VERSION),
				std::make_pair<const QString, const QString>("LegalCopyright"		,	"Copyright (C) 2017 " PROJECT_COMPANYNAME " All Rights Reserved."),
				std::make_pair<const QString, const QString>("ProductName"			,	PROJECT_FULLNAME),
				std::make_pair<const QString, const QString>("ProductVersion"		,	PROJECT_VERSION),
				std::make_pair<const QString, const QString>("OriginalFilename"		,	PROJECT_NAME ".exe"),
				std::make_pair<const QString, const QString>("InternalName"			,	PROJECT_NAME)
			};

			std::for_each(std::begin(checkParam), std::end(checkParam), [ = ](const std::pair<const QString, const QString>& namval)
			{
				for (uint i = 0; i < numLangs; ++i) // for each language
				{
					const QString subBlockName =
						QString("\\StringFileInfo\\%1%2\\%3")
						.arg(lpTranslate[i].wLanguage, 4, 16, QChar('0'))
						.arg(lpTranslate[i].wCodePage, 4, 16, QChar('0'))
						.arg(namval.first);

					const ushort* val = 0;
					uint strLen = 0;
					auto res = VerQueryValueW(lpdata.data(), (LPCWSTR)subBlockName.utf16(), (LPVOID*)&val, &strLen);
					QVERIFY(res != FALSE && strLen > 0);
					const QString extractedValue = QString::fromUtf16(val);
					QCOMPARE(extractedValue, namval.second);
				}
			});
		}
	}
}

void Test_Resources::cleanupTestCase()
{

}

#else

void Test_Resources::initTestCase()
{

}

void Test_Resources::test_icon()
{

}

void Test_Resources::test_StringFileInfo()
{

}

void Test_Resources::test_version()
{

}

void Test_Resources::test_signature()
{

}

void Test_Resources::cleanupTestCase()
{

}

#endif

QTEST_MAIN(Test_Resources);

