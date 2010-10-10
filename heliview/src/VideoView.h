// -----------------------------------------------------------------------------
// File:    VideoView.h
// Authors: Kevin Macksamie, Garrett Smith, Tyler Thierolf
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_VIDEOVIEW__H_
#define _HELIVIEW_VIDEOVIEW__H_

#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
#include <QUdpSocket>
#include <QWidget>

class VideoView : public QWidget
{
    Q_OBJECT

public:
    VideoView(QWidget *parent);
    virtual ~VideoView();

signals:
    void updateTrackColor(int r, int g, int b);

public slots:
    void onImageReady(const char *data, size_t length);
    void onTrackStatusUpdate(bool track, int x1, int y1, int x2, int y2);
    void onStatusTick();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    QTimer *m_timer;
    QImage *m_image;
    QUdpSocket *udp_sock;
    int m_ticks;
    bool m_showBox;
    bool m_dragging;
    QRect m_bbox;
    QRect m_dp;
};

#endif // _HELIVIEW_VIDEOVIEW__H_

