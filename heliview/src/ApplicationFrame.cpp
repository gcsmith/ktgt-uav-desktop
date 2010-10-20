// -----------------------------------------------------------------------------
// File:    ApplicationFrame.cpp
// Authors: Garrett Smith, Kevin Macksamie, Tyler Thierolf, Timothy Miller
// Created: 08-23-2010
//
// Wraps the main window UI script and provides the application view.
// -----------------------------------------------------------------------------

#include <QDateTime>
#include <QMessageBox>
#include <QTimer>
#include <QTextStream>
#include <iostream>
#include "ApplicationFrame.h"
#include "ConnectionDialog.h"
#include "Logger.h"
#include "SettingsDialog.h"
#include "Utility.h"

using namespace std;

// -----------------------------------------------------------------------------
ApplicationFrame::ApplicationFrame(bool noVirtualView)
: m_virtual(NULL), m_video(NULL), m_file(NULL), m_log(NULL), m_logbuffer(NULL),
  m_bufsize(1024), m_logging(false), m_controller(NULL), m_gamepad(NULL)
{
    setupUi(this);

    setupStatusBar();
    setupControllerPane();
    setupCameraView();
    setupSensorView();

    if (!noVirtualView)
        setupVirtualView();

    m_logbuffer = new QByteArray();

    connect(Logger::instance(), SIGNAL(updateLog(int, const QString &)), this,
            SLOT(onUpdateLog(int, const QString &)));

    onConnectionStatusChanged("No connection", false);
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
void ApplicationFrame::connectDeviceController()
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
}

// -----------------------------------------------------------------------------
void ApplicationFrame::connectGamepad()
{
    const QString dev = "/dev/input/js0";

    // if a gamepad is already opened, close and destroy it
    if (m_gamepad)
    {
        m_gamepad->close();
        SafeDelete(m_gamepad);
    }

    // attempt to allocate the gamepad object
    m_gamepad = CreateGamepad();
    if (NULL == m_gamepad)
    {
        Logger::err("failed to create gamepad object");
        return;
    }

    // attempt to open the gamepad device
    if (!m_gamepad->open(dev))
    {
        Logger::err(tr("failed to open gamepad device '%1'\n").arg(dev));
        SafeDelete(m_gamepad);
        return;
    }

    Logger::info(tr("opened gamepad '%1' - version:%2 buttons:%3 axes:%4\n")
            .arg(m_gamepad->driverName())
            .arg(m_gamepad->driverVersion())
            .arg(m_gamepad->buttonCount())
            .arg(m_gamepad->axisCount()));

    connect(m_gamepad, SIGNAL(inputReady(GamepadEvent, int, float)),
            m_ctlview, SLOT(onInputReady(GamepadEvent, int, float)));


    connect(m_gamepad, SIGNAL(inputReady(GamepadEvent, int, float)),
            m_controller, SLOT(onInputReady(GamepadEvent, int, float)));

    m_gamepad->start();
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
    if (!m_file)
    {
        m_file = new QFile(logfile);
        if (!m_file)
        {
            Logger::fail("failed to allocate file object\n");
            return;
        }
    }

    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Logger::err("could not open new log file\n");
        return;
    }
    Logger::info(tr("successfully opened log '%1'\n").arg(m_file->fileName()));

    if (!m_log)
    {
        m_log = new QTextStream();
        if (!m_log)
        {
            Logger::fail("failed to allocate log stream\n");
            return;
        }
    }
    m_log->setDevice(m_file);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::closeLogFile()
{
    if (m_file)
    {
        m_log->flush();
        m_file->close();
        SafeDelete(m_file);
        SafeDelete(m_log);
    }
}

