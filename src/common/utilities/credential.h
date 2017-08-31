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

} // namespace utilities