// -----------------------------------------------------------------------------
// File:    ApplicationFrame.cpp
// Authors: Garrett Smith
//
// Wraps the main window UI script and provides the application view.
// -----------------------------------------------------------------------------

#include <QMessageBox>
#include <QTimer>
#include <QTextStream>
#include <iostream>
#include "ApplicationFrame.h"
#include "ConnectionDialog.h"
#include "SettingsDialog.h"
#include "Utility.h"

using namespace std;

// -----------------------------------------------------------------------------
ApplicationFrame::ApplicationFrame(DeviceController *controller, 
        bool show_virtview)
: m_virtual(NULL), m_file(NULL), m_log(NULL), m_logging(false),
  m_controller(controller)
{
    setupUi(this);
    setupControllerPane();
    setupCameraView();
    setupSensorView();

    if(show_virtview)
        setupVirtualView();

    setupStatusBar();
    setupDeviceController();
    
    // attempt to open the specified device
    if (!m_controller->open())
    {
        qDebug() << "failed to open device" << m_controller->device();
    }
    
    setupGamepad();
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

    connect(m_video, SIGNAL(updateLog(const char *, int)), 
            this, SLOT(onUpdateLog(const char *, int)));
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setupControllerPane()
{
    m_ctlview = new ControllerView(controllerWidget);
    controllerWidget->layout()->addWidget(m_ctlview);
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
void ApplicationFrame::setupStatusBar()
{
    m_connStat = new QLabel("No Connection");
    m_connStat->setFrameStyle(QFrame::Box);
    m_connStat->setMinimumWidth(300);
    m_connStat->setMaximumWidth(300);
    statusBar()->addPermanentWidget(m_connStat);

    connectionStatusBar->setRange(0, 256);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setupDeviceController()
{
    connect(m_controller,
            SIGNAL(telemetryReady(float, float, float, int, int, int, int)),
            this,
            SLOT(onTelemetryReady(float, float, float, int, int, int, int)));

    connect(m_controller, SIGNAL(connectionStatusChanged(const QString&, bool)),
            this, SLOT(onConnectionStatusChanged(const QString&, bool)));

    connect(m_controller, SIGNAL(stateChanged(int)),
            this, SLOT(onStateChanged(int)));

    connect(m_controller, SIGNAL(videoFrameReady(const char *, size_t)),
            m_video, SLOT(onImageReady(const char *, size_t)));

    connect(m_controller, SIGNAL(trackStatusUpdate(bool, int, int, int, int)),
            m_video, SLOT(onTrackStatusUpdate(bool, int, int, int, int)));

    connect(m_video, SIGNAL(updateTracking(int, int, int, int, int)),
            m_controller, SLOT(onUpdateTrackColor(int, int, int, int, int)));

    connect(m_controller, SIGNAL(updateLog(const char *, int)), 
            this, SLOT(onUpdateLog(const char *, int)));
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setupGamepad()
{
    m_gamepad = CreateGamepad();
    const QString dev = "/dev/input/js0";

    if (NULL == m_gamepad)
    {
        //onUpdateLog("AppFrame: could not create gamepad object\n", LOG_FILE);
        qDebug() << "could not create gamepad object";
    }
    if (!m_gamepad->open(dev))
    {
        //onUpdateLog("AppFrame: failed to open /dev/input/js0\n", LOG_FILE);
        qDebug() << "failed to open /dev/input/js0";
    }
    else
    {
        //char msg[128];
        //sprintf(msg, "AppFrame: successfully opened %s\n", m_gamepad->driverName().toAscii().constData());
        //sprintf(msg, "%sAppFrame:   version: %d\n", msg, m_gamepad->driverVersion());
        //sprintf(msg, "%sAppFrame:   buttons: %d\n", msg, m_gamepad->buttonCount());
        //sprintf(msg, "%sAppFrame:   axes:    %d\n", msg, m_gamepad->axisCount());
        //onUpdateLog(msg, LOG_FILE);
        qDebug() << "successfully opened" << m_gamepad->driverName();
        qDebug() << "   version: " << m_gamepad->driverVersion();
        qDebug() << "   buttons: " << m_gamepad->buttonCount();
        qDebug() << "   axes:    " << m_gamepad->axisCount();

        connect(m_gamepad, SIGNAL(inputReady(GamepadEvent, int, float)),
                m_ctlview, SLOT(onInputReady(GamepadEvent, int, float)));

        connect(m_gamepad, SIGNAL(inputReady(GamepadEvent, int, float)),
                m_controller, SLOT(onInputReady(GamepadEvent, int, float)));

        m_gamepad->start();
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setEnabledButtons(int buttons)
{
    btnLanding->setEnabled(buttons & BUTTON_LANDING);
    btnTakeoff->setEnabled(buttons & BUTTON_TAKEOFF);
    btnOverride->setEnabled(buttons & BUTTON_OVERRIDE);
    btnKillswitch->setEnabled(buttons & BUTTON_KILLSWITCH);
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
void ApplicationFrame::onUpdateLog(const char *msg, int log_flags)
{
    if (log_flags & LOG_DIALOG)
    {
        txtCommandLog->textCursor().insertText(QString(msg));
    }

    if ((log_flags & LOG_FILE) && m_logging && m_log)
    {
        QString message(msg);
        (*m_log) << message;
        m_log->flush();
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onTelemetryReady(
    float yaw, float pitch, float roll, int alt, int rssi, int batt, int aux)
{
    static float time = 0.0f;

    if (m_virtual)
        m_virtual->setAngles(yaw, pitch, roll);

    connectionStatusBar->setValue(rssi);
    connectionStatusBar->setFormat(QString("%1 dBm").arg(rssi));

    batteryStatusBar->setValue(batt);
    batteryStatusBar->setFormat(QString("%p%"));

    elevationStatusBar->setValue(alt);
    elevationStatusBar->setFormat(QString("%1 inches").arg(alt));

    aux = (int)(100.0f * ((aux - 900.0f) / 1150.0f));
    aux = min(max(0, aux), 100);

    auxiliaryStatusBar->setValue(aux);
    auxiliaryStatusBar->setFormat(QString("%p%"));

    m_graphs[0]->addDataPoint(time, yaw + 180.0f, 0.0f);
    m_graphs[1]->addDataPoint(time, pitch + 180.0f, 0.0f);
    m_graphs[2]->addDataPoint(time, roll + 180.0f, 0.0f);

    time += 0.5f;
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

// -----------------------------------------------------------------------------
void ApplicationFrame::onStateChanged(int state)
{
    switch ((DeviceState)state)
    {
    case STATE_RADIO_CONTROL:
        setEnabledButtons(BUTTON_OVERRIDE | BUTTON_KILLSWITCH);
        btnOverride->setText("Release Manual");
        m_ctlview->setEnabled(false);
        break;
    case STATE_MIXED_CONTROL:
        setEnabledButtons(BUTTON_OVERRIDE | BUTTON_KILLSWITCH);
        m_ctlview->setEnabled(true);
        m_ctlview->setAxes(m_controller->currentAxes());
        break;
    case STATE_AUTONOMOUS:
        setEnabledButtons(BUTTON_ALL);
        btnOverride->setText("Manual Override");
        m_ctlview->setEnabled(false);
        break;
    case STATE_KILLED:
    case STATE_LOCKOUT:
        setEnabledButtons(BUTTON_NONE);
        m_ctlview->setEnabled(false);
        break;
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onFileConnectTriggered()
{
    ConnectionDialog cd(this, &m_controller);
    cd.exec();
    
    if( cd.result() == QDialog::Accepted) {
        setupDeviceController();
        setupGamepad();
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onFileExitTriggered()
{
    qDebug() << "TODO: onFileExitTriggered()";
    close();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onEditSettingsTriggered()
{
    SettingsDialog sd(this, m_controller->currentTrackSettings());

    // allow settings dialog to communicate data to device controller
    connect(&sd, SIGNAL(updateTracking(int, int, int, int, int)),
            m_controller, SLOT(onUpdateTrackColor(int, int, int, int, int)));

    sd.exec();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onHelpAboutTriggered()
{
    QMessageBox mb(QMessageBox::Information,
                   "About HeliView",
                   "HeliView: Remote Helicoptor Control Station\n\n"
                   "Authors:"
                   "\tGarrett Smith\n"
                   "\tKevin Macksamie\n"
                   "\tTyler Thierolf\n"
                   "\tTimothy Miller");
    mb.exec();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onSaveFrameTriggered()
{
    bool saved = m_video->saveFrame();

    if (saved)
    {
        onUpdateLog("Frame saved\n", LOG_ALL);
        qDebug() << "Frame saved";
    }
    else
    {
        onUpdateLog("Frame not saved\n", LOG_ALL);
        qDebug() << "Frame not saved";
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onTakeoffClicked()
{
    m_controller->requestTakeoff();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onLandingClicked()
{
    m_controller->requestLanding();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onManualOverrideClicked()
{
    switch (m_controller->currentState())
    {
    case STATE_AUTONOMOUS:
        m_controller->requestManualOverride();
        break;
    case STATE_RADIO_CONTROL:
    case STATE_MIXED_CONTROL:
        m_controller->requestAutonomous();
        break;
    default:
        // do nothing
        break;
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onKillswitchClicked()
{
    m_controller->requestKillswitch();
}

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

    // if no graphs are displayed, place a message label indicating so
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

