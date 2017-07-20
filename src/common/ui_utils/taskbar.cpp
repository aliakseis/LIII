#include "taskbar.h"

#if defined(Q_OS_WIN)

#include <windows.h>
#include <tchar.h>
#include <Shobjidl.h>

namespace ui_utils
{

unsigned int TaskBar::m_taskBarCreatedMsg = 0;

unsigned int TaskBar::InitMessage()
{
    m_taskBarCreatedMsg = ::RegisterWindowMessageW(L"TaskbarButtonCreated");
    return m_taskBarCreatedMsg;
}

TaskBar::TaskBar() : m_taskBar(NULL), isProgressInitialized(false), m_main(NULL), m_maximum(100)
{
}

TaskBar::~TaskBar()
{
    Uninit();
}

void TaskBar::Uninit()
{
    if (m_taskBar)
    {
        m_taskBar->Release();
        m_taskBar = NULL;
    }
}

void TaskBar::Init(WId main)
{
    Uninit();

    m_main = main;

    OSVERSIONINFOEX osver = {sizeof(OSVERSIONINFOEX), 6, 1};
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    bool isVersionOk = FALSE != VerifyVersionInfo(
                           &osver,
                           VER_MAJORVERSION | VER_MINORVERSION,
                           dwlConditionMask
                       );

    if (isVersionOk)
    {
        // Let the TaskbarButtonCreated message through the UIPI filter. If we don't
        // do this, Explorer would be unable to send that message to our window if we
        // were running elevated. It's OK to make the call all the time, since if we're
        // not elevated, this is a no-op.
        CHANGEFILTERSTRUCT cfs = { sizeof(CHANGEFILTERSTRUCT) };

        HMODULE user32 = ::LoadLibrary(_T("user32.dll"));
        if (user32 != NULL)
        {
            auto ChangeWindowMessageFilterEx_ = reinterpret_cast<decltype(ChangeWindowMessageFilterEx)*>(GetProcAddress(user32, "ChangeWindowMessageFilterEx"));
            if (ChangeWindowMessageFilterEx_)
            {
                ChangeWindowMessageFilterEx_((HWND)main, m_taskBarCreatedMsg, MSGFLT_ALLOW, &cfs);
            }
            else
            {
                isVersionOk = false;
            }
            ::FreeLibrary(user32);
        }
    }
    if (isVersionOk)
    {
        if (FAILED(::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&m_taskBar)))
        {
            m_taskBar = NULL;
        }
    }
}

void TaskBar::setProgress(int progress)
{
    if (m_taskBar)
    {
        if (progress > 0)
        {
            if (!isProgressInitialized)
            {
                setNormal();
                isProgressInitialized = true;
            }
            m_taskBar->SetProgressValue((HWND)m_main, progress, m_maximum);
        }
        if (progress >= m_maximum)
        {
            unsetProgress();
        }
    }
}

void TaskBar::unsetProgress()
{
    if (m_taskBar)
    {
        m_taskBar->SetProgressState((HWND)m_main, TBPF_NOPROGRESS);
        isProgressInitialized = false;
    }
}

void TaskBar::setError()
{
    if (m_taskBar)
    {
        m_taskBar->SetProgressState((HWND)m_main, TBPF_ERROR);
    }
}

void TaskBar::setPaused()
{
    if (m_taskBar)
    {
        m_taskBar->SetProgressState((HWND)m_main, TBPF_PAUSED);
    }
}

void TaskBar::setNormal()
{
    if (m_taskBar)
    {
        m_taskBar->SetProgressState((HWND)m_main, TBPF_NORMAL);
    }
}

}

#endif
