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

    char           m_axes;
    QTimer        *m_timer;
    ctlr_inputs_t  m_ctl;
    bool           m_enabled;
    bool           m_stale;
};

#endif // _CONTROLLERVIEW__H_
