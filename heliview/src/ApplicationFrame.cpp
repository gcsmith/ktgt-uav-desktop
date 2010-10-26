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
    SafeDelete(m_controller);
    SafeDelete(m_gamepad);
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
    //QBoxLayout *layout = (QBoxLayout *)tabPaneSensors->layout();

    // Tutorials showed creating a widget, giving it a layout, setting
    // that widget as the QScrollArea's widget, and then adding widgets to
    // the QScrollArea's widget.
    QWidget *graphArea = new QWidget;
    QVBoxLayout *graphLayout = new QVBoxLayout;
    graphArea->setLayout(graphLayout);
    scrollArea->setWidget(graphArea);

    // Create graphs specific to the data they will be displaying
    m_graphs[AXIS_X]     = new LineGraph(graphArea, "X-Axis",        256.0f);
    m_graphs[AXIS_Y]     = new LineGraph(graphArea, "Y-Axis",        256.0f);
    m_graphs[AXIS_Z]     = new LineGraph(graphArea, "Z-Axis",        256.0f);
    m_graphs[CONNECTION] = new LineGraph(graphArea, "Connectivity",  200.0f);
    m_graphs[BATTERY]    = new LineGraph(graphArea, "Battery",       100.0f);
    m_graphs[TRACKING]   = new LineGraph(graphArea, "Tracking",      100.0f);
    m_graphs[AUXILIARY]  = new LineGraph(graphArea, "Auxiliary",     100.0f);
    m_graphs[ELEVATION]  = new LineGraph(graphArea, "Elevation",      80.0f, 10.0f);
    m_graphs[CPU]        = new LineGraph(graphArea, "CPU",           100.0f);

    for (int i = 0; i < AXIS_COUNT; ++i)
    {
        QWidget *widget = m_graphs[i]->getPlot();
        widget->setMinimumHeight(200);
        widget->setMaximumHeight(400);
        graphLayout->insertWidget(i, m_graphs[i]->getPlot());
        //layout->insertWidget(i, m_graphs[i]->getPlot());
    }

    m_lblNoAxes = new QLabel;
    m_lblNoAxes->setText("No Axes Selected");
    m_lblNoAxes->setAlignment(Qt::AlignCenter);
    m_lblNoAxes->setVisible(true);
    graphLayout->insertWidget(AXIS_COUNT, m_lblNoAxes);
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
            SIGNAL(telemetryReady(float, float, float, int, int, int, int, int)),
            this,
            SLOT(onTelemetryReady(float, float, float, int, int, int, int, int)));

    connect(m_controller, SIGNAL(connectionStatusChanged(const QString&, bool)),
            this, SLOT(onConnectionStatusChanged(const QString&, bool)));

    connect(m_controller, SIGNAL(stateChanged(int)),
            this, SLOT(onStateChanged(int)));

    connect(m_controller, SIGNAL(videoFrameReady(const char *, size_t)),
            m_video, SLOT(onImageReady(const char *, size_t)));

    connect(m_controller, SIGNAL(trackStatusUpdate(bool, int, int, int, int)),
            m_video, SLOT(onTrackStatusUpdate(bool, int, int, int, int)));

    connect(m_video, SIGNAL(trackSettingsChanged(int, int, int, int, int, int, int)),
            m_controller,
            SLOT(updateTrackSettings(int, int, int, int, int, int, int)));
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
bool ApplicationFrame::openDevice()
{
    // attempt to connect to the device
    if (!m_controller->open())
    {
        QString device = m_controller->device();
        Logger::err(tr("failed to open device \"%1\"\n").arg(device));
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
void ApplicationFrame::setEnabledButtons(int buttons)
{
    btnLanding->setEnabled(buttons & BUTTON_LANDING);
    btnTakeoff->setEnabled(buttons & BUTTON_TAKEOFF);
    btnOverride->setEnabled(buttons & BUTTON_OVERRIDE);
    btnKillswitch->setEnabled(buttons & BUTTON_KILLSWITCH);
    btnColorTrack->setEnabled(buttons & BUTTON_TRACKING);
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
    float yaw, float pitch, float roll, int alt, int rssi, int batt, int aux, int cpu)
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
    
    if(cpu < 0)
    {
        cpu = 0;
    }
    cpuStatusBar->setValue(cpu);    
    cpuStatusBar->setFormat(QString("%p%"));    

    // add new data to their respective plots
    m_graphs[AXIS_X]->addDataPoint(time, yaw + 180.0f, 0.0f);
    m_graphs[AXIS_Y]->addDataPoint(time, pitch + 180.0f, 0.0f);
    m_graphs[AXIS_Z]->addDataPoint(time, roll + 180.0f, 0.0f);
    m_graphs[CONNECTION]->addDataPoint(time, rssi, 0.0f);
    m_graphs[BATTERY]->addDataPoint(time, batt, 0.0f);
    m_graphs[AUXILIARY]->addDataPoint(time, aux, 0.0f);
    m_graphs[ELEVATION]->addDataPoint(time, alt, 0.0f);
    m_graphs[CPU]->addDataPoint(time, cpu, 0.0f);

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
    case STATE_DISCONNECTED:
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
void ApplicationFrame::onFileReconnectTriggered()
{
    if (!m_controller)
    {
        Logger::err("no device to reconnect to\n");
        return;
    }

    m_controller->close();
    openDevice();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onFileDisconnectTriggered()
{
    if (m_controller)
    {
        Logger::info(tr("disconnecting \"%1\"\n").arg(m_controller->device()));
        m_controller->close();
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
    connectGamepad();

    return openDevice();
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
    bool track_en, btn_track_en;

    if (m_controller)
    {
        track     = m_controller->currentTrackSettings();
        track_en  = m_controller->getTrackEnabled();        
    }
    else 
        track_en = false;

    btn_track_en = btnColorTrack->isEnabled();

    SettingsDialog sd(this, track_en, btn_track_en, track, filename, m_bufsize/1024);
    if (m_controller)
    {
        // allow settings dialog to communicate data to device controller
        connect(&sd, SIGNAL(trackSettingsChanged(int, int, int, int, int, int, int)),
                m_controller,
                SLOT(updateTrackSettings(int, int, int, int, int, int, int)));

        connect(&sd, SIGNAL(updateTrackEnabled(bool)), m_controller, 
                SLOT(onUpdateTrackEnabled(bool)));

        connect(&sd, SIGNAL(deviceControlChanged(int, int)),
                m_controller, SLOT(updateDeviceControl(int, int)));

        connect(&sd, SIGNAL(trimSettingsChanged(int, int)),
                m_controller, SLOT(updateTrimSettings(int, int)));

        connect(&sd, SIGNAL(filterSettingsChanged(int, int)),
                m_controller, SLOT(updateFilterSettings(int, int)));

        connect(&sd, SIGNAL(pidSettingsChanged(int, float)), 
            m_controller, SLOT(updatePIDSettings(int, float)));

        // populate the video device control pane
        connect(m_controller, SIGNAL(deviceControlUpdated(const QString &,
                        const QString &, int, int, int, int, int, int)),
                &sd, SLOT(onDeviceControlUpdated(const QString &,
                        const QString &, int, int, int, int, int, int)));

        connect(m_controller, SIGNAL(deviceMenuUpdated(const QString&,int,int)),
                &sd, SLOT(onDeviceMenuUpdated(const QString &, int, int)));

        m_controller->requestDeviceControls();

        // initialize the trim settings sliders
        connect(m_controller, SIGNAL(trimSettingsUpdated(int, int, int, int)),
                &sd, SLOT(onTrimSettingsUpdated(int, int, int, int)));

        m_controller->requestTrimSettings();

        // initialize the filter settings sliders
        connect(m_controller, SIGNAL(filterSettingsUpdated(int, int, int, int)),
                &sd, SLOT(onFilterSettingsUpdated(int, int, int, int)));
                
        // initialize the color tracking values
        connect(m_controller, SIGNAL(colorValuesUpdate(TrackSettings)),
                &sd, SLOT(onColorValuesUpdated(TrackSettings)));

        m_controller->requestFilterSettings();
        if(!m_controller->requestColors()){
            Logger::warn("SETTINGS: Failed To send requestColors");
        }
    }
    
    connect(&sd, SIGNAL(logSettingsChanged(const QString &, int)), this, 
                SLOT(onUpdateLogFile(const QString &, int)));
        
    connect(&sd, SIGNAL(updateTrackEnabled(bool)), this, 
                SLOT(onUpdateTrackEnabled(bool)));
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
void ApplicationFrame::onColorTrackingClicked()
{
    bool track_en;

    // get current track settings
    track_en = !m_controller->getTrackEnabled();

    // update button label
    if (track_en)
        btnColorTrack->setText("Disable Tracking");
    else
        btnColorTrack->setText("Enable Tracking");

    m_controller->onUpdateTrackEnabled(track_en);
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onUpdateTrackEnabled(bool track_en)
{
    // update button label
    if (track_en)
        btnColorTrack->setText("Disable Tracking");
    else
        btnColorTrack->setText("Enable Tracking");
}

// ----------------------------------------------------------------------------
void ApplicationFrame::onShowXFChanged(bool flag)
{
    m_graphs[AXIS_X]->togglePrimaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowXUFChanged(bool flag)
{
    m_graphs[AXIS_X]->toggleSecondaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowYFChanged(bool flag)
{
    m_graphs[AXIS_Y]->togglePrimaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowYUFChanged(bool flag)
{
    m_graphs[AXIS_Y]->toggleSecondaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowZFChanged(bool flag)
{
    m_graphs[AXIS_Z]->togglePrimaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowZUFChanged(bool flag)
{
    m_graphs[AXIS_Z]->toggleSecondaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowAuxChanged(bool flag)
{
    m_graphs[AUXILIARY]->togglePrimaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowBattChanged(bool flag)
{
    m_graphs[BATTERY]->togglePrimaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowConnChanged(bool flag)
{
    m_graphs[CONNECTION]->togglePrimaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowElevChanged(bool flag)
{
    m_graphs[ELEVATION]->togglePrimaryData(flag);
    onGraphsChanged();
}

// -----------------------------------------------------------------------------
void ApplicationFrame::onShowCpuChanged(bool flag)
{
    m_graphs[CPU]->togglePrimaryData(flag);
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

    m_lblNoAxes->setVisible(visible);
}

