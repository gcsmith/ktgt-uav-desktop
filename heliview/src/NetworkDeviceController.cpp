// -----------------------------------------------------------------------------
// File:    NetworkDeviceController.pp
// Authors: Garrett Smith
//
// Network device interface implementation.
// -----------------------------------------------------------------------------

#include <QDebug>
#include "NetworkDeviceController.h"

NetworkDeviceController::NetworkDeviceController()
{
}

NetworkDeviceController::~NetworkDeviceController()
{
}

bool NetworkDeviceController::open(const QString &device)
{
    qDebug() << "creating network device" << device;

    // attempt to create and connect to the network socket
    m_sock = new QTcpSocket();
    connect(m_sock, SIGNAL(readyRead()), this,
            SLOT(onSocketReadyRead()));
    connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(onSocketError(QAbstractSocket::SocketError)));

    m_sock->connectToHost("192.168.1.103", 8090);
    if (!m_sock->waitForConnected()) {
        qDebug() << "connection timed out";
        return false;
    }

    return true;
}

void NetworkDeviceController::close()
{
}

