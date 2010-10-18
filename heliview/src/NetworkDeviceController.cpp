// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.cpp
// Authors: Garrett Smith
//
// Network device interface implementation.
// -----------------------------------------------------------------------------

#include <QDebug>
#include <iostream>
#include <algorithm>
#include "NetworkDeviceController.h"
#include "Utility.h"
#include "uav_protocol.h"

using namespace std;

const char *NetworkDeviceController::m_description = "Network description";
const bool NetworkDeviceController::m_takesDevice = true;

// -----------------------------------------------------------------------------
NetworkDeviceController::NetworkDeviceController(const QString &device)
: m_device(device), m_telem_timer(NULL), m_mjpeg_timer(NULL), m_blocksz(0),
  m_state(STATE_AUTONOMOUS), m_track(QColor(159, 39, 100), 10, 20)
{
    m_telem_timer = new QTimer(this);
    connect(m_telem_timer, SIGNAL(timeout()), this, SLOT(onTelemetryTick()));

    m_mjpeg_timer = new QTimer(this);
    connect(m_mjpeg_timer, SIGNAL(timeout()), this, SLOT(onVideoTick()));

    m_controller_timer = new QTimer(this);
    connect(m_controller_timer, SIGNAL(timeout()), this, 
            SLOT(onControllerTick()));

    m_throttle_timer = new QTimer(this);
    connect(m_throttle_timer, SIGNAL(timeout()), this, SLOT(onThrottleTick()));

    m_ctl.alt = 0.0f;
    m_ctl.pitch = 0.0f;
    m_ctl.roll = 0.0f;
    m_ctl.yaw = 0.0f;
}

