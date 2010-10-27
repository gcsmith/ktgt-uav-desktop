// -----------------------------------------------------------------------------
// File:    SimulatedDeviceController.cpp
// Authors: Garrett Smith
// Created: 08-24-2010
//
// Simulated device interface implementation.
// -----------------------------------------------------------------------------

#include <QTimer>
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
: m_device(device), m_time(0.0f), m_yaw(0), m_pitch(0), m_roll(0), m_alt(0),
  m_manual(false), m_track(QColor(0, 0, 0), 15, 15, 10, 12)
{
}

// -----------------------------------------------------------------------------
SimulatedDeviceController::~SimulatedDeviceController()
{
}

// -----------------------------------------------------------------------------
bool SimulatedDeviceController::open()
{
    Logger::info(QString("SimDC: creating simulated device\n"));

    // attach the specified simulation callback
    QTimer *timer = new QTimer(this);
    if (m_device == "noise")
        connect(timer, SIGNAL(timeout()), this, SLOT(onSimulateNoiseTick()));
    else
        connect(timer, SIGNAL(timeout()), this, SLOT(onSimulateTick()));
    timer->start(20);

    emit stateChanged(STATE_AUTONOMOUS);
    emit connectionStatusChanged(QString("Simulated connection opened"), true);
    return true;
}

// -----------------------------------------------------------------------------
void SimulatedDeviceController::close()
{
    emit connectionStatusChanged(QString("Connection closed"), false);
}

// -----------------------------------------------------------------------------
bool SimulatedDeviceController::requestDeviceControls() const
{
    emit deviceControlUpdated("Example Slider 1", "int", 0, 0, 100, 1, 0, 0);
    emit deviceControlUpdated("Example Slider 2", "int", 1, -500, 500, 10, 250, 250);
    emit deviceControlUpdated("Example Slider 3", "int", 2, 0, 10, 1, 0, 0);
    emit deviceControlUpdated("Example Slider 4", "int", 3, 0, 999999, 1, 0, 0);
    emit deviceControlUpdated("Example Menu 1", "menu", 4, 0, 2, 1, 0, 0);
    emit deviceControlUpdated("Example Menu 2", "menu", 5, 0, 2, 1, 0, 0);
    emit deviceControlUpdated("Example Boolean 1", "bool", 6, 0, 1, 1, 0, 0);
    emit deviceControlUpdated("Example Boolean 2", "bool", 7, 0, 1, 1, 0, 0);
    emit deviceControlUpdated("Example Boolean 3", "bool", 8, 0, 1, 1, 0, 0);

    emit deviceMenuUpdated("Menu 1 Item 1", 4, 0);
    emit deviceMenuUpdated("Menu 1 Item 2", 4, 1);
    emit deviceMenuUpdated("Menu 1 Item 3", 4, 2);

    emit deviceMenuUpdated("Menu 2 Item 1", 5, 0);
    emit deviceMenuUpdated("Menu 2 Item 2", 5, 1);
    emit deviceMenuUpdated("Menu 2 Item 3", 5, 2);

    return true;
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
        m_alt += m_dthrottle;
    }
    else
    {
        m_yaw += 0.25;
        m_pitch = 7.0f * sin(m_time);
        m_roll = 5.0f * sin(m_time * 2.0f);
        m_alt = 21.0f + 21.0f * sin(m_time);
    }
    emit telemetryReady(m_yaw, m_pitch, m_roll, m_alt, 200, 100, 1000, 0);
    Logger::telemetry(tr("%1 %2 %3 %4 %5 %6 %7 %8\n").arg(m_yaw).arg(m_pitch)
                    .arg(m_roll).arg(m_alt).arg(200).arg(100).arg(1000).arg(0));
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
void SimulatedDeviceController::updateTrackSettings(
        int r, int g, int b, int ht, int st, int ft, int fps)
{
    m_track.color = QColor(r, g, b);
    if (ht > 0) m_track.ht = ht;
    if (st > 0) m_track.st = st;
    if (ft > 0) m_track.ft = ft;
    if (fps >= 0) m_track.fps = fps;
}

