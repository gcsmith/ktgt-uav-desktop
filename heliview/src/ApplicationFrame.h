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
    void onTelemetryReady(float yaw, float pitch, float roll);

    void onShowXFChanged(bool flag);
    void onShowXUFChanged(bool flag);
    void onShowYFChanged(bool flag);
    void onShowYUFChanged(bool flag);
    void onShowZFChanged(bool flag);
    void onShowZUFChanged(bool flag);

    void onTabChanged(int index);
    void onGraphsChanged();

protected:
    LineGraph        *m_graphs[AXIS_COUNT];
    VirtualView      *m_virtual;
    VideoView        *m_video;
    QFile            *m_file;
    QTextStream      *m_log;
    bool              m_logging;
    DeviceController *m_controller;

    void setupCameraView();
    void setupSensorView();
    void setupVirtualView();
};

#endif // _HELIVIEW_APPLICATIONFRAME__H_

