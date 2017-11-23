#include "filesystem_utils.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QString>
#include <QStandardPaths>

#ifdef Q_OS_WIN32
#include <windows.h>
#include <winsock2.h>
#include <winnt.h>
#include <shellapi.h>
#include <knownfolders.h>
#include <ShTypes.h>
#include <objbase.h>
typedef HRESULT(STDAPICALLTYPE* GetPathFunc)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*);
#endif

namespace {

#ifdef Q_OS_WIN32
const char EXPLORER_LABEL[]            =    "explorer";
#elif defined(Q_OS_MAC)
const char EXPLORER_LABEL[]            =    "open";
const char APPLESCRIPT_BIN[]           =    "osascript";
#elif defined(Q_OS_LINUX)
const char EXPLORER_LABEL[]            =    "xdg-open";
#endif

#ifdef Q_OS_WIN32

bool MoveToTrashImpl(const QString& file)
{
    QFileInfo fileinfo(file);
    if (!fileinfo.exists())
    {
        return false;
    }
    QString absPath = fileinfo.absoluteFilePath() + '\0'; // This string must be double-null terminated
    SHFILEOPSTRUCTW fileop = {0, FO_DELETE, qUtf16Printable(absPath), 0, FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT};
    int rv = SHFileOperationW(&fileop);
    if (0 != rv)
    {
        qDebug() << __FUNCTION__ << "SHFileOperationW returned:" << rv;
        return false;
    }

    return true;
}
#endif //Q_OS_WIN32


#ifdef Q_OS_LINUX

bool TrashInitialized = false;
QString TrashPath;
QString TrashPathInfo;
QString TrashPathFiles;

bool MoveToTrashImpl(const QString& file)
{
#ifdef QT_GUI_LIB
    if (!TrashInitialized)
    {
        QStringList paths;
        const char* xdg_data_home = getenv("XDG_DATA_HOME");
        if (xdg_data_home)
        {
            qDebug() << "XDG_DATA_HOME not yet tested";
            QString xdgTrash(xdg_data_home);
            paths.append(xdgTrash + "/Trash");
        }
        QString home =
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        paths.append(home + "/.local/share/Trash");
        paths.append(home + "/.trash");
        for (const QString& path : qAsConst(paths))
        {
            if (TrashPath.isEmpty())
            {
                QDir dir(path);
                if (dir.exists())
                {
                    TrashPath = path;
                }
            }
        }
        if (TrashPath.isEmpty())
        {
            return false;
        }
        TrashPathInfo = TrashPath + "/info";
        TrashPathFiles = TrashPath + "/files";
        if (!QDir(TrashPathInfo).exists() || !QDir(TrashPathFiles).exists())
        {
            return false;
        }
        TrashInitialized = true;
    }
    QFileInfo original(file);
    if (!original.exists())
    {
        return false;
    }
    QString info;
    info += "[Trash Info]\nPath=";
    info += original.absoluteFilePath();
    info += "\nDeletionDate=";
    info += QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzzZ");
    info += "\n";
    QString trashname = original.fileName();
    QString infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
    QString filepath = TrashPathFiles + "/" + trashname;
    int nr = 1;
    while (QFileInfo(infopath).exists() || QFileInfo(filepath).exists())
    {
        nr++;
        trashname = original.baseName() + "." + QString::number(nr);
        if (!original.completeSuffix().isEmpty())
        {
            trashname += QString(".") + original.completeSuffix();
        }
        infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
        filepath = TrashPathFiles + "/" + trashname;
    }
    QDir dir;
    if (!dir.rename(original.absoluteFilePath(), filepath))
    {
        return false;
    }
    QFile(infopath).write(info.utf8());

#else
    Q_UNUSED(file);
    return false;
#endif //QT_GUI_LIB

    return true;
}
#endif //Q_OS_LINUX

#ifdef Q_OS_MAC
bool MoveToTrashImpl(const QString& file)
{
    QString moveToTrashCmdLine = "tell application \"Finder\" to delete file POSIX file \"" + file + "\"";
    return 0 == system(moveToTrashCmdLine.toUtf8().constData());
}
#endif //Q_OS_MAC

} // namespace


namespace utilities
{

bool MoveToTrash(const QString& path) {return MoveToTrashImpl(path);}

bool DeleteFileWithWaiting(const QString& file)
{
    qDebug() << __FUNCTION__ << " deleting file: " << file;
    if (QFile::exists(file) && !QFile::remove(file))
    {
        QThread::yieldCurrentThread();
        if (!QFile::remove(file))
        {
            qDebug() << __FUNCTION__ << " failed with " << file;
            return false;
        }
    }
    return true;
}

void SelectFile(const QString& fileName, const QString& defFolderName)
{
    QStringList args;
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists())
    {
        if (fileInfo.isDir())
        {
            args << QDir::toNativeSeparators(fileInfo.absoluteFilePath());
        }
        else
        {
#ifdef Q_OS_WIN
            args << "/n,/select," <<  QDir::toNativeSeparators(fileInfo.absoluteFilePath());
#elif defined (Q_OS_DARWIN)
            args << "-e";
            args << "tell application \"Finder\"";
            args << "-e";
            args << "activate";
            args << "-e";
            args << "select POSIX file \"" + fileName + "\"";
            args << "-e";
            args << "end tell";
            QProcess::startDetached(APPLESCRIPT_BIN, args);
            return;
#else
            args << QDir::toNativeSeparators(fileInfo.absoluteFilePath());
#endif
        }
    }
    else
    {
        QFileInfo fileInfo(defFolderName);
        args << QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    }
    QProcess::startDetached(EXPLORER_LABEL, args);
}

QString GetFileName(QNetworkReply* reply)
{
    QString result;
    if (reply)
    {
        QByteArray filename = reply->rawHeader("Content-Disposition");
        if (!filename.isEmpty())
        {
            QString disposition = QString::fromUtf8(filename.constData());
            QRegExp rx("filename\\s?=\\s?\\\"?([^\\\"]+)");
            if (rx.indexIn(disposition) != -1)
            {
                result = rx.cap(1);
            }
        }
    }
    return result;
}

QString GetFileName(const QString& full_path)
{
    QString file_name = QFileInfo(full_path).fileName();
    return (file_name.isEmpty()) ? full_path : file_name;
}

QString getPathForDownloadFolder()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
    QString startPath = !paths.empty() ? paths.at(0) : QString();
    return startPath;
}


QString PrepareCacheFolder(const QString& subdir)
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    QString path = !paths.isEmpty() ? paths.at(0) : QString();

    if (!subdir.isEmpty())
    {
        path += QDir::separator() + subdir;
    }
    Q_ASSERT(!path.isEmpty());

    QDir().mkpath(path);
    return path + QDir::separator();
}

} // utilities namespace

