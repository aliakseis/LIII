#pragma once

namespace utilities
{

// check if application is set to run with operating system
bool isAutorunEnabled();

// register / unregister application to autostart with OS, according to runWithOS argument
// returns true is operation succeeded
bool setAutorun(bool runWithOS = true);

}
