#pragma once

#include "qtsingleapplication/qtsingleapplication.h"
#include "TrafficControl.h"

namespace traffic_limitation
{

class InterceptingApp
    : public QtSingleApplication
{
public:
    using QtSingleApplication::QtSingleApplication;

protected:
    bool notify(QObject* receiver, QEvent* event) override
    {
        return handleSocketReadNotify(receiver, event) || QtSingleApplication::notify(receiver, event);
    }
};

} // namespace traffic_limitation
