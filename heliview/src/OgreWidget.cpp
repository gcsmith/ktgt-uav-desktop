// -----------------------------------------------------------------------------
// File:    OgreWidget.cpp
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#include "OgreWidget.h"

// -----------------------------------------------------------------------------
void OgreWidget::init(std::string plugins_file,
        std::string ogre_cfg_file,
        std::string ogre_log_file)
{
    // Create the main Ogre object
    mOgreRoot = new Ogre::Root(plugins_file, ogre_cfg_file, ogre_log_file);

    // Setup a renderer
    Ogre::RenderSystemList *renderers = mOgreRoot->getAvailableRenderers();

    // We need at least one renderer
    assert(!renderers->empty());

    Ogre::RenderSystem *renderSystem;
    renderSystem = chooseRenderer(renderers);

    // Watch if we pass back a null renderer
    assert(renderSystem);

    mOgreRoot->setRenderSystem(renderSystem);
    QString dimensions = QString("%1x%2").arg(this->width()).arg(this->height());

    renderSystem->setConfigOption("Video Mode", dimensions.toStdString());

    mOgreRoot->getRenderSystem()->setConfigOption("Full Screen", "No");
    mOgreRoot->saveConfig();

    // Don't create a window
    mOgreRoot->initialise(false);
}

// -----------------------------------------------------------------------------
void OgreWidget::initializeGL()
{
    // Get the parameters of the window QT created
    QX11Info info = x11Info();
    Ogre::String winHandle;

    winHandle = Ogre::StringConverter::toString(
            (unsigned long)(info.display()));

    winHandle += ":";

    winHandle += Ogre::StringConverter::toString(
            (unsigned long)(info.screen()));

    winHandle += ":";

    winHandle += Ogre::StringConverter::toString(
            (unsigned long)(this->parentWidget()->winId()));

    Ogre::NameValuePairList params;
    params["parentWindowHandle"] = winHandle;

    mOgreWindow = mOgreRoot->createRenderWindow("QOgreWidget_RenderWindow",
                                this->width(),
                                this->height(),
                                false,
                                &params);

    mOgreWindow->setActive(true);
    WId ogreWinId = 0x0;
    mOgreWindow->getCustomAttribute("WINDOW", &ogreWinId);

    // We need a valid window id
    assert(ogreWinId);

    this->create(ogreWinId);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoBackground);

    // Ogre Initialization
    Ogre::SceneType scene_manager_type = Ogre::ST_EXTERIOR_CLOSE;

    mSceneMgr = mOgreRoot->createSceneManager(scene_manager_type);
    mSceneMgr->setAmbientLight(Ogre::ColourValue(1, 1, 1));

    mCamera = mSceneMgr->createCamera("QOgreWidget_Cam");
    mCamera->setPosition(Ogre::Vector3(0, 1, 0));
    mCamera->lookAt(Ogre::Vector3(0, 0, 0));
    mCamera->setNearClipDistance(1.0);

    Ogre::Viewport *mViewport = mOgreWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(Ogre::ColourValue(0.8, 0.8, 1));
}

// -----------------------------------------------------------------------------
void OgreWidget::paintGL()
{
    assert(mOgreWindow);
    mOgreRoot->renderOneFrame();
}

// -----------------------------------------------------------------------------
void OgreWidget::resizeGL(int width, int height)
{
    assert(mOgreWindow);

    // Avoid errors for not using parameters
    width = height = 1;

    mOgreWindow->windowMovedOrResized();
}

// -----------------------------------------------------------------------------
Ogre::RenderSystem* OgreWidget::chooseRenderer(Ogre::RenderSystemList *renderers)
{
    // For now, return the first renderer
    return *renderers->begin();
}
