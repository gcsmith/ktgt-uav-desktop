// -----------------------------------------------------------------------------
// File:    VirtualView.cpp
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#include <iostream>
#include "VirtualView.h"
#include "Utility.h"

using namespace std;
using namespace Ogre;

// -----------------------------------------------------------------------------
VirtualView::VirtualView(QWidget *parent)
: QGLWidget(parent), m_angle(0.0f), m_yaw(0.0f), m_pitch(0.0f), m_roll(0.0f)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(20);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onPaintTick()));

    // Create the main Ogre object
    mOgreRoot = new Root(
            "cfg/plugins.cfg", 
            "cfg/ogre.cfg", 
            "cfg/ogre.log");

    mOgreRoot->loadPlugin("RenderSystem_GL");

    // load resource paths from config file
    ConfigFile cf;
    cf.load("cfg/resources.cfg");

    // iterate over each setting and add resource patch
    ConfigFile::SectionIterator seci = cf.getSectionIterator();
    String section, type, arch;

    while (seci.hasMoreElements())
    {
        section = seci.peekNextKey();
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            type = i->first;
            arch = i->second;
            ResourceGroupManager::getSingleton().addResourceLocation(
                    arch, type, section);
        }
    }

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

    // Initialize without creating a window
    mOgreRoot->getRenderSystem()->setConfigOption("Full Screen", "No");
    mOgreRoot->saveConfig();

    // Don't create a window
    mOgreRoot->initialise(false);
}

// -----------------------------------------------------------------------------
VirtualView::~VirtualView()
{
    // mOgreRoot->shutdown();
    delete mOgreRoot;
    destroy();
}

// -----------------------------------------------------------------------------
void VirtualView::initializeGL()
{
#if 0
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
#endif

    // Get the parameters of the window QT created
    QX11Info info = x11Info();
    String winHandle;

    winHandle = StringConverter::toString((unsigned long)(info.display()));

    winHandle += ":";

    winHandle += StringConverter::toString((unsigned long)(info.screen()));

    winHandle += ":";

    winHandle += StringConverter::toString(
            (unsigned long)(this->parentWidget()->winId()));

    NameValuePairList params;
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
    SceneType scene_manager_type = ST_EXTERIOR_CLOSE;

    mSceneMgr = mOgreRoot->createSceneManager(scene_manager_type);

    // Set light & fog
    mSceneMgr->setAmbientLight(ColourValue(1, 1, 1));
    mSceneMgr->setFog(
            FOG_LINEAR, 
            ColourValue(1, 1, 0.8f), 
            0, 
            80, 
            200);

    mCamera = mSceneMgr->createCamera("QOgreWidget_Cam");
    mCamera->setPosition(Vector3(0, 1, 0));
    mCamera->lookAt(Vector3(0, 0, 0));
    mCamera->setNearClipDistance(5.0);

    Viewport *mViewport = mOgreWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(ColourValue(0.8, 0.8, 1));

    // Create scene
    SceneNode *n_root = mSceneMgr->getRootSceneNode();

    TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Initialize resources
    ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    //m_physics = new HSPhysicsWorld();
    //m_physics->getDynamics()->setGravity(btVector3(0, -9.8, 0));
    //m_physics->setDebugMode(false);

    // Create the floor plane's mesh and entity
    MeshManager::getSingleton().createPlane(
            "floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Plane(Vector3::UNIT_Y, 0), 500, 500, 10, 10,
            true, 1, 10, 10, Vector3(1,1,1));

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
    e_heli = mSceneMgr->createEntity("Helicopter", "apache_body.mesh");
    n_heli = n_root->createChildSceneNode("HeliNode");
    n_heli->setPosition(Vector3(0, 40, 0));
    n_heli->setScale(Vector3(3, 3, 3));
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
RenderSystem* VirtualView::chooseRenderer(RenderSystemList *renderers)
{
    RenderSystemList::iterator i = renderers->begin();
    return *i;

#if 0
    while(i != renderers->end())
    {
        // Debugging purposes
        std::cout << "Renderer Name: " << (*i)->getName() << std::endl;

        if((*i)->getName() == "OpenGL Rendering Subsystem")
            break;

        i++;
    }

    return *i;
#endif
}

// -----------------------------------------------------------------------------
void VirtualView::setAngles(float yaw, float pitch, float roll)
{
    m_yaw   = yaw;
    m_pitch = pitch;
    m_roll  = roll;
}

// -----------------------------------------------------------------------------
void VirtualView::resizeGL(int width, int height)
{
#if 0
    glViewport(1.0f, 1.0f, (float)width, (float)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0f, 1.0f, 1.0f, 1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

#define PI_180 3.141592654f / 180.0f

// -----------------------------------------------------------------------------
void VirtualView::paintGL()
{
#if 0
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

    glRotatef(-m_yaw, 0.0f, 1.0f, 0.0f);
    glRotatef(m_pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-m_roll, 0.0f, 0.0f, 1.0f);

    glBegin(GL_QUADS);
    glColor3f(3.0f, 0.0f, 0.0f);
    glVertex3f(-3.0f, 0.0f, -3.0f);
    glVertex3f(-3.0f, 0.0f, 3.0f);
    glVertex3f(3.0f, 0.0f, 3.0f);
    glVertex3f(3.0f, 0.0f, -3.0f);
    glEnd();
#endif

    // Convert Euler angles to quaternion
    Real w = cos(-m_roll/2)*cos(m_pitch/2)*cos(-m_yaw/2) + 
             sin(-m_roll/2)*sin(m_pitch/2)*sin(-m_yaw/2);

    Real x = sin(-m_roll/2)*cos(m_pitch/2)*cos(-m_yaw/2) - 
             cos(-m_roll/2)*sin(m_pitch/2)*sin(-m_yaw/2);

    Real y = cos(-m_roll/2)*sin(m_pitch/2)*cos(-m_yaw/2) + 
             sin(-m_roll/2)*cos(m_pitch/2)*sin(-m_yaw/2);

    Real z = cos(-m_roll/2)*cos(m_pitch/2)*sin(-m_yaw/2) - 
             sin(-m_roll/2)*sin(m_pitch/2)*cos(-m_yaw/2);

    n_heli->setOrientation(w,x,y,z);
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
    //m_angle += 0.1f;
    updateGL();
}

