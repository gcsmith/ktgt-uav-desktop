// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.pp
// Authors: Garrett Smith
//
// Network device interface implementation.
// -----------------------------------------------------------------------------

#include <QDebug>
#include "NetworkDeviceController.h"
#include "Utility.h"

NetworkDeviceController::NetworkDeviceController()
{
}

NetworkDeviceController::~NetworkDeviceController()
{
}

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

