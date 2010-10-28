// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.cpp
// Authors: Garrett Smith, Kevin Macksamie
// Created: 08-24-2010
//
// Network device interface implementation.
// -----------------------------------------------------------------------------

#include "Logger.h"
#include "NetworkDeviceController.h"
#include "Utility.h"
#include "uav_protocol.h"

const char *NetworkDeviceController::m_description = "Network description";
const bool NetworkDeviceController::m_takesDevice = true;

// -----------------------------------------------------------------------------
NetworkDeviceController::NetworkDeviceController(const QString &device)
: m_device(device), m_telem_timer(NULL), m_mjpeg_timer(NULL), m_blocksz(0),
  m_state(STATE_AUTONOMOUS), m_track(QColor(159, 39, 100), 10, 20, 10, 13), 
  m_track_en(false)
{
}

// -----------------------------------------------------------------------------
NetworkDeviceController::~NetworkDeviceController()
{
    close();
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
            Logger::err("NetworkDevice: invalid address format (addr:port)\n");
            return false;
        }

        address = ssplit[0];
        portnum = ssplit[1].toInt();
    }
    Logger::info(tr("NetworkDevice: creating network device %1 port %2\n")
            .arg(address).arg(portnum));

    // attempt to create and connect to the network socket
    m_sock = new QTcpSocket();
    connect(m_sock, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
    connect(m_sock, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(onSocketError(QAbstractSocket::SocketError)));

    // attempt to connect to the specified address:port
    m_sock->connectToHost(address, portnum);
    if (!m_sock->waitForConnected(3000)) {
        emit connectionStatusChanged(QString("Connection failed"), false);
        Logger::warn("NetworkDevice: connection timed out\n");
        return false;
    }

    emit connectionStatusChanged(QString("Connected to ") + m_device, true);
    Logger::info(tr("NetworkDevice: connected to ") + m_device + "\n");

    startup();
    return true;
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::startup()
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
void NetworkDeviceController::close()
{
    if (m_sock)
    {
        // disconnect the socket, wait for completion
        Logger::info("NetworkDevice: disconnecting from host ...\n");
        m_sock->disconnectFromHost();
        m_sock->waitForDisconnected(3000);
        Logger::info("NetworkDevice: disconnected\n");
        SafeDelete(m_sock);
    }
    shutdown();
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::shutdown()
{
    if (m_telem_timer)
    {
        m_telem_timer->stop();
        SafeDelete(m_telem_timer);

        m_mjpeg_timer->stop();
        SafeDelete(m_mjpeg_timer);

        m_controller_timer->stop();
        SafeDelete(m_controller_timer);
    }

    emit connectionStatusChanged(m_device + " disconnected", false);
    emit stateChanged(STATE_DISCONNECTED);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::sendPacket(uint32_t command) const
{
    if (!m_sock || QAbstractSocket::ConnectedState != m_sock->state())
        return false;

    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    uint32_t cmd_buffer[] = { command, PKT_BASE_LENGTH };
    stream.writeRawData((char *)cmd_buffer, PKT_BASE_LENGTH);
    return m_sock->waitForBytesWritten();
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::sendPacket(uint32_t *buffer, int length) const
{
    if (!m_sock || QAbstractSocket::ConnectedState != m_sock->state())
        return false;

    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    stream.writeRawData((char *)buffer, length);
    return m_sock->waitForBytesWritten();
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestDeviceControls() const
{
    return sendPacket(CLIENT_REQ_CAM_DCI);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestColors() const
{    
    return sendPacket(CLIENT_REQ_CAM_COLORS);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestTrimSettings() const
{
    return sendPacket(CLIENT_REQ_GTS);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestFilterSettings() const
{
    return sendPacket(CLIENT_REQ_GFS);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestPIDSettings(int axis) const
{    
    uint32_t cmd_buffer[PKT_GPIDS_LENGTH];
    cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_GPIDS;
    cmd_buffer[PKT_LENGTH]   = PKT_GPIDS_LENGTH;
    cmd_buffer[PKT_GPIDS_AXIS] = axis;
    return sendPacket(cmd_buffer, PKT_GPIDS_LENGTH);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestTakeoff() const
{
    return sendPacket(CLIENT_REQ_TAKEOFF);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestLanding() const
{
    return sendPacket(CLIENT_REQ_LANDING);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestManualOverride() const
{
    uint32_t cmd_buffer[4];
    cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
    cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
    cmd_buffer[PKT_VCM_TYPE] = VCM_TYPE_RADIO;
    cmd_buffer[PKT_VCM_AXES] = VCM_AXIS_ALL;
    return sendPacket(cmd_buffer, PKT_VCM_LENGTH);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestAutonomous() const
{
    uint32_t cmd_buffer[4];
    cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
    cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
    cmd_buffer[PKT_VCM_TYPE] = VCM_TYPE_AUTO;
    cmd_buffer[PKT_VCM_AXES] = VCM_AXIS_ALL;
    return sendPacket(cmd_buffer, PKT_VCM_LENGTH);
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::requestKillswitch() const
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
        Logger::err("NetworkDevice: failed to send telemetry request\n");
    }
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onVideoTick()
{
    if (!sendPacket(CLIENT_REQ_MJPG_FRAME))
    {
        Logger::err("NetworkDevice: failed to send mjpg frame request\n");
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
            Logger::err("NetworkDevice: failed to send flight control request\n");
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
            // report the error and continue on
            Logger::err("NetworkDevice: failed to send throttle event\n");
        }
    }
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onSocketReadyRead()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int32_t rssi, battery, aux, framesz,cpu;
    float x, y, z, h;
    QString log_msg, type;

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
        Logger::info("NetworkDevice: SERVER_REQ_IDENT: sending response...\n");
        packet[PKT_COMMAND]     = CLIENT_ACK_IDENT;
        packet[PKT_LENGTH]      = PKT_RCI_LENGTH;
        packet[PKT_RCI_MAGIC]   = IDENT_MAGIC;
        packet[PKT_RCI_VERSION] = IDENT_VERSION;
        stream.writeRawData((char *)&packet[0], PKT_RCI_LENGTH);
        m_telem_timer->start(67); // begin requesting telemetry
        m_mjpeg_timer->start(67); // begin requesting frames
        m_controller_timer->start(50); // begin requesting flight control
        break;
    case SERVER_ACK_IGNORED:
        Logger::info("NetworkDevice: SERVER_ACK_IGNORED\n");
        break;
    case SERVER_ACK_TAKEOFF:
        Logger::info("NetworkDevice: SERVER_ACK_TAKEOFF\n");
        break;
    case SERVER_ACK_LANDING:
        Logger::info("NetworkDevice: SERVER_ACK_LANDING\n");
        break;
    case SERVER_ACK_TELEMETRY:
        x = *(float *)&packet[PKT_VTI_ROLL];
        y = *(float *)&packet[PKT_VTI_PITCH];
        z = *(float *)&packet[PKT_VTI_YAW];
        h = *(float *)&packet[PKT_VTI_ALT];
       
        rssi    = packet[PKT_VTI_RSSI];
        battery = packet[PKT_VTI_BATT];
        aux     = packet[PKT_VTI_AUX];
        cpu     = packet[PKT_VTI_CPU];

        emit telemetryReady(-z, -y, x, h, rssi, battery, aux, cpu);
        Logger::telemetry(tr("%1 %2 %3 %4 %5 %6 %7 %8\n").arg(-z).arg(-y).arg(x)
                            .arg(h).arg(rssi).arg(battery).arg(aux).arg(cpu));
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
            Logger::info("NetworkDevice: got UPDATE_CTL_MODE to radio\n");
            m_state = STATE_RADIO_CONTROL;
            break;
        case VCM_TYPE_AUTO:
            Logger::info("NetworkDevice: got UPDATE_CTL_MODE to auto\n");
            m_state = STATE_AUTONOMOUS;
            break;
        case VCM_TYPE_MIXED:
            Logger::info("NetworkDevice: got UPDATE_CTL_MODE to mixed\n");
            m_state = STATE_MIXED_CONTROL;
            m_prev_alt = 0.0f;
            m_throttle_timer->start(50);
            break;
        case VCM_TYPE_KILL: 
            Logger::info("NetworkDevice: got UPDATE_CTL_MODE to killed\n");
            m_state = STATE_KILLED;
            break;
        case VCM_TYPE_LOCKOUT:
            Logger::info("NetworkDevice: got UPDATE_CTL_MODE to lockout\n");
            m_state = STATE_LOCKOUT;
            break;
        default:
            Logger::info("NetworkDevice: got UPDATE_CTL_MODE !! invalid !!\n");
            m_state = STATE_KILLED;
            break;
        }

        m_axes = 0;
        if (packet[PKT_VCM_AXES] & VCM_AXIS_ALT)   m_axes |= AXIS_ALT;
        if (packet[PKT_VCM_AXES] & VCM_AXIS_YAW)   m_axes |= AXIS_YAW;
        if (packet[PKT_VCM_AXES] & VCM_AXIS_PITCH) m_axes |= AXIS_PITCH;
        if (packet[PKT_VCM_AXES] & VCM_AXIS_ROLL)  m_axes |= AXIS_ROLL;

        if (m_state == STATE_MIXED_CONTROL)
        {
            if ((m_axes & AXIS_ALT) && !m_throttle_timer->isActive())
                m_throttle_timer->start(50);
            else if (!(m_axes & AXIS_ALT) && m_throttle_timer->isActive())
                m_throttle_timer->stop();
        }
        
        emit stateChanged((int)m_state);
        break;
    case SERVER_UPDATE_TRACKING:
        emit trackStatusUpdate(
                packet[PKT_CTS_STATE] == CTS_STATE_DETECTED,
                (int)packet[PKT_CTS_X1], (int)packet[PKT_CTS_Y1], 
                (int)packet[PKT_CTS_X2], (int)packet[PKT_CTS_Y2]);
        break;
    case SERVER_UPDATE_COLOR:
        //Logger::info("NETWORK: RECEIVED Update color\n");
        //R G B, ht, st, ft, fps
        emit colorValuesUpdate(TrackSettings(
            QColor((int)packet[PKT_CAM_TC_CH0], 
                    (int)packet[PKT_CAM_TC_CH1], 
                    (int)packet[PKT_CAM_TC_CH2]),
             (int)packet[PKT_CAM_TC_TH0], 
             (int)packet[PKT_CAM_TC_TH1], 
             (int)packet[PKT_CAM_TC_FILTER], 
             (int)packet[PKT_CAM_TC_FPS]));
        break;
    case SERVER_UPDATE_CAM_DCI:
        // convert the type to a string for genericness
        switch (packet[PKT_CAM_DCI_TYPE])
        {
        case CAM_DCI_TYPE_BOOL: type = "bool"; break;
        case CAM_DCI_TYPE_INT:  type = "int"; break;
        case CAM_DCI_TYPE_MENU: type = "menu"; break;
        default:
            // bad pie
            return;
        }
        emit deviceControlUpdated(QString((char *)&packet[PKT_CAM_DCI_NAME]),
                type, packet[PKT_CAM_DCI_ID],
                packet[PKT_CAM_DCI_MIN], packet[PKT_CAM_DCI_MAX],
                packet[PKT_CAM_DCI_STEP], packet[PKT_CAM_DCI_DEFAULT], 
                packet[PKT_CAM_DCI_CURRENT]);
        break;
    case SERVER_UPDATE_CAM_DCM:
        emit deviceMenuUpdated(QString((char *)&packet[PKT_CAM_DCM_NAME]),
                packet[PKT_CAM_DCM_ID], packet[PKT_CAM_DCM_INDEX]);
        break;
    case SERVER_ACK_GTS:
        emit trimSettingsUpdated(packet[PKT_GTS_YAW], packet[PKT_GTS_PITCH],
                packet[PKT_GTS_ROLL], packet[PKT_GTS_ALT]);
        break;
    case SERVER_ACK_GFS:
        emit filterSettingsUpdated(packet[PKT_GFS_IMU], packet[PKT_GFS_ALT],
                packet[PKT_GFS_AUX], packet[PKT_GFS_BATT]);
        break;
    case SERVER_ACK_GPIDS:
        union { int i; float f; } temp;
        float p, i, d;

        // get PID Kp
        temp.i = packet[PKT_GPIDS_KP];
        p = temp.f;

        // get PID Ki
        temp.i = packet[PKT_GPIDS_KI];
        i = temp.f;

        // get PID Kd
        temp.i = packet[PKT_GPIDS_KD];
        d = temp.f;
        
        Logger::info(tr("NetworkDevice: Recieved GPID axis:%1 P:%2 I:%3 D%4\n").arg(packet[PKT_GPIDS_AXIS]).arg(packet[PKT_GPIDS_KP]).arg(packet[PKT_GPIDS_KI]).arg(packet[PKT_GPIDS_KD]));
        
        emit pidSettingsUpdated(packet[PKT_GPIDS_AXIS], p, i, d);
        break;
    default:
        Logger::err(tr("NetworkDevice: bad server cmd: %1\n").arg(packet[0]));
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
        Logger::err("NetworkDevice: connection error (RemoteHostClosed)\n");
        break;
    case QAbstractSocket::HostNotFoundError:
        Logger::err("NetworkDevice: connection error (HostNotFound)\n");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        Logger::err("NetworkDevice: connection error (ConnectionRefused)\n");
        break;
    default:
        Logger::err("NetworkDevice: connection error (generic/unknown)\n");
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
                Logger::info("NetworkDevice: requesting switch to autonomous\n");
                vcm_type = VCM_TYPE_AUTO;
            }
            else
            {
                Logger::info("NetworkDevice: requesting switch to mixed mode\n");
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
                Logger::warn("NetworkDevice: unknown controller button\n");
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
void NetworkDeviceController::onUpdateTrackEnabled(bool track_en)
{
    m_track_en = track_en;

    if (m_track_en)
        Logger::info(QString("NetworkController: tracking enabled\n"));
    else
        Logger::info(QString("NetworkController: tracking disabled\n"));

    // update tracking on helicopter by sending a CLIENT_REQ_CAM_TC packet
    // with m_track_en updated
    updateTrackSettings(m_track.color.red(), m_track.color.green(), 
            m_track.color.blue(), m_track.ht, m_track.st, m_track.ft, m_track.fps);
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::updateTrackSettings(
        int r, int g, int b, int ht, int st, int ft, int fps)
{
    uint32_t cmd_buffer[16];

    m_track.color = QColor(r, g, b);
    if (ht > 0) m_track.ht = ht;
    if (st > 0) m_track.st = st;
    if (ft > 0) m_track.ft = ft;
    
    m_track.fps = fps;

    cmd_buffer[PKT_COMMAND] = CLIENT_REQ_CAM_TC;
    cmd_buffer[PKT_LENGTH]  = PKT_CAM_TC_LENGTH;

    cmd_buffer[PKT_CAM_TC_ENABLE] = (uint32_t)m_track_en;
    cmd_buffer[PKT_CAM_TC_FMT]    = CAM_TC_FMT_RGB;

    cmd_buffer[PKT_CAM_TC_CH0] = m_track.color.red();
    cmd_buffer[PKT_CAM_TC_CH1] = m_track.color.green();
    cmd_buffer[PKT_CAM_TC_CH2] = m_track.color.blue();

    cmd_buffer[PKT_CAM_TC_TH0] = m_track.ht;
    cmd_buffer[PKT_CAM_TC_TH1] = m_track.st;
    cmd_buffer[PKT_CAM_TC_TH2] = 0;
    cmd_buffer[PKT_CAM_TC_FILTER] = m_track.ft;
    
    cmd_buffer[PKT_CAM_TC_FPS] = m_track.fps;

    Logger::info(tr("request track color [%1 %2 %3] with threshold [%4 %5] with FPS %6\n")
            .arg(r).arg(g).arg(b).arg(m_track.ht).arg(m_track.st).arg(m_track.fps));

    sendPacket(cmd_buffer, PKT_CAM_TC_LENGTH);
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::updateDeviceControl(int id, int value)
{
    uint32_t cmd_buffer[8];
    cmd_buffer[PKT_COMMAND] = CLIENT_REQ_CAM_DCC;
    cmd_buffer[PKT_LENGTH]  = PKT_CAM_DCC_LENGTH;
    cmd_buffer[PKT_CAM_DCC_ID] = id;
    cmd_buffer[PKT_CAM_DCC_VALUE] = value;
    sendPacket(cmd_buffer, PKT_CAM_DCC_LENGTH);
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::updateTrimSettings(int axes, int value)
{
    uint32_t cmd_buffer[8];
    cmd_buffer[PKT_COMMAND] = CLIENT_REQ_STS;
    cmd_buffer[PKT_LENGTH]  = PKT_STS_LENGTH;

    int vcm_axes = 0;
    if (axes & AXIS_ALT)   vcm_axes |= VCM_AXIS_ALT;
    if (axes & AXIS_YAW)   vcm_axes |= VCM_AXIS_YAW;
    if (axes & AXIS_PITCH) vcm_axes |= VCM_AXIS_PITCH;
    if (axes & AXIS_ROLL)  vcm_axes |= VCM_AXIS_ROLL;

    cmd_buffer[PKT_STS_AXES] = vcm_axes;
    cmd_buffer[PKT_STS_VALUE] = value;
    sendPacket(cmd_buffer, PKT_STS_LENGTH);
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::updateFilterSettings(int signal, int samples)
{
    uint32_t cmd_buffer[8];
    cmd_buffer[PKT_COMMAND] = CLIENT_REQ_SFS;
    cmd_buffer[PKT_LENGTH]  = PKT_SFS_LENGTH;

    uint32_t sfs_sig;
    switch (signal)
    {
    case SIGNAL_ORIENTATION:
        sfs_sig = SFS_IMU;
        break;
    case SIGNAL_ALTITUDE:
        sfs_sig = SFS_ALT;
        break;
    case SIGNAL_AUXILIARY:
        sfs_sig = SFS_AUX;
        break;
    case SIGNAL_BATTERY:
        sfs_sig = SFS_BATT;
        break;
    }

    cmd_buffer[PKT_SFS_SIGNAL] = sfs_sig;
    cmd_buffer[PKT_SFS_SAMPLES] = samples;
    sendPacket(cmd_buffer, PKT_SFS_LENGTH);
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::updatePIDSettings(int axis, int signal, float value)
{
    uint32_t cmd_buffer[8];
    union { int i; float f; } temp;

    cmd_buffer[PKT_COMMAND] = CLIENT_REQ_SPIDS;
    cmd_buffer[PKT_LENGTH]  = PKT_SPIDS_LENGTH;

    uint32_t spids_sig;
    switch (signal)
    {
    case SIGNAL_KP:
        spids_sig = SPIDS_KP;
        break;
    case SIGNAL_KI:
        spids_sig = SPIDS_KI;
        break;
    case SIGNAL_KD:
        spids_sig = SPIDS_KD;
        break;
    }

    temp.f = value;

    cmd_buffer[PKT_SPIDS_PARAM] = spids_sig;
    cmd_buffer[PKT_SPIDS_VALUE] = temp.i;
    cmd_buffer[PKT_SPIDS_AXIS] = axis;
    sendPacket(cmd_buffer, PKT_SPIDS_LENGTH);
}

