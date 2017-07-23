#pragma once

class MainWindow;

#include <QList>
#include <QUrl>
#include <QStringList>

class CommandLineParser
{
public:
    explicit CommandLineParser(MainWindow* w);

private:
    void treatParams(QStringList params) ;

private:
    MainWindow* mainWindow;

    friend class Application;
};

