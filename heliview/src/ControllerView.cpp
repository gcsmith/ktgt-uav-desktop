// -----------------------------------------------------------------------------
// File:    ControllerView.cpp
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#include <QPainter>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "ControllerView.h"
#include "uav_protocol.h"

// macro inverts the bit (y) in the number (x)
#define FLIP_A_BIT(x,y) ((((x) & (y)) ^ (y)) | ((x) & ~(y)))

// -----------------------------------------------------------------------------
ControllerView::ControllerView(QWidget *parent)
: QWidget(parent), axes_flags(0), si_on(0), m_joystick_timer(NULL)
{
    m_joystick_timer = new QTimer(this);
    connect(m_joystick_timer, SIGNAL(timeout()), this, SLOT(onJoystickTick()));
    m_joystick_timer->start(50);

    m_ctl.jl_x = 0.0f;
    m_ctl.jl_y = 0.0f;
    m_ctl.jr_x = 0.0f;
    m_ctl.jr_y = 0.0f;

    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);

    repaint();
}

// -----------------------------------------------------------------------------
ControllerView::~ControllerView()
{
    delete m_joystick_timer;
}

// -----------------------------------------------------------------------------
void ControllerView::onJoystickTick()
{
    repaint();
}

// -----------------------------------------------------------------------------
void ControllerView::onInputReady(GamepadEvent event, int index, float value)
{
    if ((GP_EVENT_BUTTON == event) && (12 == index) && (value > 0.0))
    {
        si_on = !si_on;
        if (si_on)
            axes_flags = VCM_AXIS_ALL;
        else
            axes_flags = 0;
    }
    else if ((GP_EVENT_BUTTON == event) && (index >= 4) && (index <= 7) && 
            (value > 0.0) && si_on)
    {
        switch (index)
        {
            // A
            case 4:
                axes_flags = FLIP_A_BIT(axes_flags, VCM_AXIS_ALT);
                break;
            
            // B
            case 5:
                axes_flags = FLIP_A_BIT(axes_flags, VCM_AXIS_ROLL);
                break;

            // X
            case 6:
                axes_flags = FLIP_A_BIT(axes_flags, VCM_AXIS_YAW);
                break;

            // Y
            case 7:
                axes_flags = FLIP_A_BIT(axes_flags, VCM_AXIS_PITCH);
                break;

            default:
                fprintf(stderr, "ControllerView: unknown controller button\n");
        }
    }
    else if ((GP_EVENT_AXIS == event) && (index >= 0) && (index <= 3))
    {
        switch (index)
        {
            // yaw
            case 0:
                m_ctl.jl_x = value;
                break;

            // altitude
            case 1:
                m_ctl.jl_y = value;
                break;

            // roll
            case 2:
                m_ctl.jr_x = value;
                break;

            // pitch
            case 3:
                m_ctl.jr_y = value;
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
    if (si_on)
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

    if (si_on)
    {
        // offset the nub positions if mixed mode controls are enabled
        float l_len = sqrt(m_ctl.jl_x * m_ctl.jl_x + m_ctl.jl_y * m_ctl.jl_y);
        float r_len = sqrt(m_ctl.jr_x * m_ctl.jr_x + m_ctl.jr_y * m_ctl.jr_y);

        if (axes_flags & VCM_AXIS_YAW)
            n1_dx = n_2 * ((l_len > 1.0f) ? m_ctl.jl_x / l_len : m_ctl.jl_x);
        if (axes_flags & VCM_AXIS_ALT)
            n1_dy = n_2 * ((l_len > 1.0f) ? m_ctl.jl_y / l_len : m_ctl.jl_y);
        if (axes_flags & VCM_AXIS_ROLL)
            n2_dx = n_2 * ((r_len > 1.0f) ? m_ctl.jr_x / r_len : m_ctl.jr_x);
        if (axes_flags & VCM_AXIS_PITCH)
            n2_dy = n_2 * ((r_len > 1.0f) ? m_ctl.jr_y / r_len : m_ctl.jr_y);
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
        axes_flags & VCM_AXIS_ALT,
        axes_flags & VCM_AXIS_YAW,
        axes_flags & VCM_AXIS_PITCH,
        axes_flags & VCM_AXIS_ROLL
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

