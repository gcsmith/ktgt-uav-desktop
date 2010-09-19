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

using namespace std;

// -----------------------------------------------------------------------------
NetworkDeviceController::NetworkDeviceController(const QString &device)
: m_device(device), m_telem_timer(NULL), m_mjpeg_timer(NULL), m_blocksz(0)
{
    m_telem_timer = new QTimer(this);
    connect(m_telem_timer, SIGNAL(timeout()), this, SLOT(onTelemetryTick()));

    m_mjpeg_timer = new QTimer(this);
    connect(m_mjpeg_timer, SIGNAL(timeout()), this, SLOT(onVideoTick()));
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
void NetworkDeviceController::onTelemetryTick()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int cmd_buffer[] = { CLIENT_REQ_TELEMETRY, PKT_BASE_LENGTH };
    stream.writeRawData((char *)cmd_buffer, PKT_BASE_LENGTH);
    m_sock->waitForBytesWritten();
    cerr << "requested telemetry" << endl;
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onVideoTick()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int cmd_buffer[] = { CLIENT_REQ_MJPG_FRAME, PKT_BASE_LENGTH };
    stream.writeRawData((char *)cmd_buffer, PKT_BASE_LENGTH);
    m_sock->waitForBytesWritten();
    cerr << "requested mjpeg frame" << endl;
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onSocketReadyRead()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int32_t rssi, altitude, battery, framesz;
    float x, y, z;

    if (0 == m_blocksz)
    {
        if ((size_t)m_sock->bytesAvailable() < PKT_BASE_LENGTH)
            return;

        uint32_t header[PKT_BASE];
        stream.readRawData((char *)&header[0], PKT_BASE_LENGTH);
        m_blocksz = header[PKT_LENGTH] - PKT_BASE_LENGTH;
        fprintf(stderr, "packet cmd %d size %d\n", header[0], header[1]);

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
        m_mjpeg_timer->start(50); // begin requesting frames
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
        emit telemetryReady(-z, -y, x, altitude, rssi, battery);
#if 0
        fprintf(stderr,
                "SERVER_ACK_TELEMETRY -> (%f, %f, %f) S (%d) A (%d) B (%d)\n",
                x, y, z, rssi, altitude, battery);
#endif
        break;
    case SERVER_ACK_MJPG_FRAME:
        framesz = packet[PKT_LENGTH] - PKT_BASE_LENGTH;
        emit videoFrameReady((const char *)&packet[PKT_BASE], (size_t)framesz);
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
    cerr << "networkdevicecontroller receive input\n";
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::shutdown()
{
    if (m_telem_timer)
    {
        m_telem_timer->stop();
        m_mjpeg_timer->stop();
        emit connectionStatusChanged(m_device + QString(" disconnected"), false);
    }
}

