// -----------------------------------------------------------------------------
// File:    DeviceController.h
// Authors: Garrett Smith
//
// Abstract interface for device communication.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_DEVICECONTROLLER__H_
#define _HELIVIEW_DEVICECONTROLLER__H_

#include <QWidget>

class DeviceController: public QObject
{
    Q_OBJECT

public:
    virtual bool open(const QString &device) = 0;
    virtual void close() = 0;

signals:
    void telemetryReady(float x, float y, float z);
};

DeviceController *CreateDeviceController(const std::string &name);

#endif // _HELIVIEW_DEVICECONTROLLER__H_

