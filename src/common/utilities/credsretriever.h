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
    void SetCredentials(const Credential& cred) override { m_cred = cred; }
    Credential& cred() { return m_cred; }

private:
    Credential m_cred;
};

} // namespace utilities
