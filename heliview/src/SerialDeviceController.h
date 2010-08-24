// -----------------------------------------------------------------------------
// File:    SerialDeviceController.h
// Authors: Garrett Smith
//
// Serial device interface declaration.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_SERIALDEVICECONTROLLER__H_
#define _HELIVIEW_SERIALDEVICECONTROLLER__H_

#include "DeviceController.h"

class SerialDeviceController: public DeviceController
{
    Q_OBJECT

public:
    SerialDeviceController();
    virtual ~SerialDeviceController();

    virtual bool open(const QString &device);
    virtual void close();
};

#endif // _HELIVIEW_SERIALDEVICECONTROLLER__H_

