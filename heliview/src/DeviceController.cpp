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
bool DeviceController::requestTakeoff()
{
    Logger::info("DeviceController::requestTakeoff()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestLanding()
{
    Logger::info("DeviceController::requestLanding()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestManualOverride()
{
    Logger::info("DeviceController::requestManualOverride()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestAutonomous()
{
    Logger::info("DeviceController::requestAutonomous()\n");
    return true;
}

// -----------------------------------------------------------------------------
bool DeviceController::requestKillswitch()
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
void DeviceController::onUpdateExposure(int automatic, int value)
{
    Logger::info(tr("DeviceController::onUpdateExposure(%1, %2)\n")
            .arg(automatic).arg(value));
}

// -----------------------------------------------------------------------------
void DeviceController::onUpdateFocus(int automatic, int value)
{
    Logger::info(tr("DeviceController::onUpdateFocus(%1, %2)\n")
            .arg(automatic).arg(value));
}

// -----------------------------------------------------------------------------
void DeviceController::onUpdateWhiteBalance(int automatic)
{
    Logger::info(tr("DeviceController::onUpdateWhiteBalance(%1)\n")
            .arg(automatic));
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

