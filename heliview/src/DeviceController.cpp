// -----------------------------------------------------------------------------
// File:    DeviceController.cpp
// Authors: Garrett Smith
//
// Abstract interface for device communication.
// -----------------------------------------------------------------------------

#include "NetworkDeviceController.h"
#include "SerialDeviceController.h"
#include "SimulatedDeviceController.h"

using namespace std;

DeviceState DeviceController::currentState()
{
    return STATE_AUTONOMOUS;
}

int DeviceController::currentAxes()
{
    return AXIS_ALL;
}

bool DeviceController::requestTakeoff()
{
    return true;
}

bool DeviceController::requestLanding()
{
    return true;
}

bool DeviceController::requestManualOverride()
{
    return true;
}

bool DeviceController::requestAutonomous()
{
    return true;
}

bool DeviceController::requestKillswitch()
{
    return true;
}

DeviceController *CreateDeviceController(
        const QString &name,
        const QString &device)
{
    if (name == "network")
        return new NetworkDeviceController(device);
    else if (name == "serial")
        return new SerialDeviceController(device);
    else if (name == "sim")
        return new SimulatedDeviceController(device);
    else
        return NULL;
}

