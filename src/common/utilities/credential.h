#pragma once

#include <QString>


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

} // namespace utilities