// -----------------------------------------------------------------------------
// File:    ApplicationFrame.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_APPLICATIONFRAME__H_
#define _HELIVIEW_APPLICATIONFRAME__H_

#include <QWidget>
#include <QTcpSocket>
#include <qextserialport.h>
#include "ui_ApplicationFrame.h"
#include "DeviceController.h"
#include "CVWebcamView.h"
#include "LineGraph.h"
#include "VirtualView.h"

class ApplicationFrame : public QMainWindow, public Ui::ApplicationFrame
{
    Q_OBJECT

public:
    ApplicationFrame(DeviceController *controller);
    virtual ~ApplicationFrame();

    bool openSerialCommunication(const QString &device);
    void attachSimulatedSource(bool noise);
    void openLogFile(const QString &logfile);
    void closeLogFile();
    void enableLogging(bool enable);

public slots:
    void onSerialDataReady();
    void onSimulateTick();
    void onTelemetryTick();
    void updateLine(const char *message);

    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);

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
    CVWebcamView     *m_camera;
    VirtualView      *m_virtual;
    QextSerialPort   *m_serial;
    char              m_buffer[64];
    int               m_offset;
    double            m_index;
    bool              m_noise;
    float             m_ro[9];
    QFile            *m_file;
    QTextStream      *m_log;
    QTcpSocket       *m_sock;
    bool              m_logging;
    DeviceController *m_controller;

    void setupCameraView();
    void setupSensorView();
    void setupVirtualView();
};

#endif // _HELIVIEW_APPLICATIONFRAME__H_

