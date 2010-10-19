// -----------------------------------------------------------------------------
// File:    DeviceController.h
// Authors: Garrett Smith
//
// Abstract interface for device communication.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_DEVICECONTROLLER__H_
#define _HELIVIEW_DEVICECONTROLLER__H_

#include <QWidget>
#include "Gamepad.h"

enum DeviceState
{
    STATE_RADIO_CONTROL,
    STATE_MIXED_CONTROL,
    STATE_AUTONOMOUS,
    STATE_KILLED,
    STATE_LOCKOUT
};

struct TrackSettings
{
    TrackSettings(QColor _color, int _ht, int _st)
    : color(_color), ht(_ht), st(_st) { }
    QColor color;
    int ht, st;
};

#define AXIS_ALT    0x01
#define AXIS_YAW    0x02
#define AXIS_PITCH  0x04
#define AXIS_ROLL   0x08
#define AXIS_ALL    0xFF

class DeviceController: public QObject
{
    Q_OBJECT

public:
    virtual bool open() = 0;
    virtual void close() = 0;

    virtual QString device() const = 0;
    virtual QString controllerType() const = 0;
    virtual DeviceState currentState() const;
    virtual int currentAxes() const;
    virtual TrackSettings currentTrackSettings() const;

    virtual bool requestTakeoff();
    virtual bool requestLanding();
    virtual bool requestManualOverride();
    virtual bool requestAutonomous();
    virtual bool requestKillswitch();

public slots:
    virtual void onInputReady(GamepadEvent event, int index, float value) = 0;
    virtual void onUpdateTrackColor(int r, int g, int b, int ht, int st) = 0;

signals:
    void telemetryReady(float x, float y, float z,
                        int alt, int rssi, int batt, int aux);
    void connectionStatusChanged(const QString &text, bool status);
    void videoFrameReady(const char *data, size_t length);
    void trackStatusUpdate(bool track, int x1, int y1, int x2, int y2);
    void stateChanged(int state);
    void takeoff();
    void landing();
};

DeviceController *CreateDeviceController(
        const QString &name,
        const QString &device);

#endif // _HELIVIEW_DEVICECONTROLLER__H_

