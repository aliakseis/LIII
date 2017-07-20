#pragma once

#include <map>

#include "utilities/singleton.h"
#include "translation.h"

namespace utilities
{

class ErrorCode : public Singleton<ErrorCode>
{
friend class Singleton<ErrorCode>;
public:
    enum ERROR_CODES
    {
        eNOTERROR,
        eDOWLDOPENFILERR,
        eDOWLDNETWORKERR,
        eDOWLDHTTPCODERR,
        eDOWLDCONTENTLENGTHERR,
        eENDLESSREDIRECT,
        eDOWLDUNKWNFILERR
    };

    Tr::Translation getDescription(ERROR_CODES code);
    int getTimeout(ERROR_CODES code);

private:
    ErrorCode();

    std::map<ERROR_CODES, std::pair<Tr::Translation, int>> errors_;
};

static Tr::Translation NETWORK_ERROR_NO_MSG        = Tr::translate("ErrorMsg", "Network error: %1");
static Tr::Translation NETWORK_ERROR_HTTP_STATUS   = Tr::translate("ErrorMsg", "HTTP status: %1");

} // namespace utilities
