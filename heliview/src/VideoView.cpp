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
: QWidget(parent), jpeg_image(NULL)
{
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
    if (!jpeg_image->loadFromData((const uchar *)data, (int)length))
    {
        cerr << "failed to load image data\n";
    }
    repaint();
}

