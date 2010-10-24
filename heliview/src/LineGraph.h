// -----------------------------------------------------------------------------
// File:    LineGraph.h
// Authors: Garrett Smith
// Created: 08-23-2010
//
// Plot object that wraps QWT class.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_LINEGRAPH__H_
#define _HELIVIEW_LINEGRAPH__H_

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

enum GraphType
{
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    CONNECTION,
    BATTERY,
    ELEVATION,
    AUXILIARY,
    TRACKING,
    CPU,
    AXIS_COUNT
};

class LineGraph : public QObject
{
    Q_OBJECT

public:
    LineGraph(QWidget *parent, const QString &graphLabel, double scale_max, 
            double dependentStep = 0.0f);
    virtual ~LineGraph();

    QwtPlot *getPlot() const;
    bool isVisible() const;
    void addDataPoint(float t, float value, float secondValue);

    void togglePrimaryData(bool flag);
    void toggleSecondaryData(bool flag);
    void toggleTertiaryData(bool flag);

protected:
    void toggleGeneric(QwtPlotCurve *curve, bool *oldflag, bool newflag);

protected:
    QwtPlot *m_plot;
    QwtPlotCurve *m_curvePrimary, *m_curveSecondary, *m_curveTertiary;
    QPolygonF m_polyPrimary, m_polySecondary, m_polyTertiary;
    bool m_primary, m_secondary, m_tertiary;
    int m_numVisible;
    float m_time;
    int m_ticker;
};

#endif // _HELIVIEW_LINEGRAPH__H_

