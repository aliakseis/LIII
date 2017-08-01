#pragma once

#ifdef ALLOW_TRAFFIC_CONTROL
#include "traffic_limitation/InterceptingApplication.h"
#else
#include "qtsingleapplication/qtsingleapplication.h"
#endif // ALLOW_TRAFFIC_CONTROL


#include "utilities/translatable.h"
#include "utilities/utils.h"


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

    Application(const QString& id, int& argc, char** argv);

    void passCmdLine();

    void checkFirewallException(QMainWindow* mainWindow);
    bool allowAsSecondInstance() const { return alowAsSecondInstance_; }
    bool isMissionDone() const { return missionDone; }

private:
    bool missionDone; // flag to ignore all other commands, if mission done in c-tor
    bool alowAsSecondInstance_;

    void checkSpecialCmdLine();

};
