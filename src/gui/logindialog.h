#pragma once

#include <QDialog>

namespace Ui
{
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

    QString m_login;
    QString m_pass;
public:
    LoginDialog(QWidget* parent = 0);
    ~LoginDialog();
    QString login();
    QString password();
protected:
    void changeEvent(QEvent* e);

private:
    Ui::LoginDialog* m_ui;
private slots:
    void on_buttonBox_accepted();
};
