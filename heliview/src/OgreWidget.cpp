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

    // Set light & fog
    mSceneMgr->setAmbientLight(Ogre::ColourValue(1, 1, 1));
    mSceneMgr->setFog(
            Ogre::FOG_LINEAR, 
            Ogre::ColourValue(1, 1, 0.8f), 
            0, 
            80, 
            200);

    mCamera = mSceneMgr->createCamera("QOgreWidget_Cam");
    mCamera->setPosition(Ogre::Vector3(0, 1, 0));
    mCamera->lookAt(Ogre::Vector3(0, 0, 0));
    mCamera->setNearClipDistance(1.0);

    Ogre::Viewport *mViewport = mOgreWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(Ogre::ColourValue(0.8, 0.8, 1));

    // Create scene
    Ogre::SceneNode *n_root = mSceneMgr->getRootSceneNode();

    //m_physics = new HSPhysicsWorld();
    //m_physics->getDynamics()->setGravity(btVector3(0, -9.8, 0));
    //m_physics->setDebugMode(false);

    // Create the floor plane's mesh and entity
    Ogre::MeshManager::getSingleton().createPlane(
            "floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 500, 500, 10, 10,
            true, 1, 10, 10, Ogre::Vector3::UNIT_Z);

    // Create floor
#if 0
    Ogre::Entity *e_floor = mSceneMgr->createEntity("Floor", "floor");
    e_floor->setMaterialName("world/dirt");
    e_floor->setCastShadows(false);
    n_root->attachObject(e_floor);
#endif

    // Create the ground collision plane
#if 0
    btCollisionShape *gnd = OGRE_NEW btStaticPlaneShape(btVector3(0, 1, 0), 0);
    {
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(0, 0, 0));

        btScalar mass(0.0f);
        btVector3 inertia(0, 0, 0);

        HSMotionState *motion = OGRE_NEW HSMotionState(n_root, transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion, gnd, inertia);
        btRigidBody *body = OGRE_NEW btRigidBody(rbInfo);

        m_physics->getDynamics()->addRigidBody(body);
    }
#endif

    // Create and attach a helicopter entity and scene node
    Ogre::Entity *e_heli = mSceneMgr->createEntity("Helicopter", "apache_body.mesh");
    Ogre::SceneNode *n_heli = n_root->createChildSceneNode("HeliNode");
    n_heli->setPosition(Ogre::Vector3(0, 40, 0));
    n_heli->setScale(Ogre::Vector3(3, 3, 3));
    n_heli->attachObject(e_heli);

    // Create the helicopter collision box
#if 0
    btCollisionShape *box = OGRE_NEW btBoxShape(btVector3(3, 3, 3));
    {
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(0, 40, 0));

        btScalar mass(1000.0f);
        btVector3 inertia(0, 0, 0);

        HSMotionState *motion = OGRE_NEW HSMotionState(n_heli, transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion, box, inertia);
        btRigidBody *body = OGRE_NEW btRigidBody(rbInfo);

        m_physics->getDynamics()->addRigidBody(body);
        m_physics->addObject(motion);
    }
#endif
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
