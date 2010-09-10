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
    SimulatedDeviceController();
    virtual ~SimulatedDeviceController();

    virtual bool open(const QString &device);
    virtual void close();

public slots:
    void onSimulateTick();
    void onSimulateNoiseTick();

protected:
    float m_time;
    float m_yaw, m_pitch, m_roll;
};

#endif // _HELIVIEW_SIMULATEDDEVICECONTROLLER__H_


