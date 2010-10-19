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

class ApplicationFrame: public QMainWindow, protected Ui::ApplicationFrame
{
    Q_OBJECT

public:
    ApplicationFrame(bool noVirtualView);
    virtual ~ApplicationFrame();

    void openLogFile(const QString &logfile);
    void closeLogFile();
    bool enableLogging(bool enable, const QString &verbosity);

public slots:
    bool connectTo(const QString &source, const QString &device);
    void onUpdateLogFile(const QString &file, int bufsize);
    void onUpdateLog(int type, const QString &msg);
    void onConnectionStatusChanged(const QString &text, bool status);

    void onTelemetryReady(float yaw, float pitch, float roll,
                          int alt, int rssi, int batt, int aux);
    void onStateChanged(int state);

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
    void connectDeviceController();
    void connectGamepad();
    void setupSignalsSlots();

    void setEnabledButtons(int buttons);
    void writeToLog(const QString &plain, const QString &rich);

    LineGraph        *m_graphs[AXIS_COUNT];
    VirtualView      *m_virtual;
    VideoView        *m_video;
    QFile            *m_file;
    QLabel           *m_connStat;
    QTextStream      *m_log;
    QByteArray       *m_logbuffer;
    int               m_bufsize;
    bool              m_logging;
    DeviceController *m_controller;
    Gamepad          *m_gamepad;
    ControllerView   *m_ctlview;
    int               m_verbosity;
    int               m_r, m_g, m_b;
    int               m_ht, m_st;
};

#endif // _HELIVIEW_APPLICATIONFRAME__H_

