// -----------------------------------------------------------------------------
// File:    VideoView.cpp
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#include <iostream>
#include <QPainter>
#include <QTimer>
#include "VideoView.h"

using namespace std;

// -----------------------------------------------------------------------------
VideoView::VideoView(QWidget *parent)
: QWidget(parent), jpeg_image(NULL)
{
    // Set the port
    int portnum = 9010;

    // Setup UDP socket
    udp_sock = new QUdpSocket(this);
    udp_sock->connectToHost("gumstiques.rit.edu", portnum);
    cerr << "bound port\n";

    connect(udp_sock, SIGNAL(readyRead()), this, SLOT(onSocketDataPending()));
    udp_sock->waitForConnected();
    cerr << "connected to udp socket\n";

    jpeg_image = new QImage("trees.jpg");
    repaint();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTick()));
    m_timer->start(100);
}

// -----------------------------------------------------------------------------
VideoView::~VideoView()
{
    if (jpeg_image)
    {
        delete jpeg_image;
        jpeg_image = NULL;
    }

    if (udp_sock)
    {
        delete udp_sock;
        udp_sock = NULL;
    }
}

// -----------------------------------------------------------------------------
void VideoView::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.drawImage(0, 0, jpeg_image->scaled(this->width(), this->height()));
}

// -----------------------------------------------------------------------------
void VideoView::resizeEvent(QResizeEvent *e)
{
    repaint();
}

// -----------------------------------------------------------------------------
void VideoView::onSocketDataPending()
{
    // readyRead() is not emitted recursively, so we loop just incase a 
    // readyReady() signal is emitted inside its slot (this function)
    cerr << "entered onSocketDataPending\n";
    while (udp_sock->hasPendingDatagrams())
    {
        cerr << "datagram pending\n";

        // For now the UDP packet is assummed to contain only the raw jpeg data
        // there is assummed to be no packet header right now
        QByteArray buffer(udp_sock->pendingDatagramSize(), 0);
        udp_sock->readDatagram(buffer.data(), buffer.size());

        // Create jpeg
        if (!jpeg_image)
            jpeg_image = new QImage();
        else
        {
            delete jpeg_image;
            jpeg_image = new QImage();
        }

        // Load received jpeg data into newly created QImage object
        jpeg_image->loadFromData(buffer);

        // Display image in UI
        repaint();
    }
}

// -----------------------------------------------------------------------------
void VideoView::onSocketError(QAbstractSocket::SocketError error)
{
    switch (error)
    {
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

void VideoView::onTick()
{
    cerr << "requesting image\n";
    char data[4];
    udp_sock->write(data, 4);
}

