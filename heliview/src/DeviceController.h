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
    SIGNAL_ORIENTATION,
    SIGNAL_ALTITUDE,
    SIGNAL_AUXILIARY,
    SIGNAL_BATTERY,
    SIGNAL_KP,
    SIGNAL_KI,
    SIGNAL_KD,
    SIGNAL_SP,
};

struct TrackSettings
{
    TrackSettings()
    : color(0, 0, 0), ht(0), st(0), ft(0), fps(0), enabled(0) { }
    TrackSettings(QColor _color, int _ht, int _st, int _ft, int _fps, int _enabled)
    : color(_color), ht(_ht), st(_st), ft(_ft), fps(_fps), enabled(_enabled) { }
    QColor color;
    int ht, st, ft, fps, enabled;
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
    virtual bool getTrackEnabled() const;

    virtual bool requestDeviceControls() const;
    virtual bool requestTrimSettings() const;
    virtual bool requestFilterSettings() const;
    virtual bool requestPIDSettings(int axis) const;

    virtual bool requestTakeoff() const;
    virtual bool requestLanding() const;
    virtual bool requestManualOverride() const;
    virtual bool requestAutonomous() const;
    virtual bool requestKillswitch() const;
    virtual bool requestColors() const;

public slots:
    virtual void onInputReady(GamepadEvent event, int index, float value);
    virtual void onUpdateTrackControlEnable(int track_en);
    virtual void onUpdateColorTrackEnable(int track_en);
    virtual void updateTrackSettings(int r, int g, int b, int ht, int st, int ft, int fps);
    virtual void updateDeviceControl(int id, int value);
    virtual void updateTrimSettings(int axis, int value);
    virtual void updateFilterSettings(int signal, int samples);
    virtual void updatePIDSettings(int axis, int signal, float Kd);

signals:
    void telemetryReady(float yaw, float pitch, float roll, float alt,
            int rssi, int batt, int aux, int cpu);
    void connectionStatusChanged(const QString &text, bool status);
    void videoFrameReady(const char *data, size_t length);
    void trackStatusUpdate(bool en, const QRect &bb, const QPoint &cp);
    void colorValuesUpdate(TrackSettings track);
    void deviceControlUpdated(const QString &name, const QString &type,
            int id, int minimum, int maximum, int step, int default_value, 
            int current_value) const;
    void deviceMenuUpdated(const QString &name, int id, int index) const;
    void trimSettingsUpdated(int yaw, int pitch, int roll, int thro) const;
    void updateTrackControlEnable(int track_en) const;
    void updateColorTrackEnable(int track_en) const;
    void filterSettingsUpdated(int imu, int alt, int aux, int batt) const;
    void pidSettingsUpdated(int axis, float p, float i, float d, float set);
    void controlStateChanged(int state) const;
    void flightStateChanged(int state) const;
    void takeoff();
    void landing();
};

DeviceController *CreateDeviceController(
        const QString &name,
        const QString &device);

#endif // _HELIVIEW_DEVICECONTROLLER__H_

