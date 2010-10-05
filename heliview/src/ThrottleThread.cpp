#include "ThrottleThread.h"
#include "uav_protocol.h"

ThrottleThread::ThrottleThread(NetworkDeviceController *ctlr, QTcpSocket *sock, int vcm_axes)
: ndc(ctlr), qsock(NULL), qtimer_poll_value(NULL), vcm_flags(vcm_axes)
{
    qtimer_poll_value = new QTimer();
    connect(qtimer_poll_value, SIGNAL(timeout()), this, SLOT(onPollThrottleValue()));

    connect(ndc, SIGNAL(updateThrottleValue(float)), this, SLOT(onUpdateThrottleValue(float)));
    connect(ndc, SIGNAL(updateAxesToThrottleThread(int)), this, SLOT(onUpdateAxes(int)));
    connect(ndc, SIGNAL(exitThrottleThread()), this, SLOT(onExitRun()));

    alive = 0;
    send_event = 0;
    value = 0.0f;
    prev_value = 0.0f;
}

ThrottleThread::~ThrottleThread()
{
    ndc = NULL;
    qsock = NULL;
    delete qtimer_poll_value;
}

void ThrottleThread::onPollThrottleValue()
{
    if ((value < prev_value) && (prev_value > 0.0f) && (value > 0.0f))
    {
        // joystick is going from postive to zero
        // do nothing
    }
    else if ((value > prev_value) && (prev_value < 0.0f) && (value < 0.0f))
    {
        // joystick is going from negative to zero
        // do nothing
    }
    else if ((value >= 0.1f) || (value <= -0.1f))
    {
        // prev_value == value
        // the user hasn't moved the joystick since the last poll, so tell
        // the server to keep incrementing the throttle pwm
        send_event = 1;
        fprintf(stderr, "ThrottleThread: decided to send an event %f\n", value);
    }
}

void ThrottleThread::onUpdateThrottleValue(float val)
{
    prev_value = value;
    value = val;
    fprintf(stderr, "ThrottleThread: updating throttle value to %f\n", value);
}

void ThrottleThread::onUpdateAxes(int vcm_axes)
{
    fprintf(stderr, "ThrottleThread: updating axes\n");
    vcm_flags = vcm_axes;
}

void ThrottleThread::onExitRun()
{
    fprintf(stderr, "ThrottleThread: clearing alive flag\n");
    alive = 0;
}

void ThrottleThread::run()
{
    fprintf(stderr, "ThrottleThread: run() started\n");
    alive = 1;
    qtimer_poll_value->start(50);
    while(alive)
    {
        // do stuff
        if (send_event)
        {
            fprintf(stderr, "ThrottleThread: sending event\n");
            emit sendThrottleEvent(value);
            fprintf(stderr, "ThrottleThread: event sent\n");
            send_event = 0;
        }
    }
    qtimer_poll_value->stop();
    fprintf(stderr, "ThrottleThread: run() exiting\n");
}

