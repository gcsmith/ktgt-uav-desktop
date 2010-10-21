// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.h
// Authors: Garrett Smith, Kevin Macksamie
// Created: 08-24-2010
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

    virtual QString device() const { return m_device; }
    virtual QString controllerType() const { return QString("NetworkDevice"); }
    virtual DeviceState currentState() const { return m_state; }
    virtual int currentAxes() const { return m_axes; }
    virtual TrackSettings currentTrackSettings() const { return m_track; }

    bool requestTakeoff();
    bool requestLanding();
    bool requestManualOverride();
    bool requestAutonomous();
    bool requestKillswitch();
    
    static const char *m_description;
    static const bool m_takesDevice;

public slots:
    void onTelemetryTick();
    void onVideoTick();
    void onControllerTick();
    void onThrottleTick();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onInputReady(GamepadEvent event, int index, float value);
    void onUpdateTrackColor(int r, int g, int b, int ht, int st);

protected:
    void startup();
    void shutdown();
    bool sendPacket(uint32_t command);
    bool sendPacket(uint32_t *buffer, int length);

    QString           m_device;
    QTcpSocket       *m_sock;
    QTimer           *m_telem_timer;
    QTimer           *m_mjpeg_timer;
    QTimer           *m_controller_timer;
    QTimer           *m_throttle_timer;
    uint32_t          m_blocksz;
    std::vector<char> m_buffer;
    ctrl_sigs         m_ctl;
    DeviceState       m_state;
    int               m_axes;
    float             m_prev_alt;
    TrackSettings     m_track;
};

#endif // _HELIVIEW_NETWORKDEVICECONTROLLER__H_

