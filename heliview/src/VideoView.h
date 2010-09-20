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
    void onStatusTick();

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

    QTimer *m_timer;
    QImage *jpeg_image;
    QUdpSocket *udp_sock;
    int m_ticks;
};

#endif // _HELIVIEW_VIDEOVIEW__H_

