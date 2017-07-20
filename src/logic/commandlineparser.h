#pragma once

class QString;

class MainWindow;

void processCommandLine(MainWindow* w);
void processMessageReceived(const QString&);
