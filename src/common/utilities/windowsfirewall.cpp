#include "windowsfirewall.h"
#include <QDebug>
#include <QDir>

#if defined(WIN32) || defined(_WIN32)

#include <windows.h>
#include <netfw.h>
#include <crtdbg.h>
#include <comip.h>
#include <comdef.h>

namespace utilities
{

WindowsFirewall::WindowsFirewall()
{
    initialize();
}

WindowsFirewall::~WindowsFirewall()
{
}

bool WindowsFirewall::isEnabled()
{
    Q_ASSERT(fwProfile);

    try
    {
        // Get the current state of the firewall.
        VARIANT_BOOL fwEnabled;
        _com_util::CheckError(fwProfile->get_FirewallEnabled(&fwEnabled));

        // Check to see if the firewall is on.
        return (fwEnabled != VARIANT_FALSE);
    }
    catch (_com_error const& wtf)
    {
        qWarning() << Q_FUNC_INFO << "failed: " << QString::fromUtf16((const ushort*)wtf.ErrorMessage());
        return false;
    }
}

void WindowsFirewall::initialize()
{
    _COM_SMARTPTR_TYPEDEF(INetFwMgr, __uuidof(INetFwMgr));
    _COM_SMARTPTR_TYPEDEF(INetFwPolicy, __uuidof(INetFwPolicy));

    try
    {
        INetFwMgrPtr fwMgr(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER);
        INetFwPolicyPtr fwPolicy;

        Q_ASSERT_X(!fwProfile, Q_FUNC_INFO, "profile initialized twice ???");

        _com_util::CheckError(fwMgr->get_LocalPolicy(&fwPolicy));
        // Retrieve the firewall profile currently in effect.
        _com_util::CheckError(fwPolicy->get_CurrentProfile(&fwProfile));

        Q_ASSERT(fwProfile);
    }
    catch (_com_error const& wtf)
    {
        qWarning() << Q_FUNC_INFO << "failed: " << QString::fromUtf16((const ushort*)wtf.ErrorMessage());
    }
}

WindowsFirewall::FirewallStatus WindowsFirewall::getFirewallStatus(const QString& imageName)
{
    FirewallStatus result = UNKNOWN;

    _COM_SMARTPTR_TYPEDEF(INetFwAuthorizedApplication,  __uuidof(INetFwAuthorizedApplication));
    _COM_SMARTPTR_TYPEDEF(INetFwAuthorizedApplications, __uuidof(INetFwAuthorizedApplications));

    try
    {
        _bstr_t procImgFilename((const wchar_t*)QDir::toNativeSeparators(imageName).utf16());
        INetFwAuthorizedApplicationsPtr fwApps;
        INetFwAuthorizedApplicationPtr fwApp;

        _com_util::CheckError(fwProfile->get_AuthorizedApplications(&fwApps));

        if (SUCCEEDED(fwApps->Item(procImgFilename, &fwApp)))
        {
            VARIANT_BOOL fwEnabled;
            _com_util::CheckError(fwApp->get_Enabled(&fwEnabled));

            result = (fwEnabled == VARIANT_FALSE ? BLOCKED : ALLOWED);
        }
        else
        {
            result = DOESNT_EXIST;
        }
    }
    catch (_com_error const& wtf)
    {
        qWarning() << Q_FUNC_INFO << "failed: " << QString::fromUtf16((const ushort*)wtf.ErrorMessage());
    }

    return result;
}

bool WindowsFirewall::addApplicationPolicy(const QString& imageName, const QString& applicationName)
{
    if (!fwProfile)
    {
        qWarning() << Q_FUNC_INFO << "error: fwProfile is not initialized";
        Q_ASSERT(false);
        return false;
    }

    if (getFirewallStatus(imageName) == ALLOWED)
    {
        return true;    // success is when problem does not exist
    }

    _COM_SMARTPTR_TYPEDEF(INetFwAuthorizedApplication,  __uuidof(INetFwAuthorizedApplication));
    _COM_SMARTPTR_TYPEDEF(INetFwAuthorizedApplications, __uuidof(INetFwAuthorizedApplications));

    try
    {
        _bstr_t procImgFilename((const wchar_t*)QDir::toNativeSeparators(imageName).utf16());
        _bstr_t appNameStr((const wchar_t*)applicationName.utf16());

        // query authorized firewall applications
        INetFwAuthorizedApplicationsPtr fwApps;
        _com_util::CheckError(fwProfile->get_AuthorizedApplications(&fwApps));

        // add us to this list
        INetFwAuthorizedApplicationPtr fwApp(__uuidof(NetFwAuthorizedApplication), NULL, CLSCTX_INPROC_SERVER);
        _com_util::CheckError(fwApp->put_ProcessImageFileName(procImgFilename));
        _com_util::CheckError(fwApp->put_Name(appNameStr));
        _com_util::CheckError(fwApps->Add(fwApp));
    }
    catch (_com_error const& wtf)
    {
        qWarning() << Q_FUNC_INFO << "failed: " << QString::fromUtf16((const ushort*)wtf.ErrorMessage());
        return false;
    }

    return true;
}

}


#endif
