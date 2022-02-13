#include <QDebug>
#include <QApplication>
#include <QProcess>

#include "utilities/logger.h"
#include "utilities/utils.h"

#include "configurableproxyfactory.h"

#include "mainwindow.h"
#include "LIIIstyle.h"
#include "settings_declaration.h"
#include "globals.h"
#include "application.h"
#include "version.hxx"
#include "commandlineparser.h"

#include "torrentmanager.h"

#ifdef Q_OS_MAC
#include "darwin/AppHandler.h"
#endif //Q_OS_MAC


#if defined (QT_STATICPLUGINS) && !defined (Q_OS_DARWIN)
#include <QtPlugin>
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qico)
#endif//QT_STATICPLUGINS


// Called once QCoreApplication exists
static void ApplicationDependentInitialization()
{
    if (utilities::IsPortableMode())
    {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
            QCoreApplication::applicationDirPath());
    }

    utilities::setWriteToLogFile(
        QSettings().value(app_settings::LoggingEnabled, false).toBool());
}

Q_COREAPP_STARTUP_FUNCTION(ApplicationDependentInitialization)

int main(int argc, char* argv[])
{
    utilities::InitializeProjectDescription();

    QNetworkProxyFactory::setApplicationProxyFactory(new ConfigurableProxyFactory());

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // single app
    Application app(QString(PROJECT_NAME), argc, argv);

    if (!app.allowAsSecondInstance() && app.isRunning())
    {
        app.passCmdLine();
        qDebug() << "Another application is running; exiting ...";
        return 0;
    }

    qDebug() << "Starting " PROJECT_FULLNAME " version " PROJECT_VERSION " built " __DATE__ " " __TIME__;

    Application::setQuitOnLastWindowClosed(false);

    Application::setStyle(new LIIIStyle);
    app.retranslateApp(QSettings().value(app_settings::ln, app_settings::ln_Default).toString());

    DownloadCollectionModel::instance().init();

    MainWindow w;

    QFile css_data(":/style.css");
    if (css_data.open(QIODevice::ReadOnly))
    {
        const auto css = css_data.readAll();
        w.setStyleSheet(css);
        app.setStyleSheet(css);
        css_data.close();
    }

    processCommandLine(&w);

#ifdef Q_OS_DARWIN
    Darwin::SetApplicationHandler(&w);
    Darwin::OpenEventFilter* eventFiler = new Darwin::OpenEventFilter();

    app.installEventFilter(eventFiler);
    app.addLibraryPath(
        QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() + PROJECT_NAME
                                 + ".app/Contents/MacOS/plugins"));
#endif //Q_OS_DARWIN

    w.showMainWindowAndPerformChecks();

    app.setActivationWindow(&w);
    VERIFY(QObject::connect(&app, SIGNAL(messageReceived(QString)), &w, SLOT(restore())));

    int retcode = Application::exec();

    qDebug() << "End " PROJECT_NAME;

    QNetworkProxyFactory::setApplicationProxyFactory(0);

    return retcode;
}
