#include "commandlineparser.h"

#include "utilities/utils.h"
#include "utilities/autorun_utils.h"

#include "downloadtype.h"
#include "mainwindow.h"
#include "application.h"


CommandLineParser::CommandLineParser(MainWindow* w)
    : mainWindow(w)
{
    // On Windows, the arguments() are not built from the contents of argv/argc, as the content does not support Unicode.
    // Instead, the arguments() are constructed from the return value of GetCommandLine().
    QStringList params = QApplication::arguments();
    if (params.length() > 1)
    {
        params.removeFirst();
        treatParams(params);
    }
}

QStringList CommandLineParser::getLinksFromFile(const QString& filename)
{
    QStringList links;
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Error reading file " << filename;
        return links;
    }
    QString contents = QString(f.readAll());
    f.close();

    links = utilities::ParseUrls(contents);
    return links;
}

void CommandLineParser::treatParams(QStringList params)
{
    Application* myApp = dynamic_cast<Application*>(qApp);
    if (myApp && !myApp->isMissionDone())
    {
        if (utilities::isAutorunMode())
        {
            mainWindow->setAutorunMode();
        }
        else
        {
            mainWindow->addLinks(params);
        }
    }
}

