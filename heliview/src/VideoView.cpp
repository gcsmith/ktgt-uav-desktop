// -----------------------------------------------------------------------------
// File:    VideoView.cpp
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#include <iostream>
#include <QPainter>
#include <QTimer>
#include "Utility.h"
#include "VideoView.h"

using namespace std;

// -----------------------------------------------------------------------------
VideoView::VideoView(QWidget *parent)
: QWidget(parent), jpeg_image(NULL), m_ticks(0)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onStatusTick()));
    m_timer->start(100);

    jpeg_image = new QImage(":/data/test_pattern.jpg");
    repaint();
}

// -----------------------------------------------------------------------------
VideoView::~VideoView()
{
    SafeDelete(jpeg_image);
}

// -----------------------------------------------------------------------------
void VideoView::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawImage(0, 0, jpeg_image->scaled(this->width(), this->height()));
    painter.drawRect(x1,y1,(x2-x1),(y2-y1));
}

// -----------------------------------------------------------------------------
void VideoView::resizeEvent(QResizeEvent *e)
{
    repaint();
}

// -----------------------------------------------------------------------------
void VideoView::onImageReady(const char *data, size_t length)
{
    cerr << "loading image size " << length << endl;
    if (jpeg_image->loadFromData((const uchar *)data, (int)length))
    {
        m_ticks = 0;
        repaint();
    }
    else
    {
        // uh oh
        cerr << "failed to load image data\n";
    }
}

// -----------------------------------------------------------------------------
void VideoView::onCoordinatesReady(int _x1, int _y1, int _x2, int _y2)
{
    if (m_ticks < 20)
    {
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;;
        repaint();
    }
}

// -----------------------------------------------------------------------------
void VideoView::onStatusTick()
{
    ++m_ticks;

    if (m_ticks > 20)
    {
        jpeg_image->load(":/data/test_pattern.jpg");
        repaint();
        m_ticks = 0;
    }
}

