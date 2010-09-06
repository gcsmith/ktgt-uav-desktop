// -----------------------------------------------------------------------------
// File:    SerialDeviceController.h
// Authors: Garrett Smith
//
// Serial device interface declaration.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_SERIALDEVICECONTROLLER__H_
#define _HELIVIEW_SERIALDEVICECONTROLLER__H_

#include <string>
#include <vector>
#include <qextserialport.h>
#include "DeviceController.h"

class SerialDeviceController: public DeviceController
{
    Q_OBJECT

public:
    SerialDeviceController();
    virtual ~SerialDeviceController();

    virtual bool open(const QString &device);
    virtual void close();

public slots:
    void onSerialDataReady();

protected:
    void processSingleLine(const std::string &line);

    QextSerialPort    *m_serial;
    std::vector<char>  m_buffer;
    unsigned int       m_offset;
    bool               m_validLine;
};

#endif // _HELIVIEW_SERIALDEVICECONTROLLER__H_

