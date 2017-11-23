#pragma once

#include <QString>


namespace utilities
{

struct Credential
{
    QString host, login, password;
    bool persistent = true;
};

} // namespace utilities