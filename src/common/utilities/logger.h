#pragma once

#include "utilities/singleton.h"

#include <QDebug>
#include <QMutex>
#include <QtGlobal>

class QFile;
class QTextStream;
class QString;

namespace utilities
{

class TheLogger;

class Logger
{
    friend class TheLogger;
public:
    struct LoggerHandler
    {
        virtual bool log(QtMsgType type, const QString& text) = 0;
    };

    Logger();
    ~Logger();

    void log(const QString& text);

    void InitializeLogFile();

    void messageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    QFile* log_file_;
    QTextStream* log_stream_;
    QMutex m_mutex;
    bool m_isRecursive;
    LoggerHandler* m_handler;
};

class TheLogger
{
public:
    static void setWriteToLogFile(bool write_to_log_file);
    static void setLogHandler(Logger::LoggerHandler* handler);
    static void resetLogHandler();
};

} // namespace utilities

using utilities::TheLogger;
