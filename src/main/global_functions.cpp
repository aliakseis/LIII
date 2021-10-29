#include "global_functions.h"

#include <iterator>
#include <algorithm>
#include <functional>
#include <set>

#include <QStringList>
#include <QRegExp>
#include <QVariant>
#include <QNetworkRequest>
#include <QUrl>
#include <QList>
#include <QLocale>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

#include "utilities/utils.h"
#include "utilities/filesystem_utils.h"
#include "downloadtype.h"
#include "utilities/credential.h"
#include "settings_declaration.h"
#include "utilities/simplecrypt.h"

namespace global_functions
{

using namespace app_settings;


QString GetNormalizedDomain(const QUrl& url)
{
    QString host = url.host();
    if (host.startsWith("www."))
    {
        host = host.right(host.length() - 4);
    }
    return host;
}

void openFile(const QString& filename, QWidget* parent)
{
    if (!QFileInfo(filename).isExecutable()
            || QMessageBox::question(
                parent, QObject::tr("Possible Threat"),
                QObject::tr("Are you sure you want to start an executable file\n\"%1\"?").arg(filename),
                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
    }
}

QString GetVideoFolder()
{
    QString fixedFolder = QSettings().value(VideoFolder).toString().trimmed();
    QStringList pathItems = fixedFolder.split(QRegExp("[/\\\\]+"), QString::SkipEmptyParts);

    fixedFolder = fixedFolder.isEmpty() ?
        utilities::getPathForDownloadFolder() :
        (fixedFolder.startsWith(QDir::separator()) ? QDir::separator() : QString()) + pathItems.join(QDir::separator());
    return fixedFolder.endsWith(QDir::separator()) ? fixedFolder : fixedFolder + QDir::separator();
}


int GetMaximumNumberLoadsActual()
{
    enum { PSEUDO_UNLIMITED_NUMBER_OF_DOWNLOADS = 100 };
    QSettings settings;
    return settings.value(UnlimitedLabel, UnlimitedLabel_Default).toBool()
        ? PSEUDO_UNLIMITED_NUMBER_OF_DOWNLOADS
        : settings.value(MaximumNumberLoads, MaximumNumberLoads_Default).toInt();
}

int GetTrafficLimitActual()
{
    QSettings settings;
    return settings.value(IsTrafficLimited, IsTrafficLimited_Default).toBool()
        ? settings.value(TrafficLimitKbs, TrafficLimitKbs_Default).toInt() : 0;
}

const auto cryptKey = Q_UINT64_C(0xE05CE1B3AC501B30);

QString SimpleEncryptString(const QString& str)
{
    return SimpleCrypt(cryptKey).encryptToString(str);
}

QString SimpleDecryptString(const QString& str)
{
    return SimpleCrypt(cryptKey).decryptToString(str);
}

} // namespace global_functions
