// -----------------------------------------------------------------------------
// File:    LineGraph.cpp
// Authors: Garrett Smith
// Created: 08-23-2010
//
// Plot object that wraps QWT class.
// -----------------------------------------------------------------------------

#include <cassert>
#include <qwt_plot_grid.h>
#include "LineGraph.h"

// -----------------------------------------------------------------------------
LineGraph::LineGraph(QWidget *parent)
: m_acc(false), m_vel(false), m_fvel(false), m_numVisible(0), m_ticker(0)
{
    assert(NULL != parent);

    // use the default application font, but adjust the point size
    QFont axisFont, titleFont;
    axisFont.setPointSize(8.0f);
    titleFont.setPointSize(10.0f);

    // create the graph object
    m_plot = new QwtPlot(parent);
    m_plot->setVisible(false);

    // set up X axis
    QwtText xTitle("Time (s)");
    xTitle.setFont(titleFont);

    m_plot->setAxisTitle(QwtPlot::xBottom, xTitle);
    m_plot->setAxisScale(QwtPlot::xBottom, 1.0f, 60.0f, 10.0f);
    m_plot->setAxisFont(QwtPlot::xBottom, axisFont);

    // set up Y axis
    QwtText yTitle("Sensor Value");
    yTitle.setFont(titleFont);

    m_plot->setAxisTitle(QwtPlot::yLeft, yTitle);
    m_plot->setAxisScale(QwtPlot::yLeft, 1.0f, 256.0f, 256.0f);
    m_plot->setAxisFont(QwtPlot::yLeft, axisFont);

    // overlay a grid on the plot
    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(m_plot);

    m_curveAcc = new QwtPlotCurve("Acceleration");
    m_curveAcc->setPen(QPen(Qt::red, 2));
    m_curveAcc->setRenderHint(QwtPlotItem::RenderAntialiased);

    m_curveVel = new QwtPlotCurve("Velocity");
    m_curveVel->setPen(QPen(Qt::blue, 2));
    m_curveVel->setRenderHint(QwtPlotItem::RenderAntialiased);
}

// -----------------------------------------------------------------------------
LineGraph::~LineGraph()
{
}

// -----------------------------------------------------------------------------
QwtPlot *LineGraph::getPlot() const
{
    return m_plot;
}

// -----------------------------------------------------------------------------
bool LineGraph::isVisible() const
{
    return m_numVisible > 0;
}

// -----------------------------------------------------------------------------
void LineGraph::addDataPoint(float t, float acc, float vel)
{
    m_time = t;

    // XXX this is awful, implement scrolling in a more reasonable fashion
    if (m_time >= 60.0)
    {
        m_time = 59.5;

        m_polyAcc.translate(-0.5, 0.0);
        m_polyAcc.pop_front();

        m_polyVel.translate(-0.5, 0.0);
        m_polyVel.pop_front();
    }

    m_polyAcc.push_back(QPointF(m_time, acc));
    m_curveAcc->setData(m_polyAcc);

    m_polyVel.push_back(QPointF(m_time, vel));
    m_curveVel->setData(m_polyVel);

    m_plot->replot();
}

// -----------------------------------------------------------------------------
void LineGraph::toggleAcceleration(bool flag)
{
    toggleGeneric(m_curveAcc, &m_acc, flag);
}

// -----------------------------------------------------------------------------
void LineGraph::toggleRawVelocity(bool flag)
{
    toggleGeneric(m_curveVel, &m_vel, flag);
}

// -----------------------------------------------------------------------------
void LineGraph::toggleFilteredVelocity(bool flag)
{
    toggleGeneric(m_curveFVel, &m_fvel, flag);
}

// -----------------------------------------------------------------------------
void LineGraph::toggleGeneric(QwtPlotCurve *curve, bool *oldflag, bool newflag)
{
    if (!*oldflag && newflag)
    {
        curve->attach(m_plot);
        m_numVisible++;
    }
    else if (*oldflag && !newflag)
    {
        curve->detach();
        m_numVisible--;
    }

    *oldflag = newflag;
    m_plot->setVisible(isVisible());
}

