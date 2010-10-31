// -----------------------------------------------------------------------------
// File:    VideoView.cpp
// Authors: Kevin Macksamie, Garrett Smith, Tyler Thierolf
// Created: 09-15-2010
//
// Widget that displays mjpg video frames and allows the user to click and drag
// a bounding rectangle over the image to select a target tracking color.
// -----------------------------------------------------------------------------

#include <QDateTime>
#include <QPainter>
#include <QTimer>
#include "Logger.h"
#include "Utility.h"
#include "VideoView.h"

// -----------------------------------------------------------------------------
VideoView::VideoView(QWidget *parent)
: QWidget(parent), m_ticks(0), m_maxTicks(25), m_showBox(false),
  m_dragging(false), m_bbox(0, 0, 0, 0), m_dp(0, 0, 0, 0)
{
    // set default image at startup to test pattern
    m_image = new QImage(":/data/test_pattern.jpg");

    // create a timer to serve as a simple video feed heartbeat check
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onStatusTick()));
    m_timer->start(100);

    m_dragBrush.setStyle(Qt::SolidPattern);
    m_bboxBrush.setStyle(Qt::SolidPattern);

    setDragBoxColor(255, 0, 0, 25);
    setBoundingBoxColor(255, 0, 0, 0);

    repaint();
}
// -----------------------------------------------------------------------------

void VideoView::onUpdateTrackControlEnable(int enable)
{
    if(enable)
    {
        setBoundingBoxColor(0, 255, 0 , 0);
        Logger::info(tr("Video: Box Set to GREEN\n"));
    } else {
        setBoundingBoxColor(255, 0, 0, 0);
        Logger::info(tr("Video: Box Set to RED\n"));
    }

}
// -----------------------------------------------------------------------------
VideoView::~VideoView()
{
    SafeDelete(m_image);
    SafeDelete(m_timer);
}

// -----------------------------------------------------------------------------
void VideoView::setDragBoxColor(int r, int g, int b, int a)
{
    m_dragPen.setColor(QColor(r, g, b, 255));
    m_dragBrush.setColor(QColor(r, g, b, a));
}

// -----------------------------------------------------------------------------
void VideoView::setBoundingBoxColor(int r, int g, int b, int a)
{
    m_bboxPen.setColor(QColor(r, g, b, 255));
    m_bboxBrush.setColor(QColor(r, g, b, a));
}

// -----------------------------------------------------------------------------
void VideoView::setTimeoutTicks(int ticks)
{
    m_ticks = 0;
    m_maxTicks = ticks;
}

// -----------------------------------------------------------------------------
void VideoView::paintEvent(QPaintEvent *e)
{
    // first render the video feed (or test image) into the client area
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawImage(0, 0, m_image->scaled(width(), height()));

    if (m_showBox)
    {
        float xscale = width() / (float)m_image->width();
        float yscale = height() / (float)m_image->height();

        // scale coordinates from client to jpeg image dimensions
        int x_s = (int)(m_bbox.x() * xscale);
        int y_s = (int)(m_bbox.y() * yscale);
        int w_s = (int)(m_bbox.width() * xscale);
        int h_s = (int)(m_bbox.height() * yscale);
        int xc_s = (int)(m_center.x() * xscale);
        int yc_s = (int)(m_center.y() * yscale);
        int ln_m = std::min(w_s, h_s) / 20;

        // render a wireframe rectangle around the region of interest
        painter.setPen(m_bboxPen);
        painter.setBrush(m_bboxBrush);
        painter.drawRect(x_s, y_s, w_s, h_s);
        painter.drawLine(xc_s - ln_m, yc_s - ln_m, xc_s + ln_m, yc_s + ln_m);
        painter.drawLine(xc_s - ln_m, yc_s + ln_m, xc_s + ln_m, yc_s - ln_m);
    }

    if (m_dragging)
    {
        // render a filled rectangle around the drag zone
        painter.setPen(m_dragPen);
        painter.setBrush(m_dragBrush);
        painter.drawRect(m_dp.x(), m_dp.y(), m_dp.width(), m_dp.height());
    }
}

// -----------------------------------------------------------------------------
void VideoView::resizeEvent(QResizeEvent *e)
{
    repaint();
}

// -----------------------------------------------------------------------------
bool VideoView::saveFrame()
{
    QDateTime datetime;
    datetime = QDateTime::currentDateTime();

    const char *format = "MMM-dd-yyyy_hh-mm-ss-zzz";
    const char *tstamp = datetime.toString(format).toAscii().constData();

    QString filename = QString("heliview_%1.jpg").arg(tstamp);
    Logger::info(tr("Video: Attempting to save frame as %1\n").arg(filename));

    return (m_image->save(QString(filename)));
}

// -----------------------------------------------------------------------------
void VideoView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging)
    {
        int right = std::max(0, std::min(e->x(), width() - 1));
        int bottom = std::max(0, std::min(e->y(), height() - 1));

        // repaint only the union of the old and new drag rectangles
        QRect old_rect = m_dp;
        m_dp.setRight(right);
        m_dp.setBottom(bottom);
        repaint(old_rect.united(m_dp).adjusted(-1, -1, 1, 1));
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
        float xscale = (float)m_image->width() / width();
        float yscale = (float)m_image->height() / height();

        QRect coord = m_dp.normalized();
        int x1 = (int)(xscale * coord.left());
        int y1 = (int)(yscale * coord.top());
        int x2 = (int)(xscale * coord.right());
        int y2 = (int)(yscale * coord.bottom());
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
        emit trackSettingsChanged(avg_r, avg_g, avg_b, -1, -1, -1, -1);
        
        // disable the dragging rectangle and force an update of the widget
        m_dragging = false;
        repaint(m_dp.normalized().adjusted(-1, -1, 1, 1));
    }
}

// -----------------------------------------------------------------------------
void VideoView::onImageReady(const char *data, size_t length)
{
    Logger::extraDebug(tr("loading image size %1\n").arg(length));
    if (m_image->loadFromData((const uchar *)data, (int)length))
    {
        // reset the heartbeat timeout and force a redraw of the client area
        m_ticks = 0;
        repaint();
    }
    else
    {
        // uh oh
        Logger::err("Video: failed to load image data\n");
    }
}

// -----------------------------------------------------------------------------
void VideoView::onTrackStatusUpdate(bool en, const QRect &bb, const QPoint &cp)
{
    m_showBox = en;
    m_bbox = bb;
    m_center = cp;
}

// -----------------------------------------------------------------------------
void VideoView::onStatusTick()
{
    ++m_ticks;

    // if we exceed the max tick count without receiving a new image from the
    // device, display a test image to bring attention to the operator
    if (m_ticks > m_maxTicks)
    {
        m_image->load(":/data/test_pattern.jpg");
        repaint();
        m_ticks = 0;
    }
}

