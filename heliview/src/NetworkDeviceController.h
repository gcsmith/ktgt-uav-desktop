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
#include <vector>
#include "DeviceController.h"
#include "Utility.h"

class NetworkDeviceController: public DeviceController
{
    Q_OBJECT

public:
    NetworkDeviceController(const QString &device);
    virtual ~NetworkDeviceController();

    virtual bool open();
    virtual void close();
    virtual QString device() { return m_device; }

    bool requestTakeoff();
    bool requestLanding();
    bool requestManualOverride();
    bool requestKillswitch();

public slots:
    void onTelemetryTick();
    void onVideoTick();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onInputReady(GamepadEvent event, int index, float value);

protected:
    bool sendPacket(uint32_t command);
    bool sendPacket(uint32_t *buffer, int length);
    void shutdown();

    QString           m_device;
    QTcpSocket       *m_sock;
    QTimer           *m_telem_timer;
    QTimer           *m_mjpeg_timer;
    uint32_t          m_blocksz;
    uint32_t          m_vcm_type;
    std::vector<char> m_buffer;
};

#endif // _HELIVIEW_NETWORKDEVICECONTROLLER__H_

