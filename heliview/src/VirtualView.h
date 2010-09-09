// -----------------------------------------------------------------------------
// File:    VirtualView.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_VIRTUALVIEW__H_
#define _HELIVIEW_VIRTUALVIEW__H_

#include "Ogre.h"
#include <QtOpenGL>
#include <QTimer>
#include <QX11Info>

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
    virtual Ogre::RenderSystem* chooseRenderer(Ogre::RenderSystemList*);

protected:
    QTimer *m_timer;
    float m_angle;
    float m_yaw, m_pitch, m_roll;

    Ogre::Root         *mOgreRoot;
    Ogre::RenderWindow *mOgreWindow;
    Ogre::Camera       *mCamera;
    Ogre::Viewport     *mViewport;
    Ogre::SceneManager *mSceneMgr;
    Ogre::Entity       *e_heli;
    Ogre::SceneNode    *n_heli;
};

#endif // _HELIVIEW_VIRTUALVIEW__H_

