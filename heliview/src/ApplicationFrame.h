// -----------------------------------------------------------------------------
// File:    ApplicationFrame.h
// Authors: Garrett Smith, Kevin Macksamie, Tyler Thierolf, Timothy Miller
// Created: 08-23-2010
//
// Wraps the main window UI script and provides the application view.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_APPLICATIONFRAME__H_
#define _HELIVIEW_APPLICATIONFRAME__H_

#include <QWidget>
#include <QFile>
#include "ui_ApplicationFrame.h"
#include "ControllerView.h"
#include "DeviceController.h"
#include "LineGraph.h"
#include "VirtualView.h"
#include "VideoView.h"
#include "Gamepad.h"

#define BUTTON_LANDING    0x01
#define BUTTON_TAKEOFF    0x02
#define BUTTON_OVERRIDE   0x04
#define BUTTON_KILLSWITCH 0x08
#define BUTTON_TRACKING   0x10
#define BUTTON_NONE       0x00
#define BUTTON_ALL        0xFF

using namespace std;

class ApplicationFrame: public QMainWindow, protected Ui::ApplicationFrame
{
    Q_OBJECT

public:
    ApplicationFrame(bool noVirtualView);
    virtual ~ApplicationFrame();

    void openLogFile(const QString &logfile, const QString &tlogfile);
    void closeLogFile();
    bool enableLogging(bool enable, const QString &verbosity);

signals:
    void updateTrackControlEnable(int track_en);
    
public slots:
    bool connectTo(const QString &source, const QString &device);
    void onUpdateLogFile(const QString &file, const QString &tfile, int bufsize);
    void onUpdateLog(int type, const QString &msg);
    void onConnectionStatusChanged(const QString &text, bool status);

    void onTelemetryReady(float yaw, float pitch, float roll, float alt,
            int rssi, int batt, int aux, int cpu);
    void onControlStateChanged(int state);
    void onFlightStateChanged(int state);
    void onUpdateTrackControlEnable(int track_en);

    // menu action triggered event callbacks
    void onFileConnectTriggered();
    void onFileReconnectTriggered();
    void onFileDisconnectTriggered();
    void onFileExitTriggered();
    void onEditSettingsTriggered();
    void onHelpAboutTriggered();
    void onFileSaveFrameTriggered();
    void onFileSaveLogTriggered();

    // control panel button click event callbacks
    void onTakeoffClicked();
    void onLandingClicked();
    void onManualOverrideClicked();
    void onKillswitchClicked();
    void onColorTrackingClicked();

    // sensor pane checkbox event callbacks
    void onShowXFChanged(bool flag);
    void onShowXUFChanged(bool flag);
    void onShowYFChanged(bool flag);
    void onShowYUFChanged(bool flag);
    void onShowZFChanged(bool flag);
    void onShowZUFChanged(bool flag);
    void onShowAuxChanged(bool flag);
    void onShowBattChanged(bool flag);
    void onShowConnChanged(bool flag);
    void onShowElevChanged(bool flag);
    void onShowCpuChanged(bool flag);

    void onTabChanged(int index);
    void onGraphsChanged();

protected:
    void setupCameraView();
    void setupControllerPane();
    void setupSensorView();
    void setupVirtualView();
    void setupStatusBar();
    void connectDeviceController();
    void connectGamepad();
    void setupSignalsSlots();

    bool openDevice();
    void setEnabledButtons(int buttons);
    void writeToLog(const QString &plain, const QString &rich, int log);

    LineGraph        *m_graphs[AXIS_COUNT];
    VirtualView      *m_virtual;
    VideoView        *m_video;
    QFile            *m_file;
    QFile            *m_tele_file;
    QLabel           *m_connStat;
    QTextStream      *m_log;
    QByteArray       *m_logbuffer;
    QTextStream      *m_tele_log;
    QByteArray       *m_tele_logbuffer;
    int               m_bufsize;
    bool              m_logging;
    DeviceController *m_controller;
    Gamepad          *m_gamepad;
    ControllerView   *m_ctlview;
    int               m_verbosity;
    QLabel           *m_lblNoAxes;
};

#endif // _HELIVIEW_APPLICATIONFRAME__H_

