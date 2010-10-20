// -----------------------------------------------------------------------------
// File:    ControllerView.cpp
// Authors: Kevin Macksamie, Garrett Smith
// Created: 09-25-2010
//
// Widget that displays the current gamepad positions (buttons & analog sticks)
// -----------------------------------------------------------------------------

#include <QPainter>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "ControllerView.h"
#include "uav_protocol.h"
#include "DeviceController.h"
#include "Utility.h"

// -----------------------------------------------------------------------------
ControllerView::ControllerView(QWidget *parent)
: QWidget(parent), m_timer(NULL), m_enabled(false), m_stale(false),
  m_lx(0.0f), m_ly(0.0f), m_rx(0.0f), m_ry(0.0f), m_axes(0)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRepaintTick()));
    m_timer->start(50);

    repaint();
}

// -----------------------------------------------------------------------------
ControllerView::~ControllerView()
{
    SafeDelete(m_timer);
}

// -----------------------------------------------------------------------------
void ControllerView::setEnabled(bool enabled)
{
    m_stale = (enabled != m_enabled);
    m_enabled = enabled;
    m_axes = m_enabled ? AXIS_ALL : 0;
}

// -----------------------------------------------------------------------------
bool ControllerView::enabled()
{
    return m_enabled;
}

// -----------------------------------------------------------------------------
void ControllerView::setAxes(int axes)
{
    m_axes = axes;
}

// -----------------------------------------------------------------------------
int ControllerView::axes()
{
    return m_axes;
}

// -----------------------------------------------------------------------------
void ControllerView::onRepaintTick()
{
    if (m_stale)
    {
        repaint();
        m_stale = false;
    }
}

// -----------------------------------------------------------------------------
void ControllerView::onInputReady(GamepadEvent event, int index, float value)
{
    m_stale = true;
    if ((GP_EVENT_AXIS == event) && (index >= 0) && (index <= 3))
    {
        switch (index)
        {
        case 0: // yaw
            m_lx = value;
            break;
        case 1: // altitude
            m_ly = value;
            break;
        case 2: // roll
            m_rx = value;
            break;
        case 3: // pitch
            m_ry = value;
            break;
        default:
            fprintf(stderr, "ControllerView: unknown controller axis\n");
            break;
        }
    }
}

// -----------------------------------------------------------------------------
void ControllerView::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // render the background color
    int xr = width(), yr = height();
    if (m_enabled)
    {
        painter.setBrush(QBrush(QColor(0,201,87,100)));
        painter.drawRect(0, 0, xr, yr);
    }

    float j_w = 0.9f * xr / 2.0f, j_h = 0.9f * (2.0f / 3.0f) * yr;
    float j_m = std::min(j_w, j_h); // maintain aspect ratio
    float j1_x = xr / 4.0f, j1_y = yr - yr / 3.0;

    // render the out circles for the two analog sticks
    painter.setBrush(QBrush(QColor(133, 133, 133)));
    painter.drawEllipse(j1_x - j_m / 2, j1_y - j_m / 2, j_m, j_m);
    painter.drawEllipse(xr - j1_x - j_m / 2, j1_y - j_m / 2, j_m, j_m);

    float n_w = 0.9f * 0.5f * xr / 2.0f, n_h = 0.9f * 0.5f * (2 / 3.0f) * yr;
    float n_m = std::min(n_w, n_h), n_2 = n_m / 2; // maintain aspect ratio
    float n1_dx = 0.0f, n1_dy = 0.0f;
    float n2_dx = 0.0f, n2_dy = 0.0f;

    if (m_enabled)
    {
        // offset the nub positions if mixed mode controls are enabled
        float l_len = sqrt(m_lx * m_lx + m_ly * m_ly);
        float r_len = sqrt(m_rx * m_rx + m_ry * m_ry);

        if (m_axes & AXIS_YAW)
            n1_dx = n_2 * ((l_len > 1.0f) ? m_lx / l_len : m_lx);
        if (m_axes & AXIS_ALT)
            n1_dy = n_2 * ((l_len > 1.0f) ? m_ly / l_len : m_ly);
        if (m_axes & AXIS_ROLL)
            n2_dx = n_2 * ((r_len > 1.0f) ? m_rx / r_len : m_rx);
        if (m_axes & AXIS_PITCH)
            n2_dy = n_2 * ((r_len > 1.0f) ? m_ry / r_len : m_ry);
    }

    // render the inner circle for each analog stick's nub
    painter.setBrush(QBrush(QColor(238, 232, 170, 255)));
    painter.drawEllipse(j1_x - n_2 + n1_dx, j1_y - n_2 + n1_dy, n_m, n_m);
    painter.drawEllipse(xr - j1_x - n_2 + n2_dx, j1_y - n_2 + n2_dy, n_m, n_m);

    float led_dx = xr / 5.0f, led_y = yr / 6.0f;
    float led_w = 0.5f * xr / 4.0f, led_h = 0.5f * yr / 3.0f;
    float led_m = std::min(led_w, led_h);
    static const char *labels[] = { "alt", "yaw", "pitch", "roll" };
    int led_en[] = {
        m_axes & AXIS_ALT,
        m_axes & AXIS_YAW,
        m_axes & AXIS_PITCH,
        m_axes & AXIS_ROLL
    };

    for (int i = 0; i < 4; ++i) {
        float led_x = (i + 1) * led_dx;
        QRadialGradient gradient(led_x, led_y - led_m/2, led_m,
                led_x - led_m / 2, led_y - led_m / 2);
        gradient.setColorAt(0.2, Qt::white);
        gradient.setColorAt(0.8, led_en[i] ? Qt::green : Qt::red);
        gradient.setColorAt(1, Qt::black);

        painter.setBrush(gradient);
        painter.drawEllipse(led_x - led_m / 2, led_y - led_m / 1.5f, led_m, led_m);
        painter.drawText(QRectF((i + 1) * led_dx - xr / 8, led_y + led_m / 2,
                xr / 4, yr / 10), Qt::AlignCenter, labels[i]);
    }
}

// -----------------------------------------------------------------------------
void ControllerView::resizeEvent(QResizeEvent *e)
{
    repaint();
}

