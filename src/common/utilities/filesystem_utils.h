#pragma once

#include <QString>

class QNetworkReply;

// util functions for OS-dependant file operations, not supported in Qt.


namespace utilities
{

// returns true if success
bool MoveToTrash(const QString& path);

// returns true if success
bool DeleteFileWithWaiting(const QString& file);

void SelectFile(const QString& fileName, const QString& defFolderName);

QString GetFileName(QNetworkReply* reply);
QString GetFileName(const QString& full_path);

bool IsPortableMode();

// Parameter: const QString & subdir - subdir to create;  Returns:  QString - directory name with ending slash
QString PrepareCacheFolder(const QString& subdir = QString());

QString getPathForDownloadFolder();

}
