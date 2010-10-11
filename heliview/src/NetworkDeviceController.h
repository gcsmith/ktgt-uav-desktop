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

typedef struct _ctrl_signals
{
    float alt, pitch, roll, yaw;
} ctrl_sigs;

class NetworkDeviceController: public DeviceController
{
    Q_OBJECT

public:
    NetworkDeviceController(const QString &device);
    virtual ~NetworkDeviceController();

    virtual bool open();
    virtual void close();
    virtual QString device() { return m_device; }
    virtual DeviceState currentState() { return m_state; }

    bool requestTakeoff();
    bool requestLanding();
    bool requestManualOverride();
    bool requestAutonomous();
    bool requestKillswitch();

public slots:
    void onTelemetryTick();
    void onVideoTick();
    void onControllerTick();
    void onThrottleTick();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onInputReady(GamepadEvent event, int index, float value);
    void onUpdateTrackColor(int r, int g, int b);

protected:
    bool sendPacket(uint32_t command);
    bool sendPacket(uint32_t *buffer, int length);
    void shutdown();

    QString           m_device;
    QTcpSocket       *m_sock;
    QTimer           *m_telem_timer;
    QTimer           *m_mjpeg_timer;
    QTimer           *m_controller_timer;
    QTimer           *m_throttle_timer;
    uint32_t          m_blocksz;
    std::vector<char> m_buffer;
    ctrl_sigs         m_manual_sigs;
    char              m_vcm_axes;
    DeviceState       m_state;
    DeviceState       m_saveState;
};

#endif // _HELIVIEW_NETWORKDEVICECONTROLLER__H_

