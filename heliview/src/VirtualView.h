// -----------------------------------------------------------------------------
// File:    VirtualView.h
// Authors: Garrett Smith
// Created: 08-23-2010
//
// Displays a 3D representation of the helicopter's current orientation.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_VIRTUALVIEW__H_
#define _HELIVIEW_VIRTUALVIEW__H_

#include <Ogre.h>
#include <OgreLogManager.h>
#include <QWidget>
#include <QTimer>

class VirtualView : public QWidget
{
    Q_OBJECT

public:
    VirtualView(QWidget *parent);
    virtual ~VirtualView();

    void initialize();
    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual QPaintEngine *paintEngine() const;

    void setRunning(bool flag);
    void setOrientation(float yaw, float pitch, float roll);
    void setAltitude(float altitude);

protected slots:
    void onPaintTick();

protected:
    Ogre::RenderSystem* chooseRenderer(const Ogre::RenderSystemList &);
    void createRenderWindows();
    void loadConfiguration();
    void setupRenderSystem();

    QTimer *m_timer;
    float m_time;
    float m_yaw, m_pitch, m_roll;
    float m_alt;

    Ogre::Root         *m_root;
    Ogre::RenderWindow *m_window;
    Ogre::Camera       *m_camera;
    Ogre::Viewport     *m_view;
    Ogre::SceneManager *m_scene;
    Ogre::Entity       *e_heli, *e_main_rotor, *e_tail_rotor;
    Ogre::SceneNode    *n_heli, *n_main_rotor, *n_tail_rotor;
    Ogre::LogManager   *m_logmgr;
    Ogre::Log          *m_log;
};

#endif // _HELIVIEW_VIRTUALVIEW__H_

