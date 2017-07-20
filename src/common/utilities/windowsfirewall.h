#pragma once

#if defined(WIN32) || defined(_WIN32)

#include <stdexcept>
#include <QString>
#include <atlcomcli.h>

struct INetFwProfile;

namespace utilities
{

class WindowsFirewall
{
public:
    enum FirewallStatus
    {
        DOESNT_EXIST = -2,
        BLOCKED = -1,
        UNKNOWN = 0,
        ALLOWED = 1
    };
    WindowsFirewall();
    ~WindowsFirewall();
    bool isEnabled();
    FirewallStatus getFirewallStatus(const QString& imageName);
    // returns true if succeeded
    bool addApplicationPolicy(const QString& imageName, const QString& applicationName);
private:
    void initialize();
    CComPtr<INetFwProfile> fwProfile;
};

}

#endif // #if defined(WIN32) || defined(_WIN32)
