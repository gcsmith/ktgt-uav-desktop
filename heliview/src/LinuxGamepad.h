// -----------------------------------------------------------------------------
// File:    LinuxGamepad.h
// Authors: Garrett Smith
// Created: 09-17-2010
//
// Linux implementation of the Gamepad interface.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_LINUXGAMEPAD__H_
#define _HELIVIEW_LINUXGAMEPAD__H_

#include <QWidget>
#include <QThread>
#include "Gamepad.h"

#define ANALOG_RANGE    ((1 << 15) - 1)
#define MAX_NAME_LEN    128

class LinuxGamepadThread: public QThread
{
    Q_OBJECT

public:
    LinuxGamepadThread(int fd);
    virtual ~LinuxGamepadThread();

    virtual void run();
    virtual void stop();

signals:
    void inputReady(GamepadEvent event, int index, float value);

protected:
    int m_fd;
    bool m_active;
};

class LinuxGamepad: public Gamepad
{
    Q_OBJECT

public:
    LinuxGamepad();
    virtual ~LinuxGamepad();

    virtual bool open(const QString &device);
    virtual void close();

    virtual void start();
    virtual void stop();

    virtual int axisCount() { return m_axes; }
    virtual int buttonCount() { return m_buttons; }
    virtual int driverVersion() { return m_version; }
    virtual const QString &driverName() { return m_name; }

protected:
    LinuxGamepadThread *m_thread;
    QString m_device;
    int m_fd;
    int m_axes;
    int m_buttons;
    int m_version;
    QString m_name;
};

#endif // _HELIVIEW_LINUXGAMEPAD__H_

