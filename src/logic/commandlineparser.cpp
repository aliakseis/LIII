#include "commandlineparser.h"

#include <utility>

#include "utilities/utils.h"
#include "utilities/autorun_utils.h"

#include "downloadtype.h"
#include "mainwindow.h"
#include "application.h"

namespace {

void treatParams(MainWindow* mainWindow, QStringList params)
{
    auto* myApp = dynamic_cast<Application*>(qApp);
    if (myApp && !myApp->isMissionDone())
    {
        if (utilities::isAutorunMode())
        {
            mainWindow->setAutorunMode();
        }
        else
        {
            mainWindow->addLinks(std::move(params));
        }
    }
}

} // namespace


void processCommandLine(MainWindow* w)
{
    QStringList params = QApplication::arguments();
    if (params.length() > 1)
    {
        params.removeFirst();
        treatParams(w, params);
    }
}

void processMessageReceived(const QString& cmdLine)
{
    if (!cmdLine.isEmpty())
    {
        treatParams(
            static_cast<MainWindow*>(utilities::getMainWindow()),
            cmdLine.split('\n', QString::SkipEmptyParts));
    }
}
