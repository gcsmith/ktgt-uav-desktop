// -----------------------------------------------------------------------------
// File:    VideoView.cpp
// Authors: Kevin Macksamie, Garrett Smith, Tyler Thierolf
// -----------------------------------------------------------------------------

#include <iostream>
#include <QPainter>
#include <QTimer>
#include "Utility.h"
#include "VideoView.h"

using namespace std;

// -----------------------------------------------------------------------------
VideoView::VideoView(QWidget *parent)
: QWidget(parent), m_image(NULL), m_ticks(0),
  m_showBox(false), m_dragging(false), m_bbox(0, 0, 0, 0), m_dp(0, 0, 0, 0)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onStatusTick()));
    m_timer->start(100);

    m_image = new QImage(":/data/test_pattern.jpg");
    repaint();
}

// -----------------------------------------------------------------------------
VideoView::~VideoView()
{
    SafeDelete(m_image);
}

// -----------------------------------------------------------------------------
void VideoView::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawImage(0, 0, m_image->scaled(width(), height()));

    if (m_showBox)
    {
        float xscale = width() / 320.0f;    // TODO: don't hard code this
        float yscale = height() / 240.0f;   // TODO: don't hard code this

        int x_s = (int)(m_bbox.x() * xscale);
        int y_s = (int)(m_bbox.y() * yscale);
        int w_s = (int)(m_bbox.width() * xscale);
        int h_s = (int)(m_bbox.height() * yscale);

        painter.setPen(QPen(QColor(255, 0, 0, 255), 3));
        painter.drawRect(x_s, y_s, w_s, h_s);
    }

    if (m_dragging)
    {
        painter.setPen(QPen(QColor(255, 0, 0, 255), 3));
        painter.drawRect(m_dp.x(), m_dp.y(), m_dp.width(), m_dp.height());
    }
}

// -----------------------------------------------------------------------------
void VideoView::resizeEvent(QResizeEvent *e)
{
    repaint();
}

// -----------------------------------------------------------------------------
void VideoView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging)
    {
        QRect old_rect = m_dp;
        m_dp.setRight(e->x());
        m_dp.setBottom(e->y());
        repaint();
    }
}

// -----------------------------------------------------------------------------
void VideoView::mousePressEvent(QMouseEvent *e)
{
    if (!m_dragging && Qt::LeftButton == e->button())
    {
        m_dragging = true;
        m_dp.setCoords(e->x(), e->y(), e->x(), e->y());
    }
}

// -----------------------------------------------------------------------------
void VideoView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_dragging && Qt::LeftButton == e->button())
    {
        // determine the average color of the selected region
        long avg_r = 0, avg_b = 0, avg_g = 0;
        float xscale = 320.0f / width();    // TODO: don't hard code this
        float yscale = 240.0f / height();   // TODO: don't hard code this

        QRect coord = m_dp.normalized();
        int x1 = (int)(coord.left() * xscale);
        int y1 = (int)(coord.top() * yscale);
        int x2 = (int)(coord.right() * xscale);
        int y2 = (int)(coord.bottom() * yscale);
        float inv_pixels = 1.0f / ((y2 - y1 + 1) * (x2 - x1 + 1));

        for (int y = y1; y <= y2; ++y)
        {
            for (int x = x1; x <= x2; ++x)
            {
                QRgb rgb = m_image->pixel(x, y);
                avg_r += qRed(rgb);
                avg_g += qGreen(rgb);
                avg_b += qBlue(rgb);
            }
        }

        // ping listeners of the updated tracking color
        avg_r = (int)(avg_r * inv_pixels);
        avg_g = (int)(avg_g * inv_pixels);
        avg_b = (int)(avg_b * inv_pixels);
        emit updateTrackColor(avg_r, avg_g, avg_b);

        m_dragging = false;
        m_dp.setCoords(0, 0, 0, 0);
        repaint();
    }
}

// -----------------------------------------------------------------------------
void VideoView::onImageReady(const char *data, size_t length)
{
    cerr << "loading image size " << length << endl;
    if (m_image->loadFromData((const uchar *)data, (int)length))
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
    m_bbox.setCoords(x1, y1, x2, y2);
}

// -----------------------------------------------------------------------------
void VideoView::onStatusTick()
{
    ++m_ticks;

    if (m_ticks > 20)
    {
        m_image->load(":/data/test_pattern.jpg");
        repaint();
        m_ticks = 0;
    }
}

