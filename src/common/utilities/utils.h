#pragma once

#include <QList>
#include <QMap>
#include <QRegExp>
#ifdef Q_OS_MAC
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#endif
#ifdef Q_OS_WIN
#include <qwindowdefs.h>
#endif

#include "iparentadvice.h"

#include <utility>


#ifdef QT_NO_DEBUG
#define VERIFY(x) (x)
#else //QT_NO_DEBUG
#define VERIFY(x) Q_ASSERT(x)
#endif //QT_NO_DEBUG

#ifndef PROJECT_NAME
#define PROJECT_NAME "ABSTRACT_PROJECT"
#endif//PROJECT_NAME

#ifndef PROJECT_FULLNAME
#define PROJECT_FULLNAME "ABSTRACT_PROJECT_FULLNAME"
#endif//PROJECT_FULLNAME

#ifndef PROJECT_DOMAIN
#define    PROJECT_DOMAIN "abstract-project.com"
#endif//PROJECT_DOMAIN


class QString;
class QFont;
class QUrl;
class QObject;
#ifndef Q_OS_MAC
class QXmlStreamReader;
class QXmlStreamWriter;
#endif
class QVariant;
class QPainter;
class QMargins;
class QPixmap;
class QRect;
class QMainWindow;

typedef QMap<QString, QObject*> QObjectMap;


namespace utilities
{

void InitializeProjectDescription();
QFont GetAdaptedFont(int size, int additional_amount = 2);
QStringList ParseUrls(const QString& data);
bool DeserializeObject(QXmlStreamReader* stream, QObject* object, const QString& name = QString());
void SerializeObject(QXmlStreamWriter* stream, QObject* object, const QString& name);
QString SizeToString(quint64 size, int precision = 2, int fieldWidth = 0);
QString ProgressString(double progress);

// correct QString args handling
struct QStringConversionHelper : public QString
{
    using QString::number;

    template<typename T>
    static QString number(const T& value)
    {
        return value.toString();
    }
};

template<typename T>
inline
QString asString(const T& value)
{
    return QStringConversionHelper::number(value);
}


inline QString asString(const char* value)
{
    return QString(value);
}

inline QString asString(const wchar_t* value)
{
    return QString::fromWCharArray(value);
}

inline QString asString(const QString& value)
{
    return value;
}

QString multiArg(const QString& str, int numArgs, const QString* args);

// performs port bounds check, TCP and UDP availability;
// if returns false, reason holds text message
bool CheckPortAvailable(int targetPort, const char** reason);

#ifdef Q_OS_WIN
bool isAdminRights();
void runWithPrivileges(const wchar_t* arg, WId parent = NULL);
#endif

QMainWindow* getMainWindow();

} // namespace utilities
