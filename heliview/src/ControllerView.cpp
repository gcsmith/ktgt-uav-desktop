// -----------------------------------------------------------------------------
// File:    ControllerView.cpp
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#include <QPainter>
#include <math.h>
#include <stdio.h>
#include "ControllerView.h"

// -----------------------------------------------------------------------------
ControllerView::ControllerView(QWidget *parent)
: QWidget(parent)
{
    m_joystick_timer = new QTimer(this);
    connect(m_joystick_timer, SIGNAL(timeout()), this, SLOT(onJoystickTick()));
    m_joystick_timer->start(50);

    joystick.jl_x = 0.0f;
    joystick.jl_y = 0.0f;
    joystick.jr_x = 0.0f;
    joystick.jr_y = 0.0f;

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
    if (GP_EVENT_AXIS == event && index >= 0 && index <= 3)
    {
        switch (index)
        {
            // yaw
            case 0:
                joystick.jl_x = value;
                break;

            // altitude
            case 1:
                joystick.jl_y = value;
                break;

            // roll
            case 2:
                joystick.jr_x = value;
                break;

            // pitch
            case 3:
                joystick.jr_y = value;
                break;
            default:
                fprintf(stderr, "ControllerView: unknown controller signal\n");
                break;
        }
    }
}

// -----------------------------------------------------------------------------
void ControllerView::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0, 50, 155, 127), 3));
    
    // main circles

    // main circles coords
    int ml_x0 = 10, ml_y0 = 20, m_w = 74, m_h = 74;
    int mr_x0 = ml_x0 + m_w + 5, mr_y0 = ml_y0;

    painter.drawEllipse(ml_x0, ml_y0, m_w, m_h);
    painter.drawEllipse(mr_x0, mr_y0, m_w, m_h);

    // joysticks

    // left joystick coords
    int j_w = 34, j_h = 34;
    int jl_xc = ml_x0 + (m_w / 2), jl_yc = ml_y0 + (m_h / 2);
    int jl_x0 = jl_xc - (j_w / 2), jl_y0 = jl_yc - (j_h / 2);

    // right joystick coords
    int jr_xc = mr_x0 + (m_w / 2), jr_yc = mr_y0 + (m_h / 2);
    int jr_x0 = jr_xc - (j_w / 2), jr_y0 = jr_yc - (j_h / 2);
    
    // normalize left joystick's vector
    float length = sqrt(joystick.jl_x*joystick.jl_x + joystick.jl_y*joystick.jl_y);
    if (length > 1.0f) { joystick.jl_x /= length; joystick.jl_y /= length; }
    
    // draw left joystick
    painter.drawEllipse(jl_x0 + (joystick.jl_x * (jl_x0 - ml_x0)), 
            jl_y0 + (joystick.jl_y * (jl_y0 - ml_y0)), j_w, j_h);
    
    // normalize right joystick's vector
    length = sqrt(joystick.jr_x*joystick.jr_x + joystick.jr_y*joystick.jr_y);
    if (length > 1.0f) { joystick.jr_x /= length; joystick.jr_y /= length; }

    // draw right joystick
    painter.drawEllipse(QRectF(jr_x0 + (joystick.jr_x * (jr_x0 - mr_x0)), 
                jr_y0 + (joystick.jr_y * (jr_y0 - ml_y0)), j_w, j_h));
}

// -----------------------------------------------------------------------------
void ControllerView::resizeEvent(QResizeEvent *e)
{
    repaint();
}
