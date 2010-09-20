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

