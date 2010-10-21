// -----------------------------------------------------------------------------
// File:    SimulatedDeviceController.h
// Authors: Garrett Smith
// Created: 08-24-2010
//
// Simulated device interface declaration.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_SIMULATEDDEVICECONTROLLER__H_
#define _HELIVIEW_SIMULATEDDEVICECONTROLLER__H_

#include "DeviceController.h"

class SimulatedDeviceController: public DeviceController
{
    Q_OBJECT 

public:
    SimulatedDeviceController(const QString &device);
    virtual ~SimulatedDeviceController();

    virtual bool open();
    virtual void close();

    virtual QString device() const { return m_device; }
    virtual QString controllerType() const { return QString("sim"); }
    virtual TrackSettings currentTrackSettings() const { return m_track; }
    
    static const char *m_description;
    static const bool m_takesDevice;

public slots:
    void onSimulateTick();
    void onSimulateNoiseTick();
    void onInputReady(GamepadEvent event, int index, float value);
    void onUpdateTrackColor(int r, int g, int b, int ht, int st, int ft);

protected:
    QString       m_device;
    float         m_time;
    float         m_yaw, m_pitch, m_roll, m_throttle;
    float         m_dyaw, m_dpitch, m_droll, m_dthrottle;
    bool          m_manual;
    TrackSettings m_track;
};

#endif // _HELIVIEW_SIMULATEDDEVICECONTROLLER__H_


