// -----------------------------------------------------------------------------
// File:    Logger.cpp
// Authors: Kevin Macksamie, Garrett Smith
// Created: 10-25-2010
//
// Singleton Logger class used to log messages during runtime.
// -----------------------------------------------------------------------------

#include "Logger.h"
#include "Utility.h"

// -----------------------------------------------------------------------------
Logger *Logger::instance()
{
    static Logger logger;
    return &logger;
}

// -----------------------------------------------------------------------------
void Logger::log(int type, const QString &msg)
{
    emit Logger::instance()->updateLog(type, msg);
}

// -----------------------------------------------------------------------------
void Logger::warn(const QString &msg)
{
    emit Logger::instance()->updateLog(LOG_TYPE_WARN, msg);
}

// -----------------------------------------------------------------------------
void Logger::err(const QString &msg)
{
    emit Logger::instance()->updateLog(LOG_TYPE_ERR, msg);
}

// -----------------------------------------------------------------------------
void Logger::info(const QString &msg)
{
    emit Logger::instance()->updateLog(LOG_TYPE_INFO, msg);
}

// -----------------------------------------------------------------------------
void Logger::dbg(const QString &msg)
{
    emit Logger::instance()->updateLog(LOG_TYPE_DBG, msg);
}

// -----------------------------------------------------------------------------
void Logger::extraDebug(const QString &msg)
{
    emit Logger::instance()->updateLog(LOG_TYPE_EXTRADEBUG, msg);
}

// -----------------------------------------------------------------------------
void Logger::fail(const QString &msg)
{
    emit Logger::instance()->updateLog(LOG_TYPE_FAIL, msg);
}

