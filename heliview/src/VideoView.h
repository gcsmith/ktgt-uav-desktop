// -----------------------------------------------------------------------------
// File:    VideoView.h
// Authors: Kevin Macksamie, Garrett Smith, Tyler Thierolf
// Created: 09-15-2010
//
// Widget that displays mjpg video frames and allows the user to click and drag
// a bounding rectangle over the image to select a target tracking color.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_VIDEOVIEW__H_
#define _HELIVIEW_VIDEOVIEW__H_

#include <QMouseEvent>
#include <QImage>
#include <QPainter>
#include <QTextEdit>
#include <QUdpSocket>
#include <QWidget>

class VideoView: public QWidget
{
    Q_OBJECT

public:
    VideoView(QWidget *parent);
    virtual ~VideoView();

    bool saveFrame();

signals:
    void updateTracking(int r, int g, int b, int ht, int st, int ft);

public slots:
    void setDragBoxColor(int r, int g, int b, int a);
    void setBoundingBoxColor(int r, int g, int b, int a);
    void setTimeoutTicks(int ticks);
    void onImageReady(const char *data, size_t length);
    void onTrackStatusUpdate(bool track, int x1, int y1, int x2, int y2);
    void onStatusTick();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    QImage *m_image;
    QTimer *m_timer;
    int m_ticks, m_maxTicks;
    bool m_showBox, m_dragging;
    QRect m_bbox, m_dp;
    QBrush m_dragBrush, m_bboxBrush;
    QPen m_dragPen, m_bboxPen;
};

#endif // _HELIVIEW_VIDEOVIEW__H_

