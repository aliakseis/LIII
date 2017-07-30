#include "associate_app.h"
#include "utils.h"

#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QApplication>

namespace {

const auto torrentClassName = QStringLiteral(PROJECT_NAME ".torrent");

void setClassesToSelf(QSettings& settingRoot)
{
    QString appPath = QDir::toNativeSeparators(qApp->applicationFilePath());
    settingRoot.setValue(".", torrentClassName);

    settingRoot.beginGroup("DefaultIcon");
    settingRoot.setValue(".", QString("\"%1\",0").arg(appPath));
    settingRoot.endGroup();

    settingRoot.beginGroup("shell");
    settingRoot.beginGroup("open");
    settingRoot.setValue(".", "Open with " PROJECT_NAME);
    settingRoot.beginGroup("command");
    settingRoot.setValue(".", QString("\"%1\" \"%2\"").arg(appPath, "%1"));
    settingRoot.endGroup();
    settingRoot.endGroup();
    settingRoot.endGroup();
}

void associateApp(QString const& ext, WId parent = NULL)
{
    const bool isChangeNeeded =
        QSettings("HKEY_CLASSES_ROOT\\" + torrentClassName, QSettings::NativeFormat).value(".") != PROJECT_FULLNAME ||
        QSettings("HKEY_CLASSES_ROOT\\" + ext, QSettings::NativeFormat).value(".") != torrentClassName ||
        QSettings("HKEY_CLASSES_ROOT\\" + torrentClassName + "\\shell\\open\\command", QSettings::NativeFormat).value(".") != QString("\"%1\" \"%2\"").arg(QDir::toNativeSeparators(qApp->applicationFilePath()), "%1");

    if (isChangeNeeded)
    {
        if (utilities::isAdminRights())
        {
            qDebug() << Q_FUNC_INFO << " with admin rights";
            QSettings hkcr("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
            hkcr.beginGroup(ext);
            QVariant oldVal = hkcr.value(".");
            if (!oldVal.isNull() && !oldVal.toString().isEmpty() && oldVal != torrentClassName)
            {
                hkcr.setValue(torrentClassName + "_backup", oldVal);
            }
            hkcr.setValue(".", torrentClassName);
            hkcr.setValue("Content Type", "application/x-bittorrent");
            hkcr.endGroup();

            hkcr.beginGroup(torrentClassName);
            setClassesToSelf(hkcr);
            hkcr.setValue(".", PROJECT_FULLNAME);
            hkcr.endGroup();
        }
        else
        {
            utilities::runWithPrivileges(L"--set_as_default_torrent_app", parent);
        }
    }
    else
    {
        qDebug() << Q_FUNC_INFO " did nothing as no changes supposed to be required.";
    }
}

} // namespace

bool utilities::isDefaultTorrentApp()
{
    const bool isSelf =
        QSettings("HKEY_CLASSES_ROOT\\Magnet", QSettings::NativeFormat).value(".") == torrentClassName &&
        QSettings("HKEY_CLASSES_ROOT\\.torrent", QSettings::NativeFormat).value(".") == torrentClassName &&
        QSettings("HKEY_CLASSES_ROOT\\" + torrentClassName + "\\shell\\open\\command", QSettings::NativeFormat).value(".") == QString("\"%1\" \"%2\"").arg(QDir::toNativeSeparators(qApp->applicationFilePath()), "%1") &&
        QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.torrent\\UserChoice", QSettings::NativeFormat).value("ProgId") == torrentClassName;

    return isSelf;
}

void utilities::setDefaultTorrentApp(WId parent)
{
    qDebug() << Q_FUNC_INFO;

    associateApp(".torrent", parent);
    associateApp("Magnet", parent);
    QSettings("HKEY_CLASSES_ROOT\\Magnet", QSettings::NativeFormat).setValue("URL Protocol", QString()); // to recognize "magnet:" as an uri protocol

    setClassesToSelf(QSettings("HKEY_CURRENT_USER\\Software\\Classes\\Magnet", QSettings::NativeFormat));
    setClassesToSelf(QSettings("HKEY_CURRENT_USER\\Software\\Classes\\.torrent", QSettings::NativeFormat));


    QSettings torrentExt("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.torrent", QSettings::NativeFormat);

    torrentExt.beginGroup("UserChoice");
    torrentExt.setValue("ProgId", torrentClassName);
    torrentExt.endGroup();

    torrentExt.beginGroup("OpenWithProgids");
    torrentExt.setValue(torrentClassName, QByteArray());
    torrentExt.endGroup();

    torrentExt.beginGroup("OpenWithList");
    char ind  = 'a'; 
    while (!torrentExt.value(QString(ind)).isNull()) { ++ind; }
    torrentExt.setValue(QString(ind), QString(PROJECT_NAME ".exe"));
    torrentExt.setValue("MRUList", QString(ind) + torrentExt.value("MRUList", QString()).toString());
    torrentExt.endGroup();
}

void unsetRegKey(QSettings& key)
{
    QVariant restoreVal = key.value(torrentClassName + "_backup");
    if (!restoreVal.isNull())
    {
        key.setValue(".", restoreVal);
        key.remove(torrentClassName + "_backup");
    }
    else
    {
        key.setValue(".", QVariant());
        key.remove("");
    }

}

void utilities::unsetDefaultTorrentApp()
{
    qDebug() << Q_FUNC_INFO;

    // TODO: set to previous default app

    // for now, delete settings

    if (isAdminRights())
    {
        unsetRegKey(QSettings("HKEY_CLASSES_ROOT\\Magnet", QSettings::NativeFormat));
        unsetRegKey(QSettings("HKEY_CLASSES_ROOT\\.torrent", QSettings::NativeFormat));
    }
    else if (isDefaultTorrentApp())
    {
        runWithPrivileges(L"--unset_as_default_torrent_app");
    }

    unsetRegKey(QSettings("HKEY_CURRENT_USER\\Software\\Classes\\Magnet", QSettings::NativeFormat));
    unsetRegKey(QSettings("HKEY_CURRENT_USER\\Software\\Classes\\.torrent", QSettings::NativeFormat));

    unsetRegKey(QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.torrent", QSettings::NativeFormat));
}