// -----------------------------------------------------------------------------
NetworkDeviceController::~NetworkDeviceController()
{
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::open()
{
    // set some reasonable default values
    QString address("192.168.1.100");
    int portnum = 8090;

    // was an address specified?
    if (m_device.length())
    {
        // if so, split it into ip address and port number
        QStringList ssplit = m_device.split(":", QString::SkipEmptyParts);
        if (ssplit.size() != 2)
        {
            log("invalid address format (addr:port)");
            return false;
        }

        address = ssplit[0];
        portnum = ssplit[1].toInt();
    }

    log(QString("creating network device %1:%2").arg(address).arg(portnum));

    // attempt to create and connect to the network socket
    m_sock = new QTcpSocket();
    connect(m_sock, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
    connect(m_sock, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(onSocketError(QAbstractSocket::SocketError)));

    // attempt to connect to the specified address:port
    m_sock->connectToHost(address, portnum);
    if (!m_sock->waitForConnected()) {
        emit connectionStatusChanged(QString("Connection failed"), false);
        log("socket connection to host timed out");
        return false;
    }

    emit connectionStatusChanged(QString("connected to ") + m_device, true);
    log(QString("connected to device ") + m_device);
    return true;
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::close()
{
    if (m_sock)
    {
        // disconnect the socket, wait for completion
        log(QString("disconnecting from ") + m_device);
        m_sock->disconnectFromHost();
        m_sock->waitForDisconnected();
        SafeDelete(m_sock);
        shutdown();
        log("successfully disconnected");
    }
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::sendPacket(uint32_t command)
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    uint32_t cmd_buffer[] = { command, PKT_BASE_LENGTH };
    stream.writeRawData((char *)cmd_buffer, PKT_BASE_LENGTH);
    return m_sock->waitForBytesWritten();
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::sendPacket(uint32_t *buffer, int length)
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    stream.writeRawData((char *)buffer, length);
    return m_sock->waitForBytesWritten();
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestTakeoff()
{
    return sendPacket(CLIENT_REQ_TAKEOFF);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestLanding()
{
    return sendPacket(CLIENT_REQ_LANDING);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestManualOverride()
{
    uint32_t cmd_buffer[4];
    cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
    cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
    cmd_buffer[PKT_VCM_TYPE] = VCM_TYPE_RADIO;
    cmd_buffer[PKT_VCM_AXES] = VCM_AXIS_ALL;
    return sendPacket(cmd_buffer, PKT_VCM_LENGTH);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestAutonomous()
{
    uint32_t cmd_buffer[4];
    cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
    cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
    cmd_buffer[PKT_VCM_TYPE] = VCM_TYPE_AUTO;
    cmd_buffer[PKT_VCM_AXES] = VCM_AXIS_ALL;
    return sendPacket(cmd_buffer, PKT_VCM_LENGTH);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestKillswitch()
{
    uint32_t cmd_buffer[4];
    cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
    cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
    cmd_buffer[PKT_VCM_TYPE] = VCM_TYPE_KILL;
    cmd_buffer[PKT_VCM_AXES] = VCM_AXIS_ALL;
    return sendPacket(cmd_buffer, PKT_VCM_LENGTH);
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onTelemetryTick()
{
    if (!sendPacket(CLIENT_REQ_TELEMETRY))
    {
        // report the failure
        log("failed to send telemetry request");
    }
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onVideoTick()
{
    if (!sendPacket(CLIENT_REQ_MJPG_FRAME))
    {
        // report the failure
        log("failed to send mjpg frame request");
    }
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onControllerTick()
{
    // Memory of previous mixed controller values
    static float prev_pitch = 0.0f;
    static float prev_roll = 0.0f;
    static float prev_yaw = 0.0f;

    if (STATE_MIXED_CONTROL == m_state)
    {
        uint32_t cmd_buffer[24];
        union { int i; float f; } temp;
        char send = 0;
      
        cmd_buffer[PKT_COMMAND] = CLIENT_REQ_FLIGHT_CTL;
        cmd_buffer[PKT_LENGTH]  = PKT_MCM_LENGTH;
      
        // altitude
        temp.f = m_ctl.alt;
        if ((m_axes & AXIS_ALT) && (temp.f != m_prev_alt))
        {
            m_prev_alt = m_ctl.alt;
            //send = 1;
        }
        cmd_buffer[PKT_MCM_AXIS_ALT] = temp.i;

        // pitch
        temp.f = m_ctl.pitch;
        if ((m_axes & AXIS_PITCH) && (temp.f != prev_pitch))
        {
            prev_pitch = m_ctl.pitch;
            send = 1;
        }
        cmd_buffer[PKT_MCM_AXIS_PITCH] = temp.i;

        // roll
        temp.f = m_ctl.roll;
        if ((m_axes & AXIS_ROLL) && (temp.f != prev_roll))
        {
            prev_roll = m_ctl.roll;
            send = 1;
        }
        cmd_buffer[PKT_MCM_AXIS_ROLL] = temp.i;

        // yaw
        temp.f = m_ctl.yaw;
        if ((m_axes & AXIS_YAW) && (temp.f != prev_yaw))
        {
            prev_yaw = m_ctl.yaw;
            send = 1;
        }
        cmd_buffer[PKT_MCM_AXIS_YAW] = temp.i;

        if (send && !sendPacket(cmd_buffer, PKT_MCM_LENGTH))
        {
            // report the failure
            log("failed to send flight control request");
        }
        else if (send)
        {
            fprintf(stderr, "Sent ALT:   %f\n", m_ctl.alt);
            fprintf(stderr, "Sent PITCH: %f\n", m_ctl.pitch);
            fprintf(stderr, "Sent ROLL:  %f\n", m_ctl.roll);
            fprintf(stderr, "Sent YAW:   %f\n\n", m_ctl.yaw);
        }
    }
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onThrottleTick()
{
    float value = m_ctl.alt;

    if ((value < m_prev_alt) && (m_prev_alt > 0.0f) && (value > 0.0f))
    {
        // joystick is going from postive to zero
        // do nothing
    }
    else if ((value > m_prev_alt) && (m_prev_alt < 0.0f) && (value < 0.0f))
    {
        // joystick is going from negative to zero
        // do nothing
    }
    else if ((value >= 0.1f) || (value <= -0.1f))
    {
        // m_prev_alt == value
        // the user hasn't moved the joystick since the last poll, so tell
        // the server to keep incrementing the throttle pwm
        uint32_t cmd_buffer[32];

        cmd_buffer[PKT_COMMAND] = CLIENT_REQ_FLIGHT_CTL;
        cmd_buffer[PKT_LENGTH]  = PKT_MCM_LENGTH;
        cmd_buffer[PKT_MCM_AXIS_ALT]   = *(uint32_t *)&m_ctl.alt;
        cmd_buffer[PKT_MCM_AXIS_PITCH] = *(uint32_t *)&m_ctl.pitch;
        cmd_buffer[PKT_MCM_AXIS_ROLL]  = *(uint32_t *)&m_ctl.roll;
        cmd_buffer[PKT_MCM_AXIS_YAW]   = *(uint32_t *)&m_ctl.yaw;

        fprintf(stderr, "acting on throttle event signal %f\n", m_ctl.alt);
        if (!sendPacket(cmd_buffer, PKT_MCM_LENGTH))
        {
            // report the failure
            log("failed to send throttle event"); 
        }
    }
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onSocketReadyRead()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int32_t rssi, altitude, battery, aux, framesz;
    float x, y, z;

    if (0 == m_blocksz)
    {
        if ((size_t)m_sock->bytesAvailable() < PKT_BASE_LENGTH)
            return;

        uint32_t header[PKT_BASE];
        stream.readRawData((char *)&header[0], PKT_BASE_LENGTH);
        m_blocksz = header[PKT_LENGTH] - PKT_BASE_LENGTH;
        // fprintf(stderr, "packet cmd %d size %d\n", header[0], header[1]);

        // reserve enough room for the entire message, copy the header over
        m_buffer.reserve(header[PKT_LENGTH]);
        memcpy(&m_buffer[0], &header[0], PKT_BASE_LENGTH);
    }

    if (m_sock->bytesAvailable() < m_blocksz)
        return;

    // interpret the packet
    uint32_t *packet = (uint32_t *)&m_buffer[0];
    stream.readRawData((char *)&packet[PKT_BASE], m_blocksz);
    m_blocksz = 0;

    switch (packet[0])
    {
    case SERVER_REQ_IDENT:
        log("received SERVER_REQ_IDENT: sending response...");
        packet[PKT_COMMAND]     = CLIENT_ACK_IDENT;
        packet[PKT_LENGTH]      = PKT_RCI_LENGTH;
        packet[PKT_RCI_MAGIC]   = IDENT_MAGIC;
        packet[PKT_RCI_VERSION] = IDENT_VERSION;
        stream.writeRawData((char *)&packet[0], PKT_RCI_LENGTH);
        m_telem_timer->start(50); // begin requesting telemetry
        m_mjpeg_timer->start(67); // begin requesting frames
        m_controller_timer->start(50); // begin requesting flight control
        break;
    case SERVER_ACK_IGNORED:
        log("received SERVER_ACK_IGNORED");
        break;
    case SERVER_ACK_TAKEOFF:
        log("received SERVER_ACK_TAKEOFF");
        break;
    case SERVER_ACK_LANDING:
        log("received SERVER_ACK_LANDING");
        break;
    case SERVER_ACK_TELEMETRY:
        x = *(float *)&packet[PKT_VTI_YAW];
        y = *(float *)&packet[PKT_VTI_PITCH];
        z = *(float *)&packet[PKT_VTI_ROLL];
       
#if 0
        fprintf(stderr, "Yaw   angle = %f, %f\n", x, x * 180 / 3.14159);
        fprintf(stderr, "Pitch angle = %f, %f\n", y, y * 180 / 3.14159);
        fprintf(stderr, "Roll  angle = %f, %f\n", z, z * 180 / 3.14159);
#endif

        rssi     = packet[PKT_VTI_RSSI];
        altitude = packet[PKT_VTI_ALT];
        battery  = packet[PKT_VTI_BATT];
        aux      = packet[PKT_VTI_AUX];
        emit telemetryReady(-z, -y, x, altitude, rssi, battery, aux);
        break;
    case SERVER_ACK_MJPG_FRAME:
        framesz = packet[PKT_LENGTH] - PKT_MJPG_LENGTH;
        emit videoFrameReady((const char *)&packet[PKT_MJPG_IMG], (size_t)framesz);
        break;
    case SERVER_UPDATE_CTL_MODE:
        // if current control mode was mixed, stop the throttle timer
        if (m_state == STATE_MIXED_CONTROL)
            m_throttle_timer->stop();

        switch (packet[PKT_VCM_TYPE])
        {
        case VCM_TYPE_RADIO:
            log("received UPDATE_CTL_MODE: switching to radio mode");
            m_state = STATE_RADIO_CONTROL;
            break;
        case VCM_TYPE_AUTO:
            log("received UPDATE_CTL_MODE: switching to autonomous mode");
            m_state = STATE_AUTONOMOUS;
            break;
        case VCM_TYPE_MIXED:
            log("received UPDATE_CTL_MODE: switching to mixed mode");
            m_state = STATE_MIXED_CONTROL;
            m_prev_alt = 0.0f;
            m_throttle_timer->start(50);
            break;
        case VCM_TYPE_KILL: 
            log("received UPDATE_CTL_MODE: switching to killed mode");
            m_state = STATE_KILLED;
            break;
        case VCM_TYPE_LOCKOUT:
            log("received UPDATE_CTL_MODE: switching to lockout mode");
            m_state = STATE_LOCKOUT;
            break;
        default:
            log("received UPDATE_CTL_MODE: <!!!> invalid VCM_TYPE <!!!>");
            m_state = STATE_KILLED;
            break;
        }

        m_axes = 0;
        if (packet[PKT_VCM_AXES] & VCM_AXIS_ALT)   m_axes |= AXIS_ALT;
        if (packet[PKT_VCM_AXES] & VCM_AXIS_YAW)   m_axes |= AXIS_YAW;
        if (packet[PKT_VCM_AXES] & VCM_AXIS_PITCH) m_axes |= AXIS_PITCH;
        if (packet[PKT_VCM_AXES] & VCM_AXIS_ROLL)  m_axes |= AXIS_ROLL;
        emit stateChanged((int)m_state);
        break;
    case SERVER_UPDATE_TRACKING:
        emit trackStatusUpdate(
                packet[PKT_CTS_STATE] == CTS_STATE_DETECTED,
                (int)packet[PKT_CTS_X1], (int)packet[PKT_CTS_Y1], 
                (int)packet[PKT_CTS_X2], (int)packet[PKT_CTS_Y2]);
        break;
    default:
        cerr << "unknown server command (" << packet[0] << ")\n";
        break;
    }

}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onSocketDisconnected()
{
    shutdown();
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onSocketError(QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        log("socket error: QAbstractSocket::RemoteHostClosedError");
        break;
    case QAbstractSocket::HostNotFoundError:
        log("socket error: QAbstractSocket::HostNotFoundError");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        log("socket error: QAbstractSocket::ConnectionRefusedError");
        break;
    default:
        log("socket error: unknown or generic failure");
        break;
    }
    shutdown();
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onInputReady(
        GamepadEvent event, int index, float value)
{
    uint32_t cmd_buffer[32];

    if (GP_EVENT_BUTTON == event)
    {
        if ((12 == index) && (value > 0.0))
        {
            int vcm_type;
            if (STATE_MIXED_CONTROL == m_state)
            {
                log("requesting switch to autonomous");
                vcm_type = VCM_TYPE_AUTO;
            }
            else
            {
                log("requesting switch to mixed mode");
                vcm_type = VCM_TYPE_MIXED;
            }

            // request server to set new control mode
            cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
            cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
            cmd_buffer[PKT_VCM_TYPE] = vcm_type;
            cmd_buffer[PKT_VCM_AXES] = VCM_AXIS_ALL;
            sendPacket(cmd_buffer, PKT_VCM_LENGTH);
        }
        else if ((STATE_MIXED_CONTROL == m_state) && (value > 0.0) && 
                (index >= 4) && (index <= 7))
        {
            // update the mixed mode controlled axes
            int vcm_axes = 0;
            if (m_axes & AXIS_ALT)   vcm_axes |= VCM_AXIS_ALT;
            if (m_axes & AXIS_YAW)   vcm_axes |= VCM_AXIS_YAW;
            if (m_axes & AXIS_PITCH) vcm_axes |= VCM_AXIS_PITCH;
            if (m_axes & AXIS_ROLL)  vcm_axes |= VCM_AXIS_ROLL;

            switch (index)
            {
            case 4: // A
                vcm_axes = BIT_INV(vcm_axes, VCM_AXIS_ALT);
                m_prev_alt = 0.0f;
                if (vcm_axes & VCM_AXIS_ALT)
                    m_throttle_timer->start(50);
                else
                    m_throttle_timer->stop();
                break;
            case 5: // B
                vcm_axes = BIT_INV(vcm_axes, VCM_AXIS_ROLL);
                break;
            case 6: // X
                vcm_axes = BIT_INV(vcm_axes, VCM_AXIS_YAW);
                break;
            case 7: // Y
                vcm_axes = BIT_INV(vcm_axes, VCM_AXIS_PITCH);
                break;
            default:
                log("unknown or unhandled controller button pressed");
                break;
            }

            // tell server which axes are manually controlled by mixed mode
            cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
            cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
            cmd_buffer[PKT_VCM_TYPE] = VCM_TYPE_MIXED;
            cmd_buffer[PKT_VCM_AXES] = vcm_axes;
            sendPacket(cmd_buffer, PKT_VCM_LENGTH);
        }
    }
    else if (GP_EVENT_AXIS == event)
    {
        if (index >= 0 && index <= 3)
        {
            switch (index)
            {
            case 0:
                m_ctl.yaw = value;
                break;
            case 1:
                m_ctl.alt = -value;
                break;
            case 2:
                m_ctl.roll = value;
                break;
            case 3:
                m_ctl.pitch = value;
                break;
            default:
                fprintf(stderr, "NetworkDeviceController: unknown axis");
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onUpdateTrackColor(int r, int g, int b, int ht, int st)
{
    uint32_t cmd_buffer[16];

    m_track.color = QColor(r, g, b);
    if (ht > 0) m_track.ht = ht;
    if (st > 0) m_track.st = st;

    cmd_buffer[PKT_COMMAND] = CLIENT_REQ_TRACK_COLOR;
    cmd_buffer[PKT_LENGTH]  = PKT_TC_LENGTH;

    cmd_buffer[PKT_TC_COLOR_FMT] = TC_COLOR_FMT_RGB;
    cmd_buffer[PKT_TC_CHANNEL_0] = m_track.color.red();
    cmd_buffer[PKT_TC_CHANNEL_1] = m_track.color.green();
    cmd_buffer[PKT_TC_CHANNEL_2] = m_track.color.blue();

    cmd_buffer[PKT_TC_THRESH_0] = m_track.ht;
    cmd_buffer[PKT_TC_THRESH_1] = m_track.st;
    cmd_buffer[PKT_TC_THRESH_2] = 0;

    log(QString("requesting track color [%1 %2 %3] with threshold [%4 %5]")
            .arg(r).arg(g).arg(b).arg(m_track.ht).arg(m_track.st)),

    sendPacket(cmd_buffer, PKT_TC_LENGTH);
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::shutdown()
{
    if (m_telem_timer)
    {
        m_telem_timer->stop();
        m_mjpeg_timer->stop();
        m_controller_timer->stop();
        emit connectionStatusChanged(m_device + QString(" disconnected"), false);
    }
}

