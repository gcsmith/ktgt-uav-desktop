// -----------------------------------------------------------------------------
// File:    CVWebcamView.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_CVWEBCAMVIEW__H_
#define _HELIVIEW_CVWEBCAMVIEW__H_

#include <QtOpenGL>
#include <QTimer>
#include <opencv/cv.h>
#include <opencv/highgui.h>

class CVWebcamView : public QGLWidget
{
    Q_OBJECT

public:
    CVWebcamView(QWidget *parent);
    virtual ~CVWebcamView();
    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

    void setRunning(bool flag);

public slots:
    void onPaintTick();

protected:
    void captureOneFrame();

    CvCapture *m_capture;
    QTimer *m_timer;
    GLuint m_texid;
};

#endif // _HELIVIEW_CVWEBCAMVIEW__H_

