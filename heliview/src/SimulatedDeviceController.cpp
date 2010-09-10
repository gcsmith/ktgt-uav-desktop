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
#include "SimulatedDeviceController.h"

using namespace std;

// -----------------------------------------------------------------------------
SimulatedDeviceController::SimulatedDeviceController()
: m_time(0.0f), m_yaw(0.0f), m_pitch(0.0f), m_roll(0.0f)
{
}

// -----------------------------------------------------------------------------
SimulatedDeviceController::~SimulatedDeviceController()
{
}

// -----------------------------------------------------------------------------
bool SimulatedDeviceController::open(const QString &device)
{
    qDebug() << "creating simulated device" << device;

    // attach the specified simulation callback
    QTimer *timer = new QTimer(this);
    if (device == "noise")
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
    m_time += 0.02; // 20 ms clock
    m_yaw += 0.25;
    m_pitch = 7.0f * sin(m_time);
    m_roll = 5.0f * sin(m_time * 2.0f);
    emit telemetryReady(m_yaw, m_pitch, m_roll);
}

// -----------------------------------------------------------------------------
void SimulatedDeviceController::onSimulateNoiseTick()
{
}

