// -----------------------------------------------------------------------------
// File:    ControllerView.h
// Authors: Kevin Macksamie
// Created: 09-25-2010
//
// Widget that displays the current gamepad positions (buttons & analog sticks)
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

    void setEnabled(bool enabled);
    bool enabled();

    void setAxes(int axes);
    int axes();

public slots:
    void onRepaintTick();
    void onInputReady(GamepadEvent event, int index, float value);

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

    QTimer *m_timer;
    bool    m_enabled;
    bool    m_stale;
    float   m_lx, m_ly, m_rx, m_ry;
    char    m_axes;
};

#endif // _CONTROLLERVIEW__H_
