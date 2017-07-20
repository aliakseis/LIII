#ifndef QT_GUI_LIB
#define QT_GUI_LIB
#endif

#ifndef QT_WIDGETS_LIB
#define QT_WIDGETS_LIB
#endif // QT_WIDGETS_LIB
#include "test-mainwindowwithtray.h"
#include "../AutoTest.h"

#include <QtTest>
#include "utilities/translation.h"
#include <QCloseEvent>
#include <QMenu>


namespace
{

utilities::Tr::Translation appName = utilities::Tr::translate("Test_MainWindowWithTray", "Test_MainWindowWithTray");

}

void Test_MainWindowWithTray::initTestCase()
{
	mwnd = new MainWindowWithTray(0, QIcon("qtdemo.ico"), appName);
}

void Test_MainWindowWithTray::cleanupTestCase()
{
	delete mwnd;
}

void Test_MainWindowWithTray::test_created()
{
	MainWindowWithTray* mwnd = new MainWindowWithTray(0, QIcon("qtdemo.ico"), utilities::Tr::translate("Test_MainWindowWithTray", "Test_MainWindowWithTray"));

	QVERIFY(mwnd);

	if (mwnd)
	{
		// VERIFY tray is shown
#if !defined(Q_OS_MAC)
		QVERIFY(mwnd->m_tray && mwnd->m_tray->isVisible());
#endif


		mwnd->show();


		delete mwnd;
	}
	// VERIFY tray is hidden
}

void Test_MainWindowWithTray::test_restore()
{
	Q_ASSERT(mwnd);

	mwnd->hide();

	QVERIFY(mwnd->isHidden());

	mwnd->restore(); // TODO: test mwnd->m_tray click instead

	QVERIFY(mwnd->isVisible());
	QVERIFY((mwnd->windowState() & Qt::WindowMinimized)  == 0);
	QVERIFY((mwnd->windowState() & Qt::WindowFullScreen) == 0);
}

void Test_MainWindowWithTray::test_restore_states()
{
	Q_ASSERT(mwnd);

	// test normal state
	mwnd->showNormal();						VERIFY(mwnd->isVisible());
	mwnd->hide();							VERIFY(mwnd->isHidden());
	mwnd->restore();

	// check it is in normal state
	QVERIFY(mwnd->isVisible());
	QVERIFY((mwnd->windowState() & (Qt::WindowMinimized | Qt::WindowFullScreen | Qt::WindowMaximized))  == 0);
	QVERIFY(mwnd->windowState() & Qt::WindowActive);


	// test maximized state
	mwnd->showMaximized();					VERIFY(mwnd->isVisible());
	mwnd->hide();							VERIFY(mwnd->isHidden());
	mwnd->restore();

	// check it is in maximized state
	QVERIFY(mwnd->isVisible());
	QVERIFY(mwnd->windowState() & Qt::WindowActive);
	QVERIFY((mwnd->windowState() & (Qt::WindowMinimized | Qt::WindowFullScreen)) == 0);
	QVERIFY(mwnd->windowState() & Qt::WindowMaximized);

	// check semi0-maximized state
	// TODO
}

void Test_MainWindowWithTray::test_restore_states_from_closed()
{
	QVERIFY(mwnd);

	//#ifndef DEVELOPER_FEATURES

	// test normal state
	mwnd->showNormal();						VERIFY(mwnd->isVisible());
	mwnd->close();							VERIFY(mwnd->isHidden());
	mwnd->restore();

	// check it is in normal state
	QVERIFY(mwnd->isVisible());
	QVERIFY((mwnd->windowState() & (Qt::WindowMinimized | Qt::WindowFullScreen | Qt::WindowMaximized))  == 0);
	QVERIFY(mwnd->windowState() & Qt::WindowActive);


	// test maximized state
	mwnd->showMaximized();					VERIFY(mwnd->isVisible());
	mwnd->close();							VERIFY(mwnd->isHidden());
	mwnd->restore();

	// check it is in maximized state
	QVERIFY(mwnd->isVisible());
	QVERIFY(mwnd->windowState() & Qt::WindowActive);
	QVERIFY((mwnd->windowState() & (Qt::WindowMinimized | Qt::WindowFullScreen)) == 0);
	QVERIFY(mwnd->windowState() & Qt::WindowMaximized);

	// check semi-maximized state
	// TODO

	//#endif // DEVELOPER_FEATURES
}

void Test_MainWindowWithTray::test_traymenu()
{
	Q_ASSERT(mwnd);
	mwnd->addTrayMenuItem(MainWindowWithTray::TrayMenu::Show);
	mwnd->addTrayMenuItem(MainWindowWithTray::TrayMenu::Separator);
	mwnd->addTrayMenuItem(MainWindowWithTray::TrayMenu::Exit);

	mwnd->hide();
#if !defined(Q_OS_MAC)
	auto acts = mwnd->m_tray->contextMenu()->actions();
	QVERIFY(acts.size() == 3);
	acts[0]->trigger();
	QVERIFY(mwnd->isVisible());
#endif
}


QTEST_MAIN(Test_MainWindowWithTray)