// -----------------------------------------------------------------------------
bool ApplicationFrame::enableLogging(bool enable, const QString &verbosity)
{
    m_logging = enable;

    if (m_logging)
    {
        if (verbosity == "excess")
            m_verbosity = LOG_MODE_EXCESSIVE;
        else if (verbosity == "normal")
            m_verbosity = LOG_MODE_NORMAL;
        else if (verbosity == "debug")
            m_verbosity = LOG_MODE_NORMAL_DEBUG;
        else
        {
            m_verbosity = LOG_MODE_NORMAL;
            return false;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
void ApplicationFrame::writeToLog(const QString &plain, const QString &rich)
{
    txtCommandLog->textCursor().insertHtml(rich);
   
    QTextCursor c = txtCommandLog->textCursor();
    c.movePosition(QTextCursor::End);
    txtCommandLog->setTextCursor(c);

    m_logbuffer->append(plain);
    if (m_logbuffer->size() >= m_bufsize)
    {
        if (m_log)
        {
            // only write to file when we reach bufsize limit
            *m_log << *m_logbuffer;
            m_log->flush();
        }
        m_logbuffer->clear();
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onUpdateLogFile(const QString &file, int bufsize)
{
    // close last file opened
    m_log->flush();
    m_file->flush();
    m_file->close();
    SafeDelete(m_file);

    // clear buffer to start fresh
    m_logbuffer->clear();

    // open lof file and set buffer size limit
    openLogFile(file);
    m_bufsize = bufsize * 1024;
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onUpdateLog(int type, const QString &msg)
{
    if (!m_logging)
        return;

    QString plain_msg = msg; // copy for plain text
    QString rich_msg = msg;  // copy for rich text

    rich_msg.replace(QString("\n"), QString("<br>"));
    rich_msg.replace(QString(" "), QString("&nbsp;"));

    switch (type)
    {
        case LOG_TYPE_FAIL:
            rich_msg.prepend("<font color=red><b>[failure] ");
            rich_msg.append("</b></font>");
            plain_msg.prepend("failure: ");
            break;
        case LOG_TYPE_ERR:
            rich_msg.prepend("<font color=red>[error] ");
            rich_msg.append("</font>");
            plain_msg.prepend("error: ");
            break;
        case LOG_TYPE_WARN:
            rich_msg.prepend("<font color=orange>[warning] ");
            rich_msg.append("</font>");
            plain_msg.prepend("warning: ");
            break;
        case LOG_TYPE_INFO:
            break;
        case LOG_TYPE_DBG:
        case LOG_TYPE_EXTRADEBUG:
            rich_msg.prepend("<font color=forestgreen>[debug] ");
            rich_msg.append("</font>");
            plain_msg.prepend("debug: ");
            break;
        default:
            // type is comprised of multiple flags
            break;
    }

    switch (m_verbosity)
    {
        case LOG_MODE_EXCESSIVE:
        // excessive mode logs any type of log message
        // print debug statements to cmd line as well
        if ((type & LOG_TYPE_DBG) || (type & LOG_TYPE_EXTRADEBUG))
        {
            cerr << plain_msg.toAscii().constData() << endl;
        }
        
        writeToLog(plain_msg, rich_msg);
        break;
        case LOG_MODE_NORMAL_DEBUG:
        // log debug messages plus normal messages
        case LOG_MODE_NORMAL:
        // normal mode watches for failures, errors, warnings, information
        if ((type & LOG_TYPE_FAIL) ||
            (type & LOG_TYPE_ERR)   ||
            (type & LOG_TYPE_WARN) ||
            (type & LOG_TYPE_INFO))
        {
            writeToLog(plain_msg, rich_msg);
        }
        else if ((m_verbosity == LOG_MODE_NORMAL_DEBUG) && (type & LOG_TYPE_DBG))
        {
            cerr << plain_msg.toAscii().constData() << endl;
            writeToLog(plain_msg, rich_msg);
        }
        break;
        default:
        break;
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

    btnLanding->setEnabled(status);
    btnTakeoff->setEnabled(status);
    btnOverride->setEnabled(status);
    btnKillswitch->setEnabled(status);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onStateChanged(int state)
{
    switch ((DeviceState)state)
    {
    case STATE_RADIO_CONTROL:
        setEnabledButtons(BUTTON_OVERRIDE | BUTTON_KILLSWITCH);
        btnOverride->setText("Release Radio Control");
        m_ctlview->setEnabled(false);
        break;
    case STATE_MIXED_CONTROL:
        setEnabledButtons(BUTTON_OVERRIDE | BUTTON_KILLSWITCH);
        btnOverride->setText("Release Mixed Control");
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
    ConnectionDialog cd(this);
    connect(&cd, SIGNAL(requestConnection(const QString &, const QString &)),
            this, SLOT(connectTo(const QString &, const QString &)));
    cd.exec();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onFileDisconnectTriggered()
{
    if (m_controller)
    {
        Logger::info(tr("disconnecting \"%1\"\n").arg(m_controller->device()));
        m_controller->close();
        SafeDelete(m_controller);
    }
}

// -----------------------------------------------------------------------------
bool ApplicationFrame::connectTo(const QString &source, const QString &device)
{
    onFileDisconnectTriggered();

    // attempt to allocate the specified device controller
    m_controller = CreateDeviceController(source, device);
    if (!m_controller)
    {
        Logger::fail(tr("failed to allocate \"%1\"\n").arg(source));
        return false;
    }

    // connect the signals and slots for this device
    connectDeviceController();

    // attempt to connect to the device
    if (!m_controller->open())
    {
        Logger::err(tr("failed to open device \"%1\"\n").arg(device));
        SafeDelete(m_controller);
        return false;
    }

    connectGamepad();
    return true;
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
    QString filename = m_file ? m_file->fileName() : "";
    TrackSettings track;
    if (m_controller)
        track = m_controller->currentTrackSettings();

    SettingsDialog sd(this, track, filename, m_bufsize/1024);
    if (m_controller)
    {
        // allow settings dialog to communicate data to device controller
        connect(&sd, SIGNAL(updateTracking(int, int, int, int, int)),
                m_controller, SLOT(onUpdateTrackColor(int, int, int, int, int)));
    }
    
    connect(&sd, SIGNAL(updateLogFile(const QString &, int)), this, 
                SLOT(onUpdateLogFile(const QString &, int)));

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
void ApplicationFrame::onFileSaveLogTriggered()
{
    if (!m_logging)
    {
        QMessageBox mb(QMessageBox::Information,
                "Information",
                "Logging is disabled");
        mb.exec();
    }
    else
    {
        Logger::info("Log successfully saved\n");

        *m_log << *m_logbuffer;
        m_log->flush();
        m_file->flush();
    }
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onFileSaveFrameTriggered()
{
    bool saved = m_video->saveFrame();

    if (saved)
        Logger::info(("Frame successfully saved\n"));
    else
        Logger::warn(("Failed to save frame\n"));
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onTakeoffClicked()
{
    if (!m_controller)
    {
        assert(!"attempting takeoff request in disconnected state");
        Logger::fail("attempting takeoff request in disconnected state\n");
        return;
    }

    m_controller->requestTakeoff();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onLandingClicked()
{
    if (!m_controller)
    {
        assert(!"attempting landing request in disconnected state");
        Logger::fail("attempting landing request in disconnected state");
        return;
    }

    m_controller->requestLanding();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onManualOverrideClicked()
{
    if (!m_controller)
    {
        assert(!"attempting override request in disconnected state");
        Logger::fail("attempting override request in disconnected state");
        return;
    }

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
    if (!m_controller)
    {
        assert(!"attempting killswitch request in disconnected state");
        Logger::fail("attempting killswitch request in disconnected state");
        return;
    }

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

