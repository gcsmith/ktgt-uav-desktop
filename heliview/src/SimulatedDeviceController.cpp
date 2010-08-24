// -----------------------------------------------------------------------------
// File:    SimulatedDeviceController.cpp
// Authors: Garrett Smith
//
// Simulated device interface implementation.
// -----------------------------------------------------------------------------

#include <QDebug>
#include "SimulatedDeviceController.h"

SimulatedDeviceController::SimulatedDeviceController()
{
}

SimulatedDeviceController::~SimulatedDeviceController()
{
}

bool SimulatedDeviceController::open(const QString &device)
{
    qDebug() << "creating simulated device" << device << "\n";
    return true;
}

void SimulatedDeviceController::close()
{
}

