#pragma once

#include <QString>


namespace utilities
{

struct Credential
{
    QString login, password;
    bool persistent = true;
};

} // namespace utilities