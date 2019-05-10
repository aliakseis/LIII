#include "credsretriever.h"

#include "logindialog.h"
#include "utils.h"
#include "notify_helper.h"

#include <QApplication>
#include <QMainWindow>
#include <QThread>

namespace {

class LoginDialogHelper : public NotifyHelper
{
public:
    explicit LoginDialogHelper(utilities::ICredentialsRetriever* icr)
        : m_icr(icr)
    {
        moveToThread(QApplication::instance()->thread());
    }

    void slotNoParams() override
    {
        auto* dlg = new LoginDialog(utilities::getMainWindow());

        utilities::Credential cred;
        if (dlg->exec() == QDialog::Accepted)
        {
            cred.login = dlg->login();
            cred.password = dlg->password();
        }

        m_icr->SetCredentials(cred);
        dlg->deleteLater();
        deleteLater();
    }

private:
    utilities::ICredentialsRetriever* m_icr;
};

} // namespace


namespace utilities
{

void needLogin(utilities::ICredentialsRetriever* icr)
{
    auto* helper = new LoginDialogHelper(icr);
    if (helper->thread() == QThread::currentThread())
    {
        helper->slotNoParams();
    }
    else
    {
        QMetaObject::invokeMethod(helper, "slotNoParams", Qt::BlockingQueuedConnection);
    }
}

} // namespace utilities
