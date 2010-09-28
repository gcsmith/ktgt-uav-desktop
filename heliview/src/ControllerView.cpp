// -----------------------------------------------------------------------------
// File:    ControllerView.cpp
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#include <QPainter>
#include <math.h>
#include <stdio.h>
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
    if ((GP_EVENT_BUTTON == event) && (12 == index) && (value > 0.0))
    {
        si_on = !si_on;
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
                fprintf(stderr, "ControllerView: unknown controller axis\n");
                break;
        }
    }
}

// -----------------------------------------------------------------------------
void ControllerView::paintEvent(QPaintEvent *e)
{
    float length, mul_x, mul_y;
    QString status("Disabled");
    int wid = this->width(), hgt = this->height();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0, 50, 155, 127), 3));
    painter.setBrush(QBrush(Qt::gray));
    
    // main circles
    int m_w = wid * 0.428f, m_h = hgt * 0.556f;

    // main circles coords
    int ml_x0 = wid * 0.10f/*0.0578f*/, ml_y0 = hgt * 0.450f;
    int mr_x0 = wid * 0.55f/*ml_x0 + m_w + 5*/, mr_y0 = ml_y0;
    if (m_w != m_h)
    {
        int low = (m_w < m_h) ? m_w : m_h;
        m_w = low;
        m_h = low;
    }

    // draw main circles
    painter.drawEllipse(ml_x0, ml_y0, m_w, m_h);
    painter.drawEllipse(mr_x0, mr_y0, m_w, m_h);

    // joysticks
    painter.setBrush(QBrush(QColor(238, 232, 170, 255)));

    // left joystick coords
    int j_w = wid * 0.197f, j_h = hgt * 0.256f;
    if (j_w != j_h) 
    {
        int low = (j_w < j_h) ? j_w : j_h;
        j_w = low;
        j_h = low;
    }
    int jl_xc = ml_x0 + (m_w / 2), jl_yc = ml_y0 + (m_h / 2);
    int jl_x0 = jl_xc - (j_w / 2), jl_y0 = jl_yc - (j_h / 2);

    // right joystick coords
    int jr_xc = mr_x0 + (m_w / 2), jr_yc = mr_y0 + (m_h / 2);
    int jr_x0 = jr_xc - (j_w / 2), jr_y0 = jr_yc - (j_h / 2);
    
    // normalize left joystick's vector
    if (si_on)
    {
        length = sqrt(joystick.jl_x*joystick.jl_x + joystick.jl_y*joystick.jl_y);
        if (length > 1.0f) 
        { 
            joystick.jl_x /= length; 
            joystick.jl_y /= length; 
        }

        // check axes flags
        mul_x = (axes_flags & VCM_AXIS_YAW) ? joystick.jl_x : 0;
        mul_y = (axes_flags & VCM_AXIS_ALT) ? joystick.jl_y : 0;
    }
    else
    {
        mul_x = 0;
        mul_y = 0;
    }
    
    // draw left joystick
    painter.drawEllipse(jl_x0 + (mul_x * (jl_x0 - ml_x0)), 
            jl_y0 + (mul_y * (jl_y0 - ml_y0)), j_w, j_h);

    // normalize right joystick's vector
    if (si_on)
    {
        length = sqrt(joystick.jr_x*joystick.jr_x + joystick.jr_y*joystick.jr_y);
        if (length > 1.0f) 
        { 
            joystick.jr_x /= length; 
            joystick.jr_y /= length; 
        }

        // check axes flags
        mul_x = (axes_flags & VCM_AXIS_ROLL) ? joystick.jr_x : 0;
        mul_y = (axes_flags & VCM_AXIS_PITCH) ? joystick.jr_y : 0;
    }


    // draw right joystick
    painter.drawEllipse(QRectF(jr_x0 + (mul_x * (jr_x0 - mr_x0)), 
            jr_y0 + (mul_y * (jr_y0 - ml_y0)), j_w, j_h));
    
    // axes led circles
    wid = mr_x0 + m_w - ml_x0;
    int a_w = wid * 0.1983f, a_h = hgt * 0.128f;

    // axes led coords
    int thro_x0 = ml_x0;//wid * 0.2f;
    int yaw_x0 = wid * 0.3f + ml_x0;
    int pitch_x0 = wid * 0.6f + ml_x0;
    int roll_x0 = wid;

    if (a_w != a_h)
    {
        int low = (a_w < a_h) ? a_w : a_h;
        a_w = low;
        a_h = low;
    }
    
    wid = this->width(), hgt = this->height();

    painter.setBrush(QBrush(Qt::red));

    // draw throttle led
    if (axes_flags & VCM_AXIS_ALT)
        painter.setBrush(Qt::green);
    else
        painter.setBrush(Qt::red);
   
    painter.drawEllipse(thro_x0, a_h, a_w, a_h);

    // draw throttle text
    int txt_x0 = (thro_x0 + (a_w / 2)) - (wid * 0.3f / 2);
    int txt_y0 = 2 * a_h;
    int txt_w = wid * 0.3f;
    int txt_h = hgt * 0.113f;
    painter.drawText(QRectF(txt_x0, txt_y0, txt_w, txt_h), Qt::AlignCenter, "Throttle");
    
    // draw yaw led
    if (axes_flags & VCM_AXIS_YAW)
        painter.setBrush(Qt::green);
    else
        painter.setBrush(Qt::red);
    
    painter.drawEllipse(yaw_x0, a_h, a_w, a_h);
   
    // draw yaw text
    txt_x0 = (yaw_x0 + (a_w / 2)) - (wid * 0.3f / 2);
    painter.drawText(QRectF(txt_x0, txt_y0, txt_w, txt_h), Qt::AlignCenter, "Yaw");

    // draw pitch led
    if (axes_flags & VCM_AXIS_PITCH)
        painter.setBrush(Qt::green);
    else
        painter.setBrush(Qt::red);
    
    painter.drawEllipse(pitch_x0, a_h, a_w, a_h);
    
    // draw pitch text
    txt_x0 = (pitch_x0 + (a_w / 2)) - (wid * 0.3f / 2);
    painter.drawText(QRectF(txt_x0, txt_y0, txt_w, txt_h), Qt::AlignCenter, "Pitch");

    // draw roll led
    if (axes_flags & VCM_AXIS_ROLL)
        painter.setBrush(Qt::green);
    else
        painter.setBrush(Qt::red);
    
    painter.drawEllipse(roll_x0, a_h, a_w, a_h);

    // draw roll text
    txt_x0 = (roll_x0 + (a_w / 2)) - (wid * 0.3f / 2);
    painter.drawText(QRectF(txt_x0, txt_y0, txt_w, txt_h), Qt::AlignCenter, "Roll");
}

// -----------------------------------------------------------------------------
void ControllerView::resizeEvent(QResizeEvent *e)
{
    repaint();
}
