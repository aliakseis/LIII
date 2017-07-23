#pragma once

#include <QString>

#include "utilities/credential.h"


#include "settings_declaration.h"


class QStringList;
class QNetworkRequest;
class QUrl;
class QDomElement;

Q_DECLARE_METATYPE(utilities::Credential)

namespace global_functions
{

QString GetNormalizedDomain(const QUrl& url);
bool IsExecutable(const QString& filename);
void openFile(const QString& filename, QWidget* parent = 0);

QString GetVideoFolder();
int GetMaximumNumberLoadsActual();
int GetTrafficLimitActual();

} // namespace global_functions
