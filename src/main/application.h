#pragma once

#ifdef ALLOW_TRAFFIC_CONTROL
#include "traffic_limitation/InterceptingApplication.h"
#else
#include "qtsingleapplication/qtsingleapplication.h"
#endif // ALLOW_TRAFFIC_CONTROL


#include "utilities/translatable.h"
#include "utilities/utils.h"

class CommandLineParser;

class Application
#ifdef ALLOW_TRAFFIC_CONTROL
    : public traffic_limitation::InterceptingApp,
#else
    : public QtSingleApplication,
#endif // ALLOW_TRAFFIC_CONTROL
  public utilities::Translatable
{
    Q_OBJECT
public:

    Application(const QString& id, int& argc, char** argv)
#ifdef ALLOW_TRAFFIC_CONTROL
        : traffic_limitation::InterceptingApp(id, argc, argv)
#else
        : QtSingleApplication(id, argc, argv)
#endif // ALLOW_TRAFFIC_CONTROL
        , cmdLineParser(nullptr)
        , missionDone(false)
        , alowAsSecondInstance_(false)
    {
        VERIFY(connect(this, SIGNAL(messageReceived(const QString&)), this, SLOT(processCmdLine(const QString&))));
        checkSpecialCmdLine();
    }

    void setCmdLineParser(CommandLineParser* cmdParser) { cmdLineParser = cmdParser; }
    void passCmdLine();

    void checkFirewallException(QMainWindow* mainWindow);
    bool allowAsSecondInstance()const {return alowAsSecondInstance_;}
    bool isMissionDone()const {return missionDone;};


private Q_SLOTS:
    void processCmdLine(const QString& cmdLine);

private:
    CommandLineParser* cmdLineParser;
    bool missionDone; // flag to ignore all other commands, if mission done in c-tor
    bool alowAsSecondInstance_;

    void checkSpecialCmdLine();

};
