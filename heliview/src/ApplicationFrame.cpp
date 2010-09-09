// -----------------------------------------------------------------------------
// File:    ApplicationFrame.cpp
// Authors: Garrett Smith
//
// Wraps the main window UI script and provides the application view.
// -----------------------------------------------------------------------------

#include <QTimer>
#include <iostream>
#include "ApplicationFrame.h"
#include "Utility.h"

using namespace std;

// -----------------------------------------------------------------------------
ApplicationFrame::ApplicationFrame(DeviceController *controller)
: m_camera(NULL), m_virtual(NULL), m_offset(0), m_index(0.0),
  m_file(NULL), m_log(NULL), m_logging(false), m_controller(controller)
{
    setupUi(this);
    setupCameraView();
    setupSensorView();
    setupVirtualView();

    connect(m_controller, SIGNAL(telemetryReady(float, float, float)),
            this, SLOT(onTelemetryReady(float, float, float)));
}

// -----------------------------------------------------------------------------
ApplicationFrame::~ApplicationFrame()
{
    for (int i = 0; i < AXIS_COUNT; ++i)
    {
        delete m_graphs[i];
        m_graphs[i] = NULL;
    }

    closeLogFile();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setupCameraView()
{
    // m_camera = new CVWebcamView(tabPaneCamera);
    // tabPaneCameraLayout->addWidget(m_camera);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setupSensorView()
{
    QBoxLayout *layout = (QBoxLayout *)tabPaneSensors->layout();

    for (int i = 0; i < AXIS_COUNT; ++i)
    {
        m_graphs[i] = new LineGraph(tabPaneSensors);
        layout->insertWidget(i, m_graphs[i]->getPlot());
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setupVirtualView()
{
    m_virtual = new VirtualView(tabPaneVirtual);
    m_virtual->initialize();
    tabPaneVirtualLayout->addWidget(m_virtual);

    //m_virtual = new OgreWidget(tabPaneVirtual);
    //tabPaneVirtualLayout->addWidget(m_virtual);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::attachSimulatedSource(bool noise)
{
    m_noise = noise;

    for (int i = 0; i < 9; i++)
        m_ro[i] = 360.0f * qrand() / (float)RAND_MAX;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onSimulateTick()));
    timer->start(20);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::openLogFile(const QString &logfile)
{
    m_file = new QFile(logfile);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    m_log = new QTextStream(m_file);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::closeLogFile()
{
    if (m_file)
    {
        m_file->close();
        delete m_file;
        delete m_log;
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::enableLogging(bool enable)
{
    m_logging = enable;
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onSimulateTick()
{
    m_index += 0.5;

    for (int i = 0; i < 3; i++)
    {
        float ra = 512 + 300 * sin(10 * m_index * RAD + m_ro[i*3]);
        float rg = 512 + 300 * sin((10 * m_index + m_ro[i*3+1]) * RAD + m_ro[i*3+2]);

        if (m_noise && (qrand() % 3) != 0)
        {
            ra += (500.0f * qrand() / (float)RAND_MAX - 250.0f);
            rg += (500.0f * qrand() / (float)RAND_MAX - 250.0f);
        }

        m_graphs[i]->addDataPoint(m_index, ra, rg);
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onTelemetryReady(float yaw, float pitch, float roll)
{
    cerr << "received telemetry " << yaw << ", " << pitch << ", " << roll << endl;
    if (m_virtual) m_virtual->setAngles(yaw, pitch, roll);
}

#if 0
// -----------------------------------------------------------------------------
void ApplicationFrame::updateLine(const char *message)
{
    QString msg = QString(message);
    if (msg.length() != 14)
    {
        printf("<bad length %d>\n", msg.length());
        return;
    }

    bool ok;
    qlonglong data = msg.toLongLong(&ok, 16);
    if (!ok)
    {
        printf("<bad conversion>\n");
        return;
    }

    int ax = (data >> 48) & 0xFF;
    int ay = (data >> 40) & 0xFF;
    int az = (data >> 32) & 0xFF;
    int gx = (data >> 24) & 0xFF;
    int gy = (data >> 16) & 0xFF;
    int gz = (data >>  8) & 0xFF;
    int us = data & 0xFF;

#if 0
    if ((ax < 0) || (ax > 255) || (ay < 0) || (ay > 255) || (az < 0) || (az > 255))
    {
        printf("<bad accelerometer range>\n");
        return;
    }

    if ((gx < 0) || (gx > 255) || (gy < 0) || (gy > 255) || (gz < 0) || (gz > 255))
    {
        printf("<bad gyro range>\n");
        return;
    }
#endif

    printf("%d %d %d %d %d %d %d\n", ax, ay, az, gx, gy, gz, us);

    if (m_logging)
    {
        *m_log << ax << " " << ay << " " << az << " "
               << gx << " " << gy << " " << gz << endl;
    }

    m_index += 0.5;

    m_graphs[0]->addDataPoint(m_index, ax, gx);
    m_graphs[1]->addDataPoint(m_index, ay, gy);
    m_graphs[2]->addDataPoint(m_index, az, gz);
}
#endif

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowXFChanged(bool flag)
{
    m_graphs[0]->toggleAcceleration(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowXUFChanged(bool flag)
{
    m_graphs[0]->toggleRawVelocity(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowYFChanged(bool flag)
{
    m_graphs[1]->toggleAcceleration(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowYUFChanged(bool flag)
{
    m_graphs[1]->toggleRawVelocity(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowZFChanged(bool flag)
{
    m_graphs[2]->toggleAcceleration(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowZUFChanged(bool flag)
{
    m_graphs[2]->toggleRawVelocity(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onTabChanged(int index)
{
    if (m_camera) m_camera->setRunning(index == 0);
    if (m_virtual) m_virtual->setRunning(index == 2);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onGraphsChanged()
{
    bool visible = true;

    /* if no graphs are displayed, place a message label indicating so */
    for (int i = 0; i < AXIS_COUNT; ++i)
    {
        if (m_graphs[i]->isVisible())
        {
            visible = false;
            break;
        }
    }

    lblNoAxes->setVisible(visible);
}

