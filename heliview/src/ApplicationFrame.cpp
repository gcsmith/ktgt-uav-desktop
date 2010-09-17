// -----------------------------------------------------------------------------
// File:    ApplicationFrame.cpp
// Authors: Garrett Smith
//
// Wraps the main window UI script and provides the application view.
// -----------------------------------------------------------------------------

#include <QTimer>
#include <QTextStream>
#include <iostream>
#include "ApplicationFrame.h"
#include "Utility.h"

using namespace std;

// -----------------------------------------------------------------------------
ApplicationFrame::ApplicationFrame(DeviceController *controller, 
        bool show_virtview)
: m_virtual(NULL), m_file(NULL), m_log(NULL), m_logging(false),
  m_controller(controller)
{
    setupUi(this);
    setupCameraView();
    setupSensorView();

    if(show_virtview)
        setupVirtualView();

    m_connStat = new QLabel("No Connection");
    m_connStat->setFrameStyle(QFrame::Box);
    m_connStat->setMinimumWidth(300);
    m_connStat->setMaximumWidth(300);
    statusBar()->addPermanentWidget(m_connStat);

    connectionStatusBar->setRange(0, 200);

    connect(m_controller, SIGNAL(telemetryReady(float, float, float, int, int, int)),
            this, SLOT(onTelemetryReady(float, float, float, int, int, int)));

    connect(m_controller, SIGNAL(connectionStatusChanged(const QString&, bool)),
            this, SLOT(onConnectionStatusChanged(const QString&, bool)));

    // attempt to open the specified device
    if (!m_controller->open())
    {
        qDebug() << "failed to open device" << m_controller->device();
    }
}

// -----------------------------------------------------------------------------
ApplicationFrame::~ApplicationFrame()
{
    for (int i = 0; i < AXIS_COUNT; ++i)
    {
        SafeDelete(m_graphs[i]);
    }

    closeLogFile();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setupCameraView()
{
    m_video = new VideoView(tabPaneCamera);
    tabPaneCameraLayout->addWidget(m_video);
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
        SafeDelete(m_file);
        SafeDelete(m_log);
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::enableLogging(bool enable)
{
    m_logging = enable;
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onTelemetryReady(
    float yaw, float pitch, float roll, int alt, int rssi, int batt)
{
//  cerr << "received telemetry " << yaw << ", " << pitch << ", " << roll << endl;
    if (m_virtual) m_virtual->setAngles(yaw, pitch, roll);

    connectionStatusBar->setValue(rssi);
    connectionStatusBar->setFormat(QString("%1 dBm").arg(rssi));

    batteryStatusBar->setValue(batt);
    batteryStatusBar->setFormat(QString("%p%"));

    elevationStatusBar->setValue(alt);
    elevationStatusBar->setFormat(QString("%1 inches").arg(alt));
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onConnectionStatusChanged(const QString &text, bool status)
{
    m_connStat->setText(text);
    if (!status)
    {
        connectionStatusBar->setValue(0);
        connectionStatusBar->setFormat(QString("NC"));

        batteryStatusBar->setValue(0);
        batteryStatusBar->setFormat(QString("NC"));

        elevationStatusBar->setValue(0);
        elevationStatusBar->setFormat(QString("NC"));
    }
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
    // if (m_camera) m_camera->setRunning(index == 0);
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

