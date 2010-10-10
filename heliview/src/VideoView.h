// -----------------------------------------------------------------------------
// File:    VideoView.h
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_VIDEOVIEW__H_
#define _HELIVIEW_VIDEOVIEW__H_

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

public slots:
    void onImageReady(const char *data, size_t length);
    void onTrackStatusUpdate(bool track, int x1, int y1, int x2, int y2);
    void onStatusTick();

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

    QTimer *m_timer;
    QImage *jpeg_image;
    QUdpSocket *udp_sock;
    int m_ticks, m_x1, m_x2, m_y1, m_y2;
    bool m_showBox;
};

#endif // _HELIVIEW_VIDEOVIEW__H_

