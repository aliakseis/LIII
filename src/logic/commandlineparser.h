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
    QStringList getLinksFromFile(const QString& filename);
    void treatParams(QStringList params) ;

private:
    MainWindow* mainWindow;

    friend class Application;
};

