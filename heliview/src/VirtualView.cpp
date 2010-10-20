// -----------------------------------------------------------------------------
// File:    VirtualView.cpp
// Authors: Garrett Smith
// Created: 08-23-2010
//
// Displays a 3D representation of the helicopter's current orientation.
// -----------------------------------------------------------------------------

#include <iostream>
#include <OgreMath.h>
#include <QX11Info>
#include "VirtualView.h"
#include "Utility.h"

using namespace std;

// -----------------------------------------------------------------------------
VirtualView::VirtualView(QWidget *parent)
: QWidget(parent), m_time(0.0f), m_yaw(0.0f), m_pitch(0.0f), m_roll(0.0f),
  m_root(NULL), m_window(NULL), m_camera(NULL), m_view(NULL), m_scene(NULL)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(20);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onPaintTick()));

    // create log and disable debug output
    m_logmgr = OGRE_NEW Ogre::LogManager();
    m_log = Ogre::LogManager::getSingleton().createLog("ogre.log");
    m_log->setDebugOutputEnabled(false);

    // create the Ogre root object and load the OpenGL render plugin
    m_root = OGRE_NEW Ogre::Root("cfg/plugins.cfg", "ogre.cfg");
    m_root->loadPlugin("RenderSystem_GL");

    loadConfiguration();
    setupRenderSystem();
}

// -----------------------------------------------------------------------------
VirtualView::~VirtualView()
{
    m_root->shutdown();
    destroy();
}

// -----------------------------------------------------------------------------
void VirtualView::initialize()
{
    createRenderWindows();

    // create a generic scene manager
    m_scene = m_root->createSceneManager(Ogre::ST_GENERIC, "HeliViewScene");

    m_camera = m_scene->createCamera("PlayerCam");
    m_camera->setPosition(Ogre::Vector3(0, 40, 50));
    m_camera->lookAt(Ogre::Vector3(0, 40, 0));
    m_camera->setNearClipDistance(5);

    m_view = m_window->addViewport(m_camera);
    m_view->setBackgroundColour(Ogre::ColourValue(1, 1, 0.8f));

    // match camera aspect ratio to viewport
    Ogre::Real aspect = Ogre::Real(m_view->getActualWidth()) /
                        Ogre::Real(m_view->getActualHeight());
    m_camera->setAspectRatio(aspect);

    // initialize resource system
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    // create scene
    Ogre::SceneNode *n_root = m_scene->getRootSceneNode();

    m_scene->setAmbientLight(Ogre::ColourValue(1, 1, 0.8f));
    m_scene->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue(1, 1, 0.8f), 0, 80, 200);

    // create the floor plane's mesh and entity
    Ogre::MeshManager::getSingleton().createPlane(
            "floor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 500, 500, 10, 10,
            true, 1, 10, 10, Ogre::Vector3::UNIT_Z);

    Ogre::Entity *e_floor = m_scene->createEntity("Floor", "floor");
    e_floor->setMaterialName("world/dirt");
    e_floor->setCastShadows(false);
    n_root->attachObject(e_floor);

    // create and attach a helicopter entity and scene node
    e_heli = m_scene->createEntity("Apache", "apache_body.mesh");
    n_heli = n_root->createChildSceneNode("Apache");
    n_heli->setPosition(Ogre::Vector3(0, 40, 0));
    n_heli->setScale(Ogre::Vector3(3, 3, 3));
    n_heli->attachObject(e_heli);

    e_main_rotor = m_scene->createEntity("Apache_MRotor", "main_rotor.mesh");
    n_main_rotor = n_heli->createChildSceneNode("Apache_MRotor");
    n_main_rotor->setPosition(0.0f, 0.987322f, 0.573885f);
    n_main_rotor->attachObject(e_main_rotor);

    e_tail_rotor = m_scene->createEntity("Apache_TRotor", "tail_rotor.mesh");
    n_tail_rotor = n_heli->createChildSceneNode("Apache_TRotor");
    n_tail_rotor->setPosition(0.174927f, 0.173132f, -3.50708f);
    n_tail_rotor->attachObject(e_tail_rotor);
}

