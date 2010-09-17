// -----------------------------------------------------------------------------
// File:    SimulatedDeviceController.h
// Authors: Garrett Smith
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
    virtual QString device() { return m_device; }

public slots:
    void onSimulateTick();
    void onSimulateNoiseTick();

protected:
    QString m_device;
    float   m_time;
    float   m_yaw, m_pitch, m_roll;
};

#endif // _HELIVIEW_SIMULATEDDEVICECONTROLLER__H_


