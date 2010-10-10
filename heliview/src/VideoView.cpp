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
: QWidget(parent), jpeg_image(NULL), m_ticks(0), m_showBox(false)
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
    painter.drawImage(0, 0, jpeg_image->scaled(width(), height()));

    if (m_showBox)
    {
        float xscale = width() / 320.0f;    // TODO: don't hard code this
        float yscale = height() / 240.0f;   // TODO: don't hard code this

        int x1_s = (int)(m_x1 * xscale), y1_s = (int)(m_y1 * yscale);
        int x2_s = (int)(m_x2 * xscale), y2_s = (int)(m_y2 * yscale);

        painter.setPen(QPen(QColor(255, 0, 0, 255), 3));
        painter.drawRect(x1_s, y1_s, (x2_s - x1_s), (y2_s - y1_s));
    }
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
void VideoView::onTrackStatusUpdate(bool track, int x1, int y1, int x2, int y2)
{
    m_showBox = track;
    m_x1 = x1;
    m_y1 = y1;
    m_x2 = x2;
    m_y2 = y2;;
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

