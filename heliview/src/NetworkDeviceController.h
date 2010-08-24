// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.h
// Authors: Garrett Smith
//
// Network device interface declaration.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_NETWORKDEVICECONTROLLER__H_
#define _HELIVIEW_NETWORKDEVICECONTROLLER__H_

#include <QTcpSocket>
#include "DeviceController.h"

class NetworkDeviceController: public DeviceController
{
    Q_OBJECT

public:
    NetworkDeviceController();
    virtual ~NetworkDeviceController();

    virtual bool open(const QString &device);
    virtual void close();

protected:
    QTcpSocket *m_sock;
};

#endif // _HELIVIEW_NETWORKDEVICECONTROLLER__H_

