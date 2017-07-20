#pragma once

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QRegExp>

#include "utils.h"

namespace utilities
{

struct Credential
{
    QString host, login, password;
    bool persistent;

    Credential()
        : persistent(true)
    {}
    Credential(const QString& _host, const QString& _login, const QString& _password)
        : host(_host), login(_login), password(_password), persistent(true)
    {}
};

inline bool operator== (const Credential& credential, const QString& host) {return credential.host == host;}

inline bool ValidCredential(const Credential& credential) {return !credential.host.isEmpty();}

template <class CrypterTy>
QString GetEncryptedLoginPassword(const Credential& credential)
{
    QByteArray encrypted;
    VERIFY(CrypterTy::Encrypt(QString("%1\n%2").arg(credential.login, credential.password).toUtf8(), encrypted));
    return QString(encrypted.toBase64());
}

template <class CrypterTy>
bool InitFromEncrypted(const QString& hostname, const QString& login_password_encrypted, Credential* credential)
{
    bool result = !hostname.isEmpty();
    if (result)
    {
        QByteArray decrypted;
        result = CrypterTy::Decrypt(QByteArray::fromBase64(login_password_encrypted.toUtf8()), decrypted);
        if (result)
        {
            QString login_password(decrypted);
            QStringList lp_splitted = login_password.split(QRegExp("[\\n:]"));
            result = lp_splitted.size() == 2 || (lp_splitted = login_password.split('\n')).size() == 2;
            if (result)
            {
                credential->host = hostname;
                credential->login = lp_splitted[0];
                credential->password = lp_splitted[1];
            }
        }
    }
    return result;
}

} // namespace utilities