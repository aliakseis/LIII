#pragma once

#include <QString>


class QStringList;
class QNetworkRequest;
class QUrl;
class QDomElement;
class QWidget;

namespace global_functions
{

QString GetNormalizedDomain(const QUrl& url);
void openFile(const QString& filename, QWidget* parent = 0);

QString GetVideoFolder();
int GetMaximumNumberLoadsActual();
int GetTrafficLimitActual();

QString SimpleEncryptString(const QString& str);
QString SimpleDecryptString(const QString& str);

} // namespace global_functions
