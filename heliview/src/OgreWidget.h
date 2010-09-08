// -----------------------------------------------------------------------------
// File:    OgreWidget.h
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_OGREWIDGET__H_
#define _HELIVIEW_OGREWIDGET__H_

#include "OGRE/Ogre.h"
#include <QGLWidget>
#include <QX11Info>
#include "Utility.h"

class OgreWidget : public QGLWidget
{
public:
    OgreWidget(QWidget *parent = 0):
        QGLWidget(parent),
        mOgreWindow(NULL)
        {
            init("../bin/plugins.cfg", "../bin/ogre.cfg", "../bin/ogre.log");
        }
    
     virtual ~OgreWidget()
     {
        mOgreRoot->shutdown();
        SafeDelete(mOgreRoot);
        destroy();
     }

protected:
    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

    void init(std::string plugins_file, 
            std::string ogre_cfg_file, 
            std::string ogre_log_file);

    virtual Ogre::RenderSystem* chooseRenderer(Ogre::RenderSystemList*);

    Ogre::Root *mOgreRoot;
    Ogre::RenderWindow *mOgreWindow;
    Ogre::Camera *mCamera;
    Ogre::Viewport *mViewport;
    Ogre::SceneManager *mSceneMgr;
};

#endif
