// -----------------------------------------------------------------------------
// File:    CVWebcamView.cpp
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#include <iostream>
#include "CVWebcamView.h"

using namespace std;

// -----------------------------------------------------------------------------
CVWebcamView::CVWebcamView(QWidget *parent)
: QGLWidget(parent), m_capture(NULL), m_texid(-1)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(33);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onPaintTick()));
    
    m_capture = cvCaptureFromCAM(0);
    if (!m_capture)
    {
        cerr << "failed to capture" << endl;
        return;
    }

    m_timer->start();
}

// -----------------------------------------------------------------------------
CVWebcamView::~CVWebcamView()
{
    if (m_capture)
    {
        cvReleaseCapture(&m_capture);
        m_capture = NULL;
    }
}

// -----------------------------------------------------------------------------
void CVWebcamView::initializeGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glDisable(GL_LIGHTING);

    glShadeModel(GL_SMOOTH);
    glFrontFace(GL_CCW);

    glGenTextures(1, &m_texid);
    cout << "texture " << m_texid << endl;

    captureOneFrame();
}

// -----------------------------------------------------------------------------
void CVWebcamView::resizeGL(int width, int height)
{
    glViewport(0.0f, 0.0f, (float)width, (float)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, 640.0f, 0.0f, 480.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// -----------------------------------------------------------------------------
void CVWebcamView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_texid);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0f, 480.0f); glVertex3f(0.0f, 0.0f, 0.0f);
    glTexCoord2d(640.0f, 480.0f); glVertex3f(640.0f, 0.0f, 0.0f);
    glTexCoord2d(640.0f, 0.0f); glVertex3f(640.0f, 480.0f, 0.0f);
    glTexCoord2d(0.0f, 0.0f); glVertex3f(0.0f, 480.0f, 0.0f);
    glEnd();
}

// -----------------------------------------------------------------------------
void CVWebcamView::setRunning(bool flag)
{
    if (flag)
        m_timer->start();
    else if (m_timer->isActive())
        m_timer->stop();
}

// -----------------------------------------------------------------------------
void CVWebcamView::onPaintTick()
{
    captureOneFrame();
    updateGL();
}

// -----------------------------------------------------------------------------
void CVWebcamView::captureOneFrame()
{
    IplImage* frame = cvQueryFrame(m_capture);
    if (!frame)
    {
        cerr << "failed to capture frame..." << endl;
        return;
    }

    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_texid );
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB,
                 frame->width, frame->height, 0,
                 GL_BGR, GL_UNSIGNED_BYTE, frame->imageData);
}

