// -----------------------------------------------------------------------------
// File:    SerialDeviceController.cpp
// Authors: Garrett Smith
//
// Serial device interface implementation.
// -----------------------------------------------------------------------------

#include <QDebug>
#include "SerialDeviceController.h"

SerialDeviceController::SerialDeviceController()
{
}

SerialDeviceController::~SerialDeviceController()
{
}

bool SerialDeviceController::open(const QString &device)
{
    qDebug() << "creating serial device '" << device << "'\n";
    return true;
}

void SerialDeviceController::close()
{
}

