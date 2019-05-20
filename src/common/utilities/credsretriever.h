#pragma once

#include "credential.h"

namespace utilities
{

class ICredentialsRetriever
{
public:
    virtual void SetCredentials(const Credential& cred) = 0;
};

class CredentialsRetriever : public ICredentialsRetriever
{
public:
    virtual void SetCredentials(const Credential& cred) { m_cred = cred; }
    Credential& cred() { return m_cred; }

private:
    Credential m_cred;
};

} // namespace utilities
