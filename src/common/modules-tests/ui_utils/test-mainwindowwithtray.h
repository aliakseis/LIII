#pragma once

#include <QObject>
#include "ui_utils/mainwindowwithtray.h"

using ui_utils::MainWindowWithTray;


class Test_MainWindowWithTray
	: public QObject
{
	Q_OBJECT

public:
	Test_MainWindowWithTray()
		: mwnd(0) {}

private Q_SLOTS:
	void initTestCase();

	void test_restore();
	void test_restore_states();
	void test_restore_states_from_closed();
	void test_traymenu();;

	void cleanupTestCase();
private:
	MainWindowWithTray* mwnd;
};
