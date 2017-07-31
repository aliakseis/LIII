#pragma once

class MainWindow;

#include <QStringList>

class CommandLineParser
{
public:
    explicit CommandLineParser(MainWindow* w);

    void treatParams(QStringList params);

private:
    MainWindow* mainWindow;
};

