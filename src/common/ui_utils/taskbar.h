#pragma once

#include <qglobal.h>
#include "qwindowdefs.h"

#if defined(Q_OS_WIN)

struct ITaskbarList3;

namespace ui_utils
{

class TaskBar
{
public:
    TaskBar();
    ~TaskBar();

    static unsigned int InitMessage();
    void Init(WId main);
    void Uninit();
    void setMaximum(int m) { m_maximum = m; }

    void setProgress(int progress);
    void unsetProgress();
    void setError();
    void setPaused();
    void setNormal();
private:
    static unsigned int m_taskBarCreatedMsg;
    ITaskbarList3* m_taskBar;
    bool isProgressInitialized;
    WId m_main;
    int m_maximum;
};

}

#endif
