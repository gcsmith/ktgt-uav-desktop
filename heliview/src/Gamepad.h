// -----------------------------------------------------------------------------
// File:    Gamepad.h
// Authors: Garrett Smith
//
// Abstract interface for a gamepad device.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_GAMEPAD__H_
#define _HELIVIEW_GAMEPAD__H_

#include <QObject>

enum GamepadEvent
{
    GP_EVENT_BUTTON,
    GP_EVENT_AXIS
};

class Gamepad: public QObject
{
    Q_OBJECT

public:
    virtual bool open(const QString &device) = 0;
    virtual void close() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual int axisCount() = 0;
    virtual int buttonCount() = 0;
    virtual int driverVersion() = 0;
    virtual const QString &driverName() = 0;

signals:
    void inputReady(GamepadEvent event, int index, float value);
};

Gamepad *CreateGamepad(void);

#endif // _HELIVIEW_GAMEPAD__H_

