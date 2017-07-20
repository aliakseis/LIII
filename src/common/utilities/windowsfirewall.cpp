#include "windowsfirewall.h"
#include <QDebug>
#include <QDir>

#if defined(WIN32) || defined(_WIN32)

#include <windows.h>
#include <netfw.h>
#include <crtdbg.h>
#include <comip.h>
#include <comdef.h>

#include "utils.h"

namespace utilities
{

using _com_util::CheckError;

WindowsFirewall::WindowsFirewall()
{
    _COM_SMARTPTR_TYPEDEF(INetFwMgr, __uuidof(INetFwMgr));

    try
    {
        INetFwMgrPtr fwMgr(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER);
        CComPtr<INetFwPolicy> fwPolicy;

        Q_ASSERT_X(!fwProfile, Q_FUNC_INFO, "profile initialized twice ???");

        CheckError(fwMgr->get_LocalPolicy(&fwPolicy));
        // Retrieve the firewall profile currently in effect.
        CheckError(fwPolicy->get_CurrentProfile(&fwProfile));

        Q_ASSERT(fwProfile);
    }
    catch (_com_error const& wtf)
    {
        qWarning() << Q_FUNC_INFO << "failed: " << asString(wtf.ErrorMessage());
    }
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
        CheckError(fwProfile->get_FirewallEnabled(&fwEnabled));

        // Check to see if the firewall is on.
        return (fwEnabled != VARIANT_FALSE);
    }
    catch (_com_error const& wtf)
    {
        qWarning() << Q_FUNC_INFO << "failed: " << asString(wtf.ErrorMessage());
        return false;
    }
}

WindowsFirewall::FirewallStatus WindowsFirewall::getFirewallStatus(const QString& imageName)
{
    try
    {
        _bstr_t procImgFilename(qUtf16Printable(QDir::toNativeSeparators(imageName)));
        CComPtr<INetFwAuthorizedApplications> fwApps;
        CComPtr<INetFwAuthorizedApplication> fwApp;

        CheckError(fwProfile->get_AuthorizedApplications(&fwApps));

        if (SUCCEEDED(fwApps->Item(procImgFilename, &fwApp)))
        {
            VARIANT_BOOL fwEnabled;
            CheckError(fwApp->get_Enabled(&fwEnabled));

            return (fwEnabled == VARIANT_FALSE) ? BLOCKED : ALLOWED;
        }

        return DOESNT_EXIST;
    }
    catch (_com_error const& wtf)
    {
        qWarning() << Q_FUNC_INFO << "failed: " << asString(wtf.ErrorMessage());
        return UNKNOWN;
    }
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

    try
    {
        _bstr_t procImgFilename(qUtf16Printable(QDir::toNativeSeparators(imageName)));
        _bstr_t appNameStr(qUtf16Printable(applicationName));

        // query authorized firewall applications
        CComPtr<INetFwAuthorizedApplications> fwApps;
        CheckError(fwProfile->get_AuthorizedApplications(&fwApps));

        // add us to this list
        INetFwAuthorizedApplicationPtr fwApp(__uuidof(NetFwAuthorizedApplication), NULL, CLSCTX_INPROC_SERVER);
        CheckError(fwApp->put_ProcessImageFileName(procImgFilename));
        CheckError(fwApp->put_Name(appNameStr));
        CheckError(fwApps->Add(fwApp));
    }
    catch (_com_error const& wtf)
    {
        qWarning() << Q_FUNC_INFO << "failed: " << asString(wtf.ErrorMessage());
        return false;
    }

    return true;
}

}

#endif
