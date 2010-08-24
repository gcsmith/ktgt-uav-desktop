// -----------------------------------------------------------------------------
// File:    LineGraph.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_LINEGRAPH__H_
#define _HELIVIEW_LINEGRAPH__H_

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

enum GraphAxes
{
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_COUNT
};

class LineGraph : public QObject
{
    Q_OBJECT

public:
    LineGraph(QWidget *parent);
    virtual ~LineGraph();

    QwtPlot *getPlot() const;
    bool isVisible() const;
    void addDataPoint(float t, float acc, float vel);

    void toggleAcceleration(bool flag);
    void toggleRawVelocity(bool flag);
    void toggleFilteredVelocity(bool flag);

protected:
    void toggleGeneric(QwtPlotCurve *curve, bool *oldflag, bool newflag);

protected:
    QwtPlot *m_plot;
    QwtPlotCurve *m_curveAcc, *m_curveVel, *m_curveFVel;
    QPolygonF m_polyAcc, m_polyVel, m_polyFVel;
    bool m_acc, m_vel, m_fvel;
    int m_numVisible;
    float m_time;
    int m_ticker;
};

#endif // _HELIVIEW_LINEGRAPH__H_