// -----------------------------------------------------------------------------
Ogre::RenderSystem* VirtualView::chooseRenderer(const Ogre::RenderSystemList &renderers)
{
    Ogre::RenderSystemList::const_iterator i = renderers.begin();
#if 0
    while(i != renderers.end())
    {
        // Debugging purposes
        std::cout << "Renderer Name: " << (*i)->getName() << std::endl;

        if((*i)->getName() == "OpenGL Rendering Subsystem")
            break;

        i++;
    }
#endif

    return *i;
}

#ifdef PLATFORM_UNIX_GCC
#include <X11/Xlib.h>
#endif

// -----------------------------------------------------------------------------
void VirtualView::createRenderWindows()
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoBackground);
    setAttribute(Qt::WA_NoSystemBackground);

    Ogre::NameValuePairList params;
    Ogre::String win_handle;
#ifdef PLATFORM_UNIX_GCC
    const QX11Info info = this->x11Info();
    win_handle = Ogre::StringConverter::toString((unsigned long)(info.display()));
    win_handle += ":";
    win_handle += Ogre::StringConverter::toString((unsigned int)(info.screen()));
    win_handle += ":";
    win_handle += Ogre::StringConverter::toString((unsigned long)(winId()));
    win_handle += ":";
    win_handle += Ogre::StringConverter::toString((unsigned long)(info.visual()));
    XSync(info.display(), False);
    params["externalWindowHandle"] = win_handle;
#else
    win_handle = Ogre::StringConverter::toString((unsigned long)(winId()));
    params["externalWindowHandle"] = win_handle;
#endif
    m_window = m_root->createRenderWindow(
            "HeliView_RenderWindow",
            width(),
            height(),
            false,
            &params);

    m_window->setActive(true);
    m_window->setVisible(true);
}

// -----------------------------------------------------------------------------
void VirtualView::loadConfiguration()
{
    // load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load("cfg/resources.cfg");

    // iterate over each setting and add resource patch
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    Ogre::String section, type, arch;

    while (seci.hasMoreElements())
    {
        section = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            type = i->first;
            arch = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    arch, type, section);
        }
    }
}

// -----------------------------------------------------------------------------
void VirtualView::setupRenderSystem()
{
    // setup a renderer
    const Ogre::RenderSystemList &renderers = m_root->getAvailableRenderers();
    assert(!renderers.empty());

    Ogre::RenderSystem *renderSystem = chooseRenderer(renderers);
    assert(renderSystem);

    m_root->setRenderSystem(renderSystem);
    QString dimensions = QString("%1x%2").arg(width()).arg(height());
    renderSystem->setConfigOption("Video Mode", dimensions.toStdString());

    // initialize without creating a window
    m_root->getRenderSystem()->setConfigOption("Full Screen", "No");
    m_root->saveConfig();

    // don't create a window
    m_root->initialise(false);
}

// -----------------------------------------------------------------------------
void VirtualView::setAngles(float yaw, float pitch, float roll)
{
    // update current yaw/pitch/roll
    m_yaw   = yaw;
    m_pitch = pitch;
    m_roll  = roll;

    // convert yaw pitch and roll degrees to radians
    Ogre::Radian y(D2R(m_yaw));
    Ogre::Radian p(D2R(m_pitch));
    Ogre::Radian r(D2R(m_roll));

    // build a quaternion from the euler angles
    Ogre::Matrix3 m;
    m.FromEulerAnglesYXZ(y, p, r);
    Ogre::Quaternion q(m);

    // update helicopter orientation and render the next frame
    n_heli->setOrientation(q);
}

// -----------------------------------------------------------------------------
void VirtualView::paintEvent(QPaintEvent *)
{
    if (!m_window)
        initialize();

    m_root->renderOneFrame();
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
    assert(m_root);

    n_main_rotor->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(0.4f));
    n_tail_rotor->rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(0.4f));

    update();
}

// -----------------------------------------------------------------------------
void VirtualView::resizeEvent(QResizeEvent *event)
{
    if (m_window)
    {
        m_window->resize(width(), height());
        m_window->windowMovedOrResized();
        Ogre::Real aspect = Ogre::Real(m_view->getActualWidth()) / 
                            Ogre::Real(m_view->getActualHeight());
        m_camera->setAspectRatio(aspect);
    }
}

// -----------------------------------------------------------------------------
QPaintEngine *VirtualView::paintEngine() const
{
    return NULL;
}
