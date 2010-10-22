// -----------------------------------------------------------------------------
// File:    DeviceController.cpp
// Authors: Garrett Smith
// Created: 08-24-2010
//
// Abstract interface for device communication.
// -----------------------------------------------------------------------------

#include "NetworkDeviceController.h"
#include "SerialDeviceController.h"
#include "SimulatedDeviceController.h"
#include "Logger.h"

using namespace std;

// -----------------------------------------------------------------------------
DeviceState DeviceController::currentState() const
{
    return STATE_AUTONOMOUS;
}

// -----------------------------------------------------------------------------
int DeviceController::currentAxes() const
{
    return AXIS_ALL;
}

// -----------------------------------------------------------------------------
TrackSettings DeviceController::currentTrackSettings() const
{
    return TrackSettings(QColor(255, 255, 255), 10, 10, 10);
}

// -----------------------------------------------------------------------------
bool DeviceController::requestDeviceControls() const
{
    Logger::info("DeviceController::requestDeviceControls()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestTakeoff() const
{
    Logger::info("DeviceController::requestTakeoff()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestLanding() const
{
    Logger::info("DeviceController::requestLanding()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestManualOverride() const
{
    Logger::info("DeviceController::requestManualOverride()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestAutonomous() const
{
    Logger::info("DeviceController::requestAutonomous()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestKillswitch() const
{
    Logger::info("DeviceController::requestKillswitch()\n");
    return true;
}

// -----------------------------------------------------------------------------
void DeviceController::onInputReady(GamepadEvent event, int index, float value)
{
    Logger::info(tr("DeviceController::onInputReady(%1, %2)\n")
            .arg(index).arg(value));
}

// -----------------------------------------------------------------------------
void DeviceController::onUpdateTrackColor(int r, int g, int b,
        int ht, int st, int ft)
{
    Logger::info(tr("DeviceController::onUpdateTrackColor(%1, %2, %3, %4, "
                "%5, %6)\n").arg(r).arg(g).arg(b).arg(ht).arg(st).arg(ft));
}

// -----------------------------------------------------------------------------
void DeviceController::onUpdateDeviceControl(int id, int value)
{
    Logger::info(tr("DeviceController::onUpdateDeviceControl(%1, %2)\n")
            .arg(id).arg(value));
}

// -----------------------------------------------------------------------------
DeviceController *CreateDeviceController(
        const QString &name,
        const QString &device)
{
    if (name == "network")
        return new NetworkDeviceController(device);
    else if (name == "serial")
        return new SerialDeviceController(device);
    else if (name == "simulated")
        return new SimulatedDeviceController(device);
    else
        return NULL;
}

