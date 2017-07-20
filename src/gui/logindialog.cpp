#include "logindialog.h"
#include "ui_logindialog.h"
#include "mainwindow.h"
#include "globals.h"

LoginDialog::LoginDialog(QWidget* parent) :
    QDialog(parent),
    m_ui(new Ui::LoginDialog)
{
    m_ui->setupUi(this);
    QIcon icon(PROJECT_ICON);
    setWindowIcon(icon);

    connect(m_ui->buttonBox, SIGNAL(accepted()), SLOT(on_buttonBox_accepted()));
}

LoginDialog::~LoginDialog()
{
    delete m_ui;
}

QString LoginDialog::login()
{
    return m_login;
}

QString LoginDialog::password()
{
    return m_pass;
}

void LoginDialog::on_buttonBox_accepted()
{
    m_login = m_ui->editLogin->text();
    m_pass = m_ui->editPassword->text();
}

void LoginDialog::changeEvent(QEvent* e)
{
    QDialog::changeEvent(e);
    if (e->type() == QEvent::LanguageChange)
        m_ui->retranslateUi(this);
}
