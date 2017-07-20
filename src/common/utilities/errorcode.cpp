#include "errorcode.h"

namespace utilities
{

ErrorCode::ErrorCode()
    : errors_
{
    { eNOTERROR, {Tr::translate("ErrorMsg", "Unknown error"), 0} },
    { eSCRPHTTPERR, {Tr::translate("ErrorMsg", "Script error"), 0} },
    { eSCRPNETWORKERR, {Tr::translate("ErrorMsg", "Strategy network error"), 10} },
    { eSCRPHTMLLOADERR, {Tr::translate("ErrorMsg", "HTML Loading error"), 0} },
    { eSCRPCLOSEDWEBVIEWERR, {Tr::translate("ErrorMsg", "Manually closed"), 0} },
    { eSCRPWAITINGERR, {Tr::translate("ErrorMsg", "Hanged, waiting too much... enough!"), 10} },
    { eDOWLDOPENFILERR, {Tr::translate("ErrorMsg", "Can't open file for writing"), 0} },
    { eDOWLDNETWORKERR, {Tr::translate("ErrorMsg", "Downloader network error"), 10} },
    { eDOWLDHTTPCODERR, {Tr::translate("ErrorMsg", "HTTP status > 400"), 0} },
    { eDOWLDCONTENTLENGTHERR, {Tr::translate("ErrorMsg", "Invalid content"), 10} },
    { eENDLESSREDIRECT, {Tr::translate("ErrorMsg", "Endless redirect loop"), 0} },
    { eDOWLDUNKWNFILERR, {Tr::translate("ErrorMsg", "Unknown file error"), 0} },
}
{
}

Tr::Translation ErrorCode::getDescription(ERROR_CODES code)
{
    static const Tr::Translation defErrStr = {"ErrorMsg", "no description"};
    auto it = errors_.find(code);
    return (it != errors_.end()) ? it->second.first : defErrStr;
}

int ErrorCode::getTimeout(ERROR_CODES code)
{
    auto it = errors_.find(code);
    return (it != errors_.end()) ? it->second.second : 0;
}

} // namespace utilities
