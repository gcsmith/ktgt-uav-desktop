// -----------------------------------------------------------------------------
// File:    ApplicationFrame.h
// Authors: Garrett Smith
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
#define BUTTON_NONE       0x00
#define BUTTON_ALL        0xFF

using namespace std;

class ApplicationFrame : public QMainWindow, public Ui::ApplicationFrame
{
    Q_OBJECT

public:
    ApplicationFrame(DeviceController *controller, bool disable_virtual_view);
    virtual ~ApplicationFrame();

    void openLogFile(const QString &logfile);
    void closeLogFile();
    bool enableLogging(bool enable, const QString &verbosity);

public slots:
    void onTelemetryReady(float yaw, float pitch, float roll,
                          int alt, int rssi, int batt, int aux);
    void onConnectionStatusChanged(const QString &text, bool status);
    void onStateChanged(int state);
    void onUpdateLog(const QString &msg, int log_flags, int priority);

    // menu action triggered event callbacks
    void onFileConnectTriggered();
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

    // sensor pane checkbox event callbacks
    void onShowXFChanged(bool flag);
    void onShowXUFChanged(bool flag);
    void onShowYFChanged(bool flag);
    void onShowYUFChanged(bool flag);
    void onShowZFChanged(bool flag);
    void onShowZUFChanged(bool flag);

    void onTabChanged(int index);
    void onGraphsChanged();

protected:
    void setupCameraView();
    void setupControllerPane();
    void setupSensorView();
    void setupVirtualView();
    void setupStatusBar();
    void setupDeviceController();
    void setupGamepad();
    void setupSignalsSlots();

    void setEnabledButtons(int buttons);

    LineGraph        *m_graphs[AXIS_COUNT];
    VirtualView      *m_virtual;
    VideoView        *m_video;
    QFile            *m_file;
    QLabel           *m_connStat;
    QTextStream      *m_log;
    bool              m_logging;
    DeviceController *m_controller;
    Gamepad          *m_gamepad;
    ControllerView   *m_ctlview;
    int               m_verbosity;
    int               m_r, m_g, m_b;
    int               m_ht, m_st;
};

#endif // _HELIVIEW_APPLICATIONFRAME__H_

