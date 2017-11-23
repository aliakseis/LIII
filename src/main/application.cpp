#include "application.h"
#include "commandlineparser.h"
#include "utilities/associate_app.h"
#include <QMessageBox>
#ifdef Q_OS_WIN
#include "utilities/windowsfirewall.h"
#endif
#include "mainwindow.h"
#include "branding.hxx"

Application::Application(const QString& id, int& argc, char** argv)
#ifdef ALLOW_TRAFFIC_CONTROL
    : traffic_limitation::InterceptingApp(id, argc, argv)
#else
    : QtSingleApplication(id, argc, argv)
#endif // ALLOW_TRAFFIC_CONTROL
    , missionDone(false)
    , alowAsSecondInstance_(false)
{
    connect(this, &QtSingleApplication::messageReceived, &processMessageReceived);
    checkSpecialCmdLine();
}


void Application::passCmdLine()
{
    if (missionDone)
    {
        return;
    }
    // On Windows, the arguments() are not built from the contents of argv/argc, as the content does not support Unicode.
    // Instead, the arguments() are constructed from the return value of GetCommandLine().
    QStringList args = QApplication::arguments();
    args.removeFirst();
    sendMessage(args.join("\n"), 1000);
}

void Application::checkFirewallException(QMainWindow* mainWindow)
{
#ifdef Q_OS_WIN
    utilities::WindowsFirewall firewall;

    if (!firewall.isEnabled())
    {
        return;
    }

    QString appPath = QCoreApplication::applicationFilePath();
    utilities::WindowsFirewall::FirewallStatus status = firewall.getFirewallStatus(appPath);

    Q_ASSERT(status != utilities::WindowsFirewall::UNKNOWN);

    if (status == utilities::WindowsFirewall::DOESNT_EXIST || status == utilities::WindowsFirewall::BLOCKED)
    {
        QMessageBox msgBox(
            QMessageBox::NoIcon,
            utilities::Tr::Tr(PROJECT_FULLNAME_TRANSLATION),
            tr("Allow %1 on Firewall").arg(utilities::Tr::Tr(PROJECT_FULLNAME_TRANSLATION)),
            QMessageBox::Ok,
            mainWindow);
        if (utilities::isAdminRights())
        {
            const bool isOk = firewall.addApplicationPolicy(appPath, PROJECT_NAME);
            if (!isOk)
            {
                msgBox.setInformativeText(tr("%1 failed to apply firewall rules. Please<br>setup your firewall manually.")
                               .arg(utilities::Tr::Tr(PROJECT_FULLNAME_TRANSLATION)));
                msgBox.exec();
            }
        }
        else
        {
            msgBox.setInformativeText(tr("Click \"Yes\" on the User Account Control notification in your taskbar to continue using %1.").arg(utilities::Tr::Tr(PROJECT_FULLNAME_TRANSLATION)));
            
            msgBox.exec();

            utilities::runWithPrivileges(L"--windows_firewall_apply", mainWindow->winId());
        }
    }
#endif
}

void Application::checkSpecialCmdLine()
{
    QStringList args = arguments();
    qDebug() << Q_FUNC_INFO << " found args " << args;
    if (args.size() >= 2)
    {
        if (args[1] == "--set_as_default_torrent_app")
        {
            utilities::setDefaultTorrentApp();
            missionDone = true;
        }
        else if (args[1] == "--unset_as_default_torrent_app")
        {
            utilities::unsetDefaultTorrentApp();
            missionDone = true;
        }

        if (args[1] == "--windows_firewall_apply" || (args.size() == 3 && args[2] == "--windows_firewall_apply"))
        {
#ifdef Q_OS_WIN

            utilities::WindowsFirewall().addApplicationPolicy(QCoreApplication::applicationFilePath(), PROJECT_NAME);
            missionDone = true;

#else
#pragma message("TODO: implement this")
#endif
        }

    }
}
