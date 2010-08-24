// -----------------------------------------------------------------------------
// File:    VirtualView.cpp
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#include "VirtualView.h"

// -----------------------------------------------------------------------------
VirtualView::VirtualView(QWidget *parent)
: QGLWidget(parent), m_angle(0.0f), m_rx(0.0f), m_ry(0.0f), m_rz(0.0f)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(20);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onPaintTick()));
}

// -----------------------------------------------------------------------------
VirtualView::~VirtualView()
{
}

// -----------------------------------------------------------------------------
void VirtualView::initializeGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glShadeModel(GL_SMOOTH);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT, GL_FILL);

}

// -----------------------------------------------------------------------------
void VirtualView::setAngles(float x, float y, float z)
{
    m_rx = x;
    m_ry = y;
    m_rz = z;
}

// -----------------------------------------------------------------------------
void VirtualView::resizeGL(int width, int height)
{
    glViewport(1.0f, 1.0f, (float)width, (float)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0f, 1.0f, 1.0f, 1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

#define PI_180 3.141592654f / 180.0f

// -----------------------------------------------------------------------------
void VirtualView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -10.0f);

    // this is all temporary. just using it as a placeholder
    gluLookAt(cos(m_angle * PI_180) * 10.0f, 3.0f, sin(m_angle * PI_180) * 10.0f,
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);

    const int steps = 40;
    const float width = 100.0f;

    float cur = -(width/2), step = (width / steps);

    glBegin(GL_LINES);
    glColor3f(0.5f, 0.5f, 0.5f);

    for (int i = 0; i < steps; i++)
    {
        glVertex3f(-(width/2), 0.0f, (float)cur);
        glVertex3f(width/2, 0.0f, (float)cur);
        cur += step;
    }

    cur = -(width/2);
    for (int i = 0; i < steps; i++)
    {
        glVertex3f((float)cur, 0.0f, -(width/2));
        glVertex3f((float)cur, 0.0f, width/2);
        cur += step;
    }
    glEnd();

    glTranslatef(0.0f, 2.0f, 0.0f);
    glRotatef(m_rx, 1.0f, 0.0f, 0.0f);
    glRotatef(m_ry, 0.0f, 1.0f, 0.0f);
    glRotatef(m_rz, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glColor3f(3.0f, 0.0f, 0.0f);
    glVertex3f(-3.0f, 0.0f, -3.0f);
    glVertex3f(-3.0f, 0.0f, 3.0f);
    glVertex3f(3.0f, 0.0f, 3.0f);
    glVertex3f(3.0f, 0.0f, -3.0f);
    glEnd();
}

// -----------------------------------------------------------------------------
void VirtualView::setRunning(bool flag)
{
    if (flag)
        m_timer->start();
    else if (m_timer->isActive())
        m_timer->stop();
}

// -----------------------------------------------------------------------------
void VirtualView::onPaintTick()
{
    m_angle += 0.1f;
    updateGL();
}

