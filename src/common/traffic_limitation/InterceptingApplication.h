#pragma once

#include "qtsingleapplication/qtsingleapplication.h"
#include "TrafficControl.h"

namespace traffic_limitation
{

class InterceptingApp
    : public QtSingleApplication
{
public:
    InterceptingApp(const QString& id, int& argc, char** argv)
        : QtSingleApplication(id, argc, argv)
    {
    }
    InterceptingApp(int& argc, char** argv, bool GUIenabled = true)
        : QtSingleApplication(argc, argv, GUIenabled)
    {
    }

protected:
    virtual bool notify(QObject* receiver, QEvent* event) override
    {
        return handleSocketReadNotify(receiver, event) || QtSingleApplication::notify(receiver, event);
    }
};

} // namespace traffic_limitation
