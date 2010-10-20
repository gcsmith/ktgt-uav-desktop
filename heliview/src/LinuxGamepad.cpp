// -----------------------------------------------------------------------------
// File:    LinuxGamepad.cpp
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#include <QDebug>
#include <QMetaType>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <fcntl.h>
#include <cassert>
#include "LinuxGamepad.h"
#include "Logger.h"
#include "Utility.h"

using namespace std;

// -----------------------------------------------------------------------------
LinuxGamepad::LinuxGamepad()
: m_thread(NULL), m_device(""), m_fd(-1), m_axes(0), m_buttons(0), m_version(0)
{
}

// -----------------------------------------------------------------------------
LinuxGamepad::~LinuxGamepad()
{
    close();
}

// -----------------------------------------------------------------------------
bool LinuxGamepad::open(const QString &device)
{
    char driver_name[MAX_NAME_LEN];

    m_device = device;
    if (0 == m_device.length())
    {
        // set to a reasonable default
        m_device = QString("/dev/input/js0");
    }

    const char *sdev = m_device.toStdString().c_str();
    m_fd = ::open(sdev, O_RDONLY);
    if (m_fd < 0)
    {
        qDebug() << "failed to open device" << sdev;
        return false;
    }


    ioctl(m_fd, JSIOCGAXES, &m_axes);
    ioctl(m_fd, JSIOCGBUTTONS, &m_buttons);
    ioctl(m_fd, JSIOCGVERSION, &m_version);
    ioctl(m_fd, JSIOCGNAME(MAX_NAME_LEN), &driver_name);

    m_name = QString(driver_name);
    return true;
}

// -----------------------------------------------------------------------------
void LinuxGamepad::close()
{
    if (m_fd >= 0)
    {
        // terminate the polling thread and close the js* file handle
        stop();
        ::close(m_fd);
        m_fd = -1;
    }
}

// -----------------------------------------------------------------------------
void LinuxGamepad::start()
{
    if (!m_thread)
    {
        // kick off the device io thread
        m_thread = new LinuxGamepadThread(m_fd);
        connect(m_thread, SIGNAL(inputReady(GamepadEvent, int, float)),
                this, SIGNAL(inputReady(GamepadEvent, int, float)));
        m_thread->start();
    }
}

// -----------------------------------------------------------------------------
void LinuxGamepad::stop()
{
    if (m_thread)
    {
        m_thread->stop();
        m_thread->wait();
        SafeDelete(m_thread);
    }
}

// -----------------------------------------------------------------------------
LinuxGamepadThread::LinuxGamepadThread(int fd)
: m_fd(fd), m_active(true)
{
}

// -----------------------------------------------------------------------------
LinuxGamepadThread::~LinuxGamepadThread()
{
    m_active = false;
}

// -----------------------------------------------------------------------------
void LinuxGamepadThread::run()
{
    struct js_event event;
    struct timeval tv;
    fd_set rdset;

    for (;;)
    {
        // wait until joydev data is available
        for (;;)
        {
            // kill the thread if requested
            if (!m_active)
                return;

            FD_ZERO(&rdset);
            FD_SET(m_fd, &rdset);
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            int rc = select(m_fd + 1, &rdset, NULL, NULL, &tv);
            if (rc < 0)
                Logger::err("select call failed in LinuxGamepadThread\n");
            else if (rc > 0)
                break;
        }

        // attempt to read the joydev data
        if (read(m_fd, &event, sizeof(struct js_event)) != sizeof(event)) {
            Logger::err("failed to read from joystick\n");
            continue;
        }

        GamepadEvent gpe;
        float val;

        switch (event.type)
        {
            case JS_EVENT_BUTTON:
                gpe = GP_EVENT_BUTTON;
                val = (event.value == 0) ? -1.0f : 1.0f;
                break;
            case JS_EVENT_AXIS:
                gpe = GP_EVENT_AXIS;
                val = (float)event.value / ANALOG_RANGE;
                break;
            default:
                continue;
        }

        emit inputReady(gpe, event.number, val);
    }
}

// -----------------------------------------------------------------------------
void LinuxGamepadThread::stop()
{
    m_active = false;
}

// -----------------------------------------------------------------------------
Gamepad *CreateGamepad(void)
{
    qRegisterMetaType<GamepadEvent>("GamepadEvent");
    return new LinuxGamepad();
}

