// -----------------------------------------------------------------------------
// File:    ApplicationFrame.cpp
// Authors: Garrett Smith
//
// Wraps the main window UI script and provides the application view.
// -----------------------------------------------------------------------------

#include <QTimer>
#include <iostream>
#include <algorithm>
#include "ApplicationFrame.h"
#include "Utility.h"

#define IDENT_MAGIC     0x09291988  // identification number
#define IDENT_VERSION   0x00000001  // software version

#define CLIENT_ACK_IDENT        0   // identify self to server
#define CLIENT_REQ_TAKEOFF      1   // command the helicopter to take off
#define CLIENT_REQ_LANDING      2   // command the helicopter to land
#define CLIENT_REQ_TELEMETRY    3   // state, orientation, altitude, battery

#define SERVER_REQ_IDENT        0   // request client to identify itself
#define SERVER_ACK_IGNORED      1   // client request ignored (invalid state)
#define SERVER_ACK_TAKEOFF      2   // acknowledge request to take off
#define SERVER_ACK_LANDING      3   // acknowledge request to land
#define SERVER_ACK_TELEMETRY    4   // acknowledge request for telemetry (+data)

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
    tabPaneVirtualLayout->addWidget(m_virtual);
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
void ApplicationFrame::onTelemetryTick()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int cmd_buffer[1] = { CLIENT_REQ_TELEMETRY };
    stream.writeRawData((char *)cmd_buffer, sizeof(int) * 1);
    cout << "requested telemetry" << endl;
}

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

// -----------------------------------------------------------------------------
void ApplicationFrame::onSocketReadyRead()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);
    int32_t cmd_buffer[32], altitude, battery;
    float x, y, z;
    memset((void *)cmd_buffer, 0, sizeof(int32_t) * 32);

    size_t num_bytes = m_sock->bytesAvailable();
    if (num_bytes < 4)
        return;

    num_bytes = max(num_bytes, sizeof(int32_t) * 32);
    stream.readRawData((char *)cmd_buffer, num_bytes);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTelemetryTick()));

    switch (cmd_buffer[0])
    {
    case SERVER_REQ_IDENT:
        cerr << "SERVER_REQ_IDENT: sending response...\n";
        cmd_buffer[0] = CLIENT_ACK_IDENT;
        cmd_buffer[1] = IDENT_MAGIC;
        cmd_buffer[2] = IDENT_VERSION;
        stream.writeRawData((char *)cmd_buffer, sizeof(int32_t) * 3);
        timer->start(50); // begin requesting telemetry
        break;
    case SERVER_ACK_IGNORED:
        cerr << "SERVER_ACK_IGNORED" << endl;
        break;
    case SERVER_ACK_TAKEOFF:
        cerr << "SERVER_ACK_TAKEOFF" << endl;
        break;
    case SERVER_ACK_LANDING:
        cerr << "SERVER_ACK_LANDING" << endl;
        break;
    case SERVER_ACK_TELEMETRY:
        x = *(float *)&cmd_buffer[1];
        y = *(float *)&cmd_buffer[2];
        z = *(float *)&cmd_buffer[3];
        altitude = cmd_buffer[4];
        battery = cmd_buffer[5];
        if (m_virtual) m_virtual->setAngles(x, z, y);
        fprintf(stderr, "SERVER_ACK_TELEMETRY -> (%f, %f, %f) A (%d) B (%d)\n",
                x, y, z, altitude, battery);
        break;
    default:
        cerr << "unknown server command (" << cmd_buffer[0] << ")\n";
        break;
    }

}

// -----------------------------------------------------------------------------
void ApplicationFrame::onSocketError(QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        cerr << "QAbstractSocket::RemoteHostClosedError" << endl;
        break;
    case QAbstractSocket::HostNotFoundError:
        cerr << "QAbstractSocket::HostNotFoundError" << endl;
        break;
    case QAbstractSocket::ConnectionRefusedError:
        cerr << "QAbstractSocket::ConnectionRefusedError" << endl;
        break;
    default:
        cerr << "unknown socket error" << endl;
        break;
    }
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

