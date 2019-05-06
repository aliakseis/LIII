#include "credsretriever.h"

#include "logindialog.h"
#include "utils.h"

#include <QMainWindow>

namespace utilities
{

void needLogin(utilities::ICredentialsRetriever* icr)
{
    LoginDialog dlg(utilities::getMainWindow());
    utilities::Credential cred;
    if (dlg.exec() == QDialog::Accepted)
    {
        cred.login = dlg.login();
        cred.password = dlg.password();
    }
    icr->SetCredentials(cred);
}

} // namespace utilities
