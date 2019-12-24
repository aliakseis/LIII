#include "mainwindowwithtray.h"
#include "utilities/translation.h"

#include <QApplication>
#include <QEvent>
#include <QCloseEvent>
#include <QAction>
#include <QMenu>


#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace
{
utilities::Tr::Translation appExitTr = utilities::Tr::translate("MainWindow", "Exit");
utilities::Tr::Translation appShowTr = utilities::Tr::translate("MainWindow", "Show %1");
}

namespace ui_utils
{

MainWindowWithTray::MainWindowWithTray(QWidget*  /*parent*/, QIcon const& icon, utilities::Tr::Translation const& projFullNameTr)
    : m_tray(nullptr)
    , m_isExiting(false)
{
    createTrayMenu(icon, projFullNameTr);
}

MainWindowWithTray::~MainWindowWithTray()
{
    if (m_tray)
    {
        m_tray->hide();
    }
}

void MainWindowWithTray::trayAction(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::MiddleClick:
    case QSystemTrayIcon::Trigger:
    {
        restore();
        break;
    }
    case QSystemTrayIcon::Context:
    {
        emit trayMenuShouldBeUpdated();
        break;
    }
    default:
        ;
    }
}

void MainWindowWithTray::closeApp()
{
    m_isExiting = true;
    QApplication::closeAllWindows();
    QCoreApplication::exit(0);
}

void MainWindowWithTray::setVisible(bool visible)
{
#ifdef  Q_OS_WIN
    if (visible)
    {
        ShowWindow((HWND)internalWinId(), SW_SHOW);
        setAttribute(Qt::WA_WState_Hidden, false);
        setAttribute(Qt::WA_WState_Visible, true);
        QMainWindow::setVisible(true);
    }
    else
    {
        ShowWindow((HWND)internalWinId(), SW_HIDE);
        setAttribute(Qt::WA_WState_Hidden, true);
        setAttribute(Qt::WA_WState_Visible, false);
        if (!m_isExiting)
        {
            showHideNotify();
        }
    }
#else
    QMainWindow::setVisible(visible);
#endif

}


void MainWindowWithTray::closeEvent(QCloseEvent* event)
{
    hide();

#if !defined(Q_OS_MAC)
    event->ignore();
#else
    QMainWindow::closeEvent(event);
#endif
}

void MainWindowWithTray::showHideNotify() {}

void MainWindowWithTray::createTrayMenu(QIcon const& icon, utilities::Tr::Translation const& projFullNameTr)
{
    QApplication::setWindowIcon(icon);
#if !defined(Q_OS_MAC)
    m_tray = new QSystemTrayIcon(this);
    utilities::Tr::SetTr(m_tray, &QSystemTrayIcon::setToolTip, projFullNameTr);
    m_tray->setIcon(icon);
    VERIFY(connect(m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(trayAction(QSystemTrayIcon::ActivationReason))));
    VERIFY(connect(m_tray, SIGNAL(messageClicked()), SLOT(restore())));
    m_tray->setContextMenu(new QMenu(this)); // TODO: doubt if need this
    m_tray->show();
#endif
}

void MainWindowWithTray::addTrayMenuItem(QAction* action)
{
#if !defined(Q_OS_MAC)
    m_tray->contextMenu()->addAction(action);
#endif
}

QAction* MainWindowWithTray::addTrayMenuItem(TrayMenu::ItemType itemType)
{
    QAction* action = nullptr;
#if !defined(Q_OS_MAC)
    switch (itemType)
    {
    case TrayMenu::Exit:
        action = createAction("actionExit", appExitTr, QKeySequence::Quit, SLOT(closeApp()));
        m_tray->contextMenu()->addAction(action);
        break;
    case TrayMenu::Show:
    {
        action = createAction("actionShow", appShowTr, QKeySequence::UnknownKey, SLOT(restore()));
        action->setIcon(QApplication::windowIcon());
        action->setIconVisibleInMenu(true);
        m_tray->contextMenu()->addAction(action);
    }
    break;
    case TrayMenu::Separator:
        action = m_tray->contextMenu()->addSeparator();
        break;
    default:
        Q_ASSERT(false && "not implemented item type");
    }
#endif

    return action;
}

void MainWindowWithTray::showTrayMessage(const QString& message)
{
#ifndef Q_OS_MAC
    Q_ASSERT(m_tray);
    m_tray->showMessage(windowTitle(), message);
#endif
}

void MainWindowWithTray::restore()
{
#ifdef Q_OS_WIN
    // workaround for semi-maximized windows on Windows Vista and newer
    if (windowState() & Qt::WindowMinimized)
    {
        ShowWindow((HWND)internalWinId(), SW_RESTORE);
    }
    else
    {
        setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    }
#else
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
#endif
    show();
    raise();
    QApplication::alert(this);
}

QAction* MainWindowWithTray::createAction(const char* actName, utilities::Tr::Translation translation, const QKeySequence& shortcut, const char* onTriggered)
{
    auto* action = new QAction(this);
    action->setObjectName(actName);
    action->setAutoRepeat(false);
    action->setIconVisibleInMenu(false);
    action->setShortcut(shortcut);
    utilities::Tr::SetTr(action, &QAction::setText, translation, PROJECT_NAME);
    VERIFY(connect(action, SIGNAL(triggered()), onTriggered));
    return action;
}

}//namespace ui_utils
