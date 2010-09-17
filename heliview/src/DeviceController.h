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
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual QString device() = 0;

signals:
    void telemetryReady(float x, float y, float z, int alt, int rssi, int batt);
    void connectionStatusChanged(const QString &text, bool status);
};

DeviceController *CreateDeviceController(
        const QString &name,
        const QString &device);

#endif // _HELIVIEW_DEVICECONTROLLER__H_

