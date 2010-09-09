// -----------------------------------------------------------------------------
// File:    VirtualView.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_VIRTUALVIEW__H_
#define _HELIVIEW_VIRTUALVIEW__H_

#include "Ogre.h"
#include <QWidget>
#include <QTimer>
#include <QX11Info>

class VirtualView : public QWidget
{
    Q_OBJECT

public:
    VirtualView(QWidget *parent);
    virtual ~VirtualView();

    void initialize();
    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *);

    void setRunning(bool flag);
    void setAngles(float yaw, float pitch, float roll);

protected slots:
    void createRenderWindows();
    Ogre::RenderSystem* chooseRenderer(const Ogre::RenderSystemList &);
    void onPaintTick();

protected:
    QTimer *m_timer;
    float m_angle;
    float m_yaw, m_pitch, m_roll;

    Ogre::Root         *m_root;
    Ogre::RenderWindow *m_window;
    Ogre::Camera       *m_camera;
    Ogre::Viewport     *m_view;
    Ogre::SceneManager *m_manager;
    Ogre::Entity       *e_heli;
    Ogre::SceneNode    *n_heli;
};

#endif // _HELIVIEW_VIRTUALVIEW__H_

