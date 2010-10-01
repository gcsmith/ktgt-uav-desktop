// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.pp
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

// macro inverts the bit (y) in the number (x)
#define FLIP_A_BIT(x,y) ((((x) & (y)) ^ (y)) | ((x) & ~(y)))

using namespace std;

// -----------------------------------------------------------------------------
NetworkDeviceController::NetworkDeviceController(const QString &device)
: m_device(device), m_telem_timer(NULL), m_mjpeg_timer(NULL), m_blocksz(0),
  m_vcm_type(VCM_TYPE_RADIO), m_vcm_axes(0)
{
    m_telem_timer = new QTimer(this);
    connect(m_telem_timer, SIGNAL(timeout()), this, SLOT(onTelemetryTick()));

    m_mjpeg_timer = new QTimer(this);
    connect(m_mjpeg_timer, SIGNAL(timeout()), this, SLOT(onVideoTick()));

    m_controller_timer = new QTimer(this);
    connect(m_controller_timer, SIGNAL(timeout()), this, 
            SLOT(onControllerTick()));

    m_manual_sigs.alt = 0.0f;
    m_manual_sigs.pitch = 0.0f;
    m_manual_sigs.roll = 0.0f;
    m_manual_sigs.yaw = 0.0f;

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
            qDebug() << "invalid address format (addr:port)";
            return false;
        }

        address = ssplit[0];
        portnum = ssplit[1].toInt();
    }
    qDebug() << "creating network device" << address << "port" << portnum;

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
        qDebug() << "connection timed out";
        return false;
    }

    emit connectionStatusChanged(QString("Connected to ") + m_device, true);
    return true;
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::close()
{
    if (m_sock)
    {
        // disconnect the socket, wait for completion
        qDebug() << "disconnecting from host ...";
        m_sock->disconnectFromHost();
        m_sock->waitForDisconnected();
        SafeDelete(m_sock);
        shutdown();
        qDebug() << "disconnected";
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
        cerr << "failed to send telemetry request\n";
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onVideoTick()
{
    if (!sendPacket(CLIENT_REQ_MJPG_FRAME))
        cerr << "failed to send mjpg frame request\n";
}

void NetworkDeviceController::onControllerTick()
{
    // Memory of previous mixed controller values
    static float prev_alt = 0.0;
    static float prev_pitch = 0.0;
    static float prev_roll = 0.0;
    static float prev_yaw = 0.0;

    if (m_vcm_type == VCM_TYPE_MIXED)
    {
        uint32_t cmd_buffer[24];
        union { int i; float f; } temp;
        char send = 0;
      
        cmd_buffer[PKT_COMMAND] = CLIENT_REQ_FLIGHT_CTL;
        cmd_buffer[PKT_LENGTH]  = PKT_MCM_LENGTH;
      
        // altitude
        temp.f = m_manual_sigs.alt;
        if (m_manual_sigs.alt != prev_alt)
        {
            prev_alt = m_manual_sigs.alt;
            send = 1;
        }
        cmd_buffer[PKT_MCM_AXIS_ALT] = temp.i;

        // pitch
        temp.f = m_manual_sigs.pitch;
        if (m_manual_sigs.pitch != prev_pitch)
        {
            prev_pitch = m_manual_sigs.pitch;
            send = 1;
        }
        cmd_buffer[PKT_MCM_AXIS_PITCH] = temp.i;

        // roll
        temp.f = m_manual_sigs.roll;
        if (m_manual_sigs.roll != prev_roll)
        {
            prev_roll = m_manual_sigs.roll;
            send = 1;
        }
        cmd_buffer[PKT_MCM_AXIS_ROLL] = temp.i;

        // yaw
        temp.f = m_manual_sigs.yaw;
        if (m_manual_sigs.yaw!= prev_yaw)
        {
            prev_yaw = m_manual_sigs.yaw;
            send = 1;
        }
        cmd_buffer[PKT_MCM_AXIS_YAW] = temp.i;

        if (send && !sendPacket(cmd_buffer, PKT_MCM_LENGTH))
            cerr << "failed to send flight control request\n";
        else if (send)
        {
            fprintf(stderr, "Sent ALT:   %f\n", m_manual_sigs.alt);
            fprintf(stderr, "Sent PITCH: %f\n", m_manual_sigs.pitch);
            fprintf(stderr, "Sent ROLL:  %f\n", m_manual_sigs.roll);
            fprintf(stderr, "Sent YAW:   %f\n\n", m_manual_sigs.yaw);
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
        cerr << "SERVER_REQ_IDENT: sending response...\n";
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
        cerr << "SERVER_ACK_IGNORED" << endl;
        break;
    case SERVER_ACK_TAKEOFF:
        cerr << "SERVER_ACK_TAKEOFF" << endl;
        break;
    case SERVER_ACK_LANDING:
        cerr << "SERVER_ACK_LANDING" << endl;
        break;
    case SERVER_ACK_TELEMETRY:
        x = *(float *)&packet[PKT_VTI_YAW];
        y = *(float *)&packet[PKT_VTI_PITCH];
        z = *(float *)&packet[PKT_VTI_ROLL];
        rssi     = packet[PKT_VTI_RSSI];
        altitude = packet[PKT_VTI_ALT];
        battery  = packet[PKT_VTI_BATT];
        aux      = packet[PKT_VTI_AUX];
        emit telemetryReady(-z, -y, x, altitude, rssi, battery, aux);
#if 0
        fprintf(stderr,
                "SERVER_ACK_TELEMETRY -> (%f, %f, %f) S (%d) A (%d) B (%d)\n",
                x, y, z, rssi, altitude, battery);
#endif
        break;
    case SERVER_ACK_MJPG_FRAME:
        framesz = packet[PKT_LENGTH] - PKT_MJPG_LENGTH;
        emit videoFrameReady((const char *)&packet[PKT_MJPG_IMG], (size_t)framesz);
        break;
    case SERVER_ACK_SET_CTL_MODE:
        cerr << "got ACK for SET_CTL_MODE: ";
        switch (packet[PKT_VCM_TYPE])
        {
        case VCM_TYPE_RADIO: cerr << "radio\n"; break;
        case VCM_TYPE_AUTO:  cerr << "auto\n"; break;
        case VCM_TYPE_MIXED: cerr << "mixed\n"; break;
        case VCM_TYPE_KILL:  cerr << "killed\n"; break;
        default:             cerr << "!!! invalid !!!\n"; break;
        }
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
        cerr << "QAbstractSocket::RemoteHostClosedError" << endl;
        break;
    case QAbstractSocket::HostNotFoundError:
        cerr << "QAbstractSocket::HostNotFoundError" << endl;
        break;
    case QAbstractSocket::ConnectionRefusedError:
        cerr << "QAbstractSocket::ConnectionRefusedError" << endl;
        break;
    default:
        cerr << "unknown socket error" << endl;
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
            QDataStream stream(m_sock);
            stream.setVersion(QDataStream::Qt_4_0);

            if (VCM_TYPE_MIXED == m_vcm_type)
            {
                cerr << "requesting switch to radio mode...\n";
                m_vcm_type = VCM_TYPE_RADIO;
            }
            else
            {
                cerr << "requesting switch to mixed mode...\n";
                m_vcm_type = VCM_TYPE_MIXED;
            }

            m_vcm_axes = 0;

            cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
            cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
            cmd_buffer[PKT_VCM_TYPE] = m_vcm_type;
            cmd_buffer[PKT_VCM_AXES] = m_vcm_axes;

            stream.writeRawData((char *)cmd_buffer, PKT_VCM_LENGTH);
            m_sock->waitForBytesWritten();
        }
        else if ((m_vcm_type == VCM_TYPE_MIXED) && (value > 0.0) && 
                (index >= 4) && (index <= 7))
        {
            QDataStream stream(m_sock);
            stream.setVersion(QDataStream::Qt_4_0);

            // update the mixed mode controlled axes
            switch (index)
            {
                // A
                case 4:
                    m_vcm_axes = FLIP_A_BIT(m_vcm_axes, VCM_AXIS_ALT);
                    break;

                // B
                case 5:
                    m_vcm_axes = FLIP_A_BIT(m_vcm_axes, VCM_AXIS_ROLL);
                    break;

                // X
                case 6:
                    m_vcm_axes = FLIP_A_BIT(m_vcm_axes, VCM_AXIS_YAW);
                    break;

                // Y
                case 7:
                    m_vcm_axes = FLIP_A_BIT(m_vcm_axes, VCM_AXIS_PITCH);
                    break;

                default:
                    fprintf(stderr, "NetworkDeviceController: unknown controller button\n");
                    break;
            }

            // tell server about which axes are controlled by the mixed mode controller
            cmd_buffer[PKT_COMMAND]  = CLIENT_REQ_SET_CTL_MODE;
            cmd_buffer[PKT_LENGTH]   = PKT_VCM_LENGTH;
            cmd_buffer[PKT_VCM_TYPE] = VCM_TYPE_MIXED;
            cmd_buffer[PKT_VCM_AXES] = m_vcm_axes;

            stream.writeRawData((char *)cmd_buffer, PKT_VCM_LENGTH);
            m_sock->waitForBytesWritten();
        }
    }
    else if (GP_EVENT_AXIS == event)
    {
        if (index >= 0 && index <= 3)
        {
            switch (index)
            {
                case 0:
                    m_manual_sigs.yaw = value;
                    break;
                    
                case 1:
                    m_manual_sigs.alt = -value;
                    break;

                case 2:
                    m_manual_sigs.roll = value;
                    break;

                case 3:
                    m_manual_sigs.pitch = value;
                    break;

                default:
                    fprintf(stderr, "NetworkDeviceController: unknown controller axis");
                    break;
            }
        }
    }
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

