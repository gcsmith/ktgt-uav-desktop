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
    SerialDeviceController(const QString &device);
    virtual ~SerialDeviceController();

    virtual bool open();
    virtual void close();

    virtual QString device() const { return m_device; }
    virtual QString controllerType() const { return QString("serial"); }
    
    static const char *m_description;
    static const bool m_takesDevice;

public slots:
    void onSerialDataReady();
    void onInputReady(GamepadEvent event, int index, float value) { }
    void onUpdateTrackColor(int r, int g, int b, int ht, int st) { }

protected:
    void processSingleLine(const std::string &line);

    QString            m_device;
    QextSerialPort    *m_serial;
    std::vector<char>  m_buffer;
    unsigned int       m_offset;
    bool               m_validLine;
};

#endif // _HELIVIEW_SERIALDEVICECONTROLLER__H_

