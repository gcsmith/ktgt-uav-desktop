// -----------------------------------------------------------------------------
// File:    DeviceController.h
// Authors: Garrett Smith
// Created: 08-24-2010
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
    STATE_LOCKOUT,
    STATE_DISCONNECTED,
};

enum SignalFilters
{
    FILTER_ORIENTATION,
    FILTER_ALTITUDE,
    FILTER_BATTERY,
};

struct TrackSettings
{
    TrackSettings()
    : color(0, 0, 0), ht(0), st(0), ft(0) { }
    TrackSettings(QColor _color, int _ht, int _st, int _ft)
    : color(_color), ht(_ht), st(_st), ft(_ft) { }
    QColor color;
    int ht, st, ft;
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

    virtual bool requestDeviceControls() const;
    virtual bool requestTakeoff() const;
    virtual bool requestLanding() const;
    virtual bool requestManualOverride() const;
    virtual bool requestAutonomous() const;
    virtual bool requestKillswitch() const;

public slots:
    virtual void onInputReady(GamepadEvent event, int index, float value);
    virtual void onUpdateTrackColor(int r, int g, int b, int ht, int st, int ft);
    virtual void onUpdateDeviceControl(int id, int value);

signals:
    void telemetryReady(float x, float y, float z,
                        int alt, int rssi, int batt, int aux);
    void connectionStatusChanged(const QString &text, bool status);
    void videoFrameReady(const char *data, size_t length);
    void trackStatusUpdate(bool track, int x1, int y1, int x2, int y2);
    void deviceControlUpdate(const QString &name, const QString &type,
            int id, int minimum, int maximum, int step, int default_value, 
            int current_value) const;
    void deviceMenuUpdate(const QString &name, int id, int index) const;
    void stateChanged(int state) const;
    void takeoff();
    void landing();
};

DeviceController *CreateDeviceController(
        const QString &name,
        const QString &device);

#endif // _HELIVIEW_DEVICECONTROLLER__H_

