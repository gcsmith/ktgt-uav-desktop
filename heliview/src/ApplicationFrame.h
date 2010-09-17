// -----------------------------------------------------------------------------
// File:    ApplicationFrame.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_APPLICATIONFRAME__H_
#define _HELIVIEW_APPLICATIONFRAME__H_

#include <QWidget>
#include <QFile>
#include "ui_ApplicationFrame.h"
#include "DeviceController.h"
#include "LineGraph.h"
#include "VirtualView.h"
#include "VideoView.h"
#include "Gamepad.h"

class ApplicationFrame : public QMainWindow, public Ui::ApplicationFrame
{
    Q_OBJECT

public:
    ApplicationFrame(DeviceController *controller, bool disable_virtual_view);
    virtual ~ApplicationFrame();

    void openLogFile(const QString &logfile);
    void closeLogFile();
    void enableLogging(bool enable);

public slots:
    void onTelemetryReady(float yaw, float pitch, float roll,
                          int alt, int rssi, int batt);
    void onConnectionStatusChanged(const QString &text, bool status);
    void onInputReady(GamepadEvent event, int index, float value);

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
    void setupSensorView();
    void setupVirtualView();
    void setupStatusBar();
    void setupDeviceController();
    void setupGamepad();

    LineGraph        *m_graphs[AXIS_COUNT];
    VirtualView      *m_virtual;
    VideoView        *m_video;
    QFile            *m_file;
    QLabel           *m_connStat;
    QTextStream      *m_log;
    bool              m_logging;
    DeviceController *m_controller;
    Gamepad          *m_gamepad;
};

#endif // _HELIVIEW_APPLICATIONFRAME__H_

