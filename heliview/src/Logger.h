// -----------------------------------------------------------------------------
// File:    Logger.h
// Authors: Kevin Macksamie
//
// Singleton Logger class used to log messages during runtime.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_LOGGER__H_
#define _HELIVIEW_LOGGER__H_

#include <QWidget>

// logging modes
#define LOG_MODE_EXCESSIVE      2  // logs normal and debug plus excessive
#define LOG_MODE_NORMAL_DEBUG   1  // logs normal (below) plus debug messages
#define LOG_MODE_NORMAL         0  // logs fails, errors, warnings, information

// logging types
#define LOG_TYPE_FAIL       0x0001
#define LOG_TYPE_ERR        0x0002
#define LOG_TYPE_WARN       0x0004
#define LOG_TYPE_INFO       0x0008
#define LOG_TYPE_DBG        0x0010
#define LOG_TYPE_EXTRADEBUG 0x0020

class Logger : public QObject
{
    Q_OBJECT

private:
    Logger() {};
    ~Logger() {};

public:
    static Logger *instance();

    static void log(int type, const QString &msg);
    static void warn(const QString &msg);
    static void err(const QString &msg);
    static void info(const QString &msg);
    static void dbg(const QString &msg);
    static void extraDebug(const QString &msg);
    static void fail(const QString &msg);

signals:
    void updateLog(int type, const QString &msg);
};

#endif

