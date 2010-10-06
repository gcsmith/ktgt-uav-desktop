#ifndef _THROTTLE_THREAD__H_
#define _THROTTLE_THREAD__H_

#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include "NetworkDeviceController.h"

class ThrottleThread : public QThread
{
    Q_OBJECT

public:
    ThrottleThread(NetworkDeviceController *ctlr, QTcpSocket *sock, int vcm_axes);
    virtual ~ThrottleThread();

public slots:
    void onPollThrottleValue();
    void onUpdateThrottleValue(float val);
    void onExitRun();

signals:
    void sendThrottleEvent(float value);

protected:
    void run();

    NetworkDeviceController  *ndc;
    QTcpSocket               *qsock;
    QTimer                   *qtimer_poll_value;
    int                       alive;
    float                     value;
    float                     prev_value;
    int                       send_event;
};

#endif
