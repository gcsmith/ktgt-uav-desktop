// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.h
// Authors: Garrett Smith
//
// Network device interface declaration.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_NETWORKDEVICECONTROLLER__H_
#define _HELIVIEW_NETWORKDEVICECONTROLLER__H_

#include <QTcpSocket>
#include <QTimer>
#include "DeviceController.h"

class NetworkDeviceController: public DeviceController
{
    Q_OBJECT

public:
    NetworkDeviceController(const QString &device);
    virtual ~NetworkDeviceController();

    virtual bool open();
    virtual void close();
    virtual QString device() { return m_device; }

public slots:
    void onTelemetryTick();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);

protected:
    void shutdown();

    QString     m_device;
    QTcpSocket *m_sock;
    QTimer     *m_timer;
};

#endif // _HELIVIEW_NETWORKDEVICECONTROLLER__H_

