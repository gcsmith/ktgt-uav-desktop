// -----------------------------------------------------------------------------
// File:    SimulatedDeviceController.cpp
// Authors: Garrett Smith
//
// Simulated device interface implementation.
// -----------------------------------------------------------------------------

#include <QTimer>
#include <QDebug>
#include <iostream>
#include <cmath>
#include "Logger.h"
#include "Utility.h"
#include "SimulatedDeviceController.h"

using namespace std;

const char * SimulatedDeviceController::m_description = "Simulated description";
const bool SimulatedDeviceController::m_takesDevice = false;

// -----------------------------------------------------------------------------
SimulatedDeviceController::SimulatedDeviceController(const QString &device)
: m_device(device), m_time(0.0f), m_yaw(0.0f), m_pitch(0.0f), m_roll(0.0f),
  m_manual(false), m_track(QColor(0, 0, 0), 15, 15)
{
}

// -----------------------------------------------------------------------------
SimulatedDeviceController::~SimulatedDeviceController()
{
}

// -----------------------------------------------------------------------------
bool SimulatedDeviceController::open()
{
    qDebug() << "creating simulated device" << m_device;
    Logger::info(QString("SimDC: creating simulated device\n"));

    // attach the specified simulation callback
    QTimer *timer = new QTimer(this);
    if (m_device == "noise")
        connect(timer, SIGNAL(timeout()), this, SLOT(onSimulateNoiseTick()));
    else
        connect(timer, SIGNAL(timeout()), this, SLOT(onSimulateTick()));

    timer->start(20);
    return true;
}

// -----------------------------------------------------------------------------
void SimulatedDeviceController::close()
{
}

// -----------------------------------------------------------------------------
void SimulatedDeviceController::onSimulateTick()
{
    m_time += 0.02f; // 20 ms clock
    if (m_manual)
    {
        m_yaw += m_dyaw;
        m_pitch += m_dpitch;
        m_roll += m_droll;
        m_throttle += m_dthrottle;
    }
    else
    {
        m_yaw += 0.25;
        m_pitch = 7.0f * sin(m_time);
        m_roll = 5.0f * sin(m_time * 2.0f);
    }
    emit telemetryReady(m_yaw, m_pitch, m_roll, 0, 200, 100, 1500);
}

// -----------------------------------------------------------------------------
void SimulatedDeviceController::onSimulateNoiseTick()
{
}
 
// -----------------------------------------------------------------------------
void SimulatedDeviceController::onInputReady(
        GamepadEvent event, int index, float value)
{
    if ((GP_EVENT_AXIS == event) && m_manual)
    {
        switch (index)
        {
        case 0: m_dyaw = -value; break;
        case 1: m_dthrottle = value; break;
        case 2: m_droll = value; break;
        case 3: m_dpitch = -value; break;
        }
    }
    else if (GP_EVENT_BUTTON == event)
    {
        if ((12 == index) && (value > 0.0))
        {
            cerr << "toggle\n";
            m_manual = !m_manual;
        }
    }
}

// -----------------------------------------------------------------------------
void SimulatedDeviceController::onUpdateTrackColor(
        int r, int g, int b, int ht, int st)
{
    m_track.color = QColor(r, g, b);
    if (ht > 0) m_track.ht = ht;
    if (st > 0) m_track.st = st;
}

