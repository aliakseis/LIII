#include "utils.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#include <winsock2.h>
#include <winnt.h>
#include <shellapi.h>
#elif defined(Q_OS_LINUX)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET_ERROR (-1)
#elif defined(Q_OS_DARWIN)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include "darwin/AppHandler.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET_ERROR (-1)
#endif

#include "modelserializer.h"
#include "modeldeserializer.h"

#include <algorithm>
#include <iterator>
#include <QObject>
#include <QStringList>
#include <QDir>
#include <QFont>
#include <QByteArray>
#include <QUrl>
#include <QMetaProperty>
#include <QDebug>
#include <QCoreApplication>
#include <QApplication>
#include <QMainWindow>
#include <set>
#include <map>


namespace utilities
{

const char kValidIPAddressRegex[] =
    "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";
const char kValidHostnameRegex[] = // modified to have at least one '.'
    "^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)+([A-Za-z]|[A-Za-z][A-Za-z0-9\\-]*[A-Za-z0-9])$";
const char* kURLPrefixes[] = {"http://", "https://", "vidxden://1", "ftp://"};
const int kOffsetPastSeparator[] = {2, 2, 3, 2};

template <typename T, size_t N>
char(&ArraySizeHelper(T(&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

QStringList ParseUrls(const QString& data)
{
    QStringList list = data.split(QRegExp("[\\s\\\"\\n]+"), QString::SkipEmptyParts);
    QStringList res;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        QString t = it->trimmed();
        if (t.isEmpty())
        {
            continue;
        }

        if (QRegExp("^[a-z]:[\\\\/]|^/|^~/|^file:///|^magnet:", Qt::CaseInsensitive).indexIn(t) > -1)
        {
            res << t;
            continue;
        }

        QUrl url;

        int offset = t.indexOf("//");
        if (offset > 0)
        {
            // fix for urls that have few starting symbols lost
            static_assert(arraysize(kURLPrefixes) == arraysize(kOffsetPastSeparator),
                          "Sizes of kURLPrefixes and kOffsetPastSeparator should be the same");
            for (size_t i = 0; i < arraysize(kURLPrefixes); ++i)
            {
                int start = offset + kOffsetPastSeparator[i];
                QString protocol_prefix = t.left(start);
                QString prefix(kURLPrefixes[i]);
                if (prefix.endsWith(protocol_prefix, Qt::CaseInsensitive))
                {
                    url.setUrl(prefix + t.mid(start).remove('"'));
                    break;
                }
            }
        }
        else
        {
            // Assume that http:// is default
            if (0 == offset)
            {
                t = t.mid(2);
            }
            else if (t.startsWith('/'))
            {
                t = t.mid(1);
            }

            url.setUrl("http://" + t);
            QString host = url.host();
            if (!host.contains(QRegExp(kValidIPAddressRegex)) &&
                    !host.contains(QRegExp(kValidHostnameRegex)))
            {
                continue;
            }
            if (url.path().isEmpty())
            {
                continue;
            }
        }
        if (url.isValid())
        {
            res << url.toString();
        }
    }
    return res;
}

void InitializeProjectDescription()
{
    static struct Initializer
    {
        Initializer()
        {
            QCoreApplication::setApplicationVersion(BRAND_VERSION);
            QCoreApplication::setApplicationName(PROJECT_NAME);
            QCoreApplication::setOrganizationName(PROJECT_NAME);
            QCoreApplication::setOrganizationDomain(PROJECT_DOMAIN);
        }
    } initializer;
}

QFont GetAdaptedFont(int size, int additional_amount)
{
    Q_UNUSED(additional_amount)
#ifdef Q_OS_DARWIN
    QFont f("Lucida Grande");
    f.setPixelSize(size + additional_amount);
    return f;
#else
    const float koef = 4 / 3.f;
    QFont f("Segoe UI");
    f.setPixelSize(size * koef);
    return f;
#endif
}

///////////////////////////////////////////////////////////////////////////////


bool DeserializeObject(QXmlStreamReader* stream, QObject* object, const QString& name)
{
    Q_ASSERT(stream);
    ModelDeserializer deserializer(*stream);
    return deserializer.deserialize(object, name);
}

void SerializeObject(QXmlStreamWriter* stream, QObject* object, const QString& name)
{
    Q_ASSERT(stream);
    ModelSerializer serializer(*stream);
    serializer.serialize(object, name);
}


///////////////////////////////////////////////////////////////////////////////

QString SizeToString(quint64 size, int precision, int fieldWidth)
{
    const unsigned int Kbytes_limit = 1 << 10; //1 Kb
    const unsigned int Mbytes_limit = 1 << 20; //1 Mb
    const unsigned int Gbytes_limit = 1 << 30; //1 Gb

    if (size < Kbytes_limit)
    {
        return QStringLiteral("%1 B").arg(size);
    }
    else if (size < Mbytes_limit)
    {
        const double sizef = size / static_cast<double>(Kbytes_limit);
        return QStringLiteral("%1 kB").arg(sizef, fieldWidth, 'f', precision);
    }
    else if (size < Gbytes_limit)
    {
        const double sizef = size / static_cast<double>(Mbytes_limit);
        return QStringLiteral("%1 MB").arg(sizef, fieldWidth, 'f', precision);
    }

    const double sizef = size / static_cast<double>(Gbytes_limit);
    return QStringLiteral("%1 GB").arg(sizef, fieldWidth, 'f', precision);
}


QString secondsToString(int seconds)
{
    int s = seconds % 60;
    int m = seconds / 60;
    int h = seconds / 3600;
    m = m - h * 60;

    if (h > 0)
    {
        return QStringLiteral("%3:%2:%1")
            .arg(s, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(h, 2, 10, QChar('0'));
    }

    return QStringLiteral("%2:%1")
        .arg(s, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'));
}

// shamelessly stolen from qstring.cpp
int getEscape(const QChar* uc, int* pos, int len, int maxNumber = 999)
{
    int i = *pos;
    ++i;
    if (i < len && uc[i] == QLatin1Char('L'))
    {
        ++i;
    }
    if (i < len)
    {
        int escape = uc[i].unicode() - '0';
        if (uint(escape) >= 10U)
        {
            return -1;
        }
        ++i;
        while (i < len)
        {
            int digit = uc[i].unicode() - '0';
            if (uint(digit) >= 10U)
            {
                break;
            }
            escape = (escape * 10) + digit;
            ++i;
        }
        if (escape <= maxNumber)
        {
            *pos = i;
            return escape;
        }
    }
    return -1;
}

QString multiArg(const QString& str, int numArgs, const QString* args)
{
    QString result;
    QMap<int, int> numbersUsed;
    const QChar* uc = str.constData();
    const int len = str.size();
    const int end = len - 1;
    int lastNumber = -1;
    int i = 0;

    // populate the numbersUsed map with the %n's that actually occur in the string
    while (i < end)
    {
        if (uc[i] == QLatin1Char('%'))
        {
            int number = getEscape(uc, &i, len);
            if (number != -1)
            {
                numbersUsed.insert(number, -1);
                continue;
            }
        }
        ++i;
    }

    // assign an argument number to each of the %n's
    QMap<int, int>::iterator j = numbersUsed.begin();
    QMap<int, int>::iterator jend = numbersUsed.end();
    int arg = 0;
    while (j != jend && arg < numArgs)
    {
        *j = arg++;
        lastNumber = j.key();
        ++j;
    }

    // sanity
    if (numArgs > arg)
    {
        qWarning("QString::arg: %d argument(s) missing in %s", numArgs - arg, str.toLocal8Bit().data());
        numArgs = arg;
    }

    i = 0;
    while (i < len)
    {
        if (uc[i] == QLatin1Char('%') && i != end)
        {
            int number = getEscape(uc, &i, len, lastNumber);
            int arg = numbersUsed[number];
            if (number != -1 && arg != -1)
            {
                result += args[arg];
                continue;
            }
        }
        result += uc[i++];
    }
    return result;
}

bool isTCPportAvalible(short int dwPort)
{
    struct sockaddr_in client;
    int sock;

    client.sin_family      = AF_INET;
    client.sin_port        = htons(dwPort);
    client.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock = (int) socket(AF_INET, SOCK_STREAM, 0);
    int result = bind(sock, (struct sockaddr*) &client, sizeof(client));
#ifdef Q_OS_WIN
    closesocket(sock);
#else
    close(sock);
#endif
    return result != SOCKET_ERROR;
}

bool isUDPportAvalible(short int dwPort)
{
    struct sockaddr_in client;
    int sock;

    client.sin_family      = AF_INET;
    client.sin_port        = htons(dwPort);
    client.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock = (int) socket(AF_INET, SOCK_DGRAM, 0);
    int result = bind(sock, (struct sockaddr*) &client, sizeof(sockaddr_in));
#ifdef Q_OS_WIN
    closesocket(sock);
#else
    close(sock);
#endif
    return result != SOCKET_ERROR;
}

bool CheckPortAvailable(int targetPort, const char** reason)
{
    if (targetPort < 1 || targetPort > 0xffff)
    {
        if (reason)
        {
            *reason = "Port is out bounds.";
        }
        return false;
    }

    if (!isTCPportAvalible(targetPort))
    {
        if (reason)
        {
            *reason = "This TCP port has already used";
        }
        return false;
    }

    if (!isUDPportAvalible(targetPort))
    {
        if (reason)
        {
            *reason = "This UDP port has already used";
        }
        return false;
    }

    return true;
}

#ifdef Q_OS_WIN
bool isAdminRights()
{
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup) != FALSE)
    {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin))
        {
            isAdmin = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }
    return isAdmin != FALSE;
}

void runWithPrivileges(const wchar_t* arg, WId parent)
{
    SHELLEXECUTEINFOW shex =
    {
        sizeof(SHELLEXECUTEINFOW),
        0,                                                        // fMask
        ((parent) ? (HWND)parent : GetDesktopWindow()),
        L"runas",
        (LPCWSTR)qApp->applicationFilePath().utf16(),
        arg,
        (LPCWSTR)qApp->applicationDirPath().utf16(),
        SW_NORMAL
    };

    BOOL res = ShellExecuteExW(&shex);

    qDebug() << Q_FUNC_INFO << (res ? " succeeded" : " failed");
}
#endif

QMainWindow* getMainWindow()
{
    for (QWidget* widget : QApplication::topLevelWidgets())
        if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(widget))
            return mainWindow;
    return nullptr;
}

} // namespace utilities
