// -----------------------------------------------------------------------------
// File:    ControllerView.h
// Authors: Kevin Macksamie
// -----------------------------------------------------------------------------

#ifndef _CONTROLLERVIEW__H_
#define _CONTROLLERVIEW__H_

#include "Gamepad.h"
#include <QTimer>
#include <QWidget>

typedef struct ctlr_inputs
{
    float jl_x, jl_y, jr_x, jr_y;
} ctlr_inputs_t;

class ControllerView : public QWidget
{
    Q_OBJECT

public:
    ControllerView(QWidget *parent);
    virtual ~ControllerView();

public slots:
    void onJoystickTick();
    void onInputReady(GamepadEvent event, int index, float value);

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

    char           axes_flags;
    char           si_on;
    QTimer        *m_joystick_timer;
    ctlr_inputs_t  m_ctl;
};

#endif // _CONTROLLERVIEW__H_
