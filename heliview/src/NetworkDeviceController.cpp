// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.pp
// Authors: Garrett Smith
//
// Network device interface implementation.
// -----------------------------------------------------------------------------

#include <QDebug>
#include <QTimer>
#include <iostream>
#include <algorithm>
#include "NetworkDeviceController.h"
#include "Utility.h"

#define IDENT_MAGIC     0x09291988  // identification number
#define IDENT_VERSION   0x00000001  // software version

#define CLIENT_ACK_IDENT        0   // identify self to server
#define CLIENT_REQ_TAKEOFF      1   // command the helicopter to take off
#define CLIENT_REQ_LANDING      2   // command the helicopter to land
#define CLIENT_REQ_TELEMETRY    3   // state, orientation, altitude, battery

#define SERVER_REQ_IDENT        0   // request client to identify itself
#define SERVER_ACK_IGNORED      1   // client request ignored (invalid state)
#define SERVER_ACK_TAKEOFF      2   // acknowledge request to take off
#define SERVER_ACK_LANDING      3   // acknowledge request to land
#define SERVER_ACK_TELEMETRY    4   // acknowledge request for telemetry (+data)

using namespace std;

// -----------------------------------------------------------------------------
NetworkDeviceController::NetworkDeviceController()
{
}

// -----------------------------------------------------------------------------
NetworkDeviceController::~NetworkDeviceController()
{
}

// -----------------------------------------------------------------------------
bool NetworkDeviceController::open(const QString &device)
{
    // set some reasonable default values
    QString address("192.168.1.100");
    int portnum = 8090;

    // was an address specified?
    if (device.length())
    {
        // if so, split it into ip address and port number
        QStringList ssplit = device.split(":", QString::SkipEmptyParts);
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
    connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(onSocketError(QAbstractSocket::SocketError)));

    // attempt to connect to the specified address:port
    m_sock->connectToHost(address, portnum);
    if (!m_sock->waitForConnected()) {
        qDebug() << "connection timed out";
        return false;
    }

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
        qDebug() << "disconnected";
    }
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onTelemetryTick()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int cmd_buffer[1] = { CLIENT_REQ_TELEMETRY };
    stream.writeRawData((char *)cmd_buffer, sizeof(int) * 1);
    cout << "requested telemetry" << endl;
}

// -----------------------------------------------------------------------------
void NetworkDeviceController::onSocketReadyRead()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int32_t cmd_buffer[32], altitude, battery;
    float x, y, z;
    memset((void *)cmd_buffer, 0, sizeof(int32_t) * 32);

    size_t num_bytes = m_sock->bytesAvailable();
    if (num_bytes < 4)
        return;

    num_bytes = max(num_bytes, sizeof(int32_t) * 32);
    stream.readRawData((char *)cmd_buffer, num_bytes);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTelemetryTick()));

    switch (cmd_buffer[0])
    {
    case SERVER_REQ_IDENT:
        cerr << "SERVER_REQ_IDENT: sending response...\n";
        cmd_buffer[0] = CLIENT_ACK_IDENT;
        cmd_buffer[1] = IDENT_MAGIC;
        cmd_buffer[2] = IDENT_VERSION;
        stream.writeRawData((char *)cmd_buffer, sizeof(int32_t) * 3);
        timer->start(50); // begin requesting telemetry
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
        x = *(float *)&cmd_buffer[1];
        y = *(float *)&cmd_buffer[2];
        z = *(float *)&cmd_buffer[3];
        altitude = cmd_buffer[4];
        battery = cmd_buffer[5];
        emit telemetryReady(-z, -y, x);
        fprintf(stderr, "SERVER_ACK_TELEMETRY -> (%f, %f, %f) A (%d) B (%d)\n",
                x, y, z, altitude, battery);
        break;
    default:
        cerr << "unknown server command (" << cmd_buffer[0] << ")\n";
        break;
    }

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
}

