// -----------------------------------------------------------------------------
// File:    VirtualView.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_VIRTUALVIEW__H_
#define _HELIVIEW_VIRTUALVIEW__H_

#include <QtOpenGL>
#include <QTimer>

class VirtualView : public QGLWidget
{
    Q_OBJECT

public:
    VirtualView(QWidget *parent);
    virtual ~VirtualView();

    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

    void setRunning(bool flag);
    void setAngles(float yaw, float pitch, float roll);

protected slots:
    void onPaintTick();

protected:
    QTimer *m_timer;
    float m_angle;
    float m_yaw, m_pitch, m_roll;
};

#endif // _HELIVIEW_VIRTUALVIEW__H_

