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
LineGraph::LineGraph(QWidget *parent, const QString &graphLabel, double scale_max,
        double dependentStep)
: m_primary(false), m_secondary(false), m_tertiary(false), m_numVisible(0), m_ticker(0)
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
    QwtText yTitle(graphLabel);
    yTitle.setFont(titleFont);

    // determine Y axis step from constructor input
    if (dependentStep == 0.0f)
        dependentStep = scale_max;

    m_plot->setAxisTitle(QwtPlot::yLeft, yTitle);
    m_plot->setAxisScale(QwtPlot::yLeft, 1.0f, scale_max, dependentStep);
    m_plot->setAxisFont(QwtPlot::yLeft, axisFont);

    // overlay a grid on the plot
    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(m_plot);

    m_curvePrimary = new QwtPlotCurve("Acceleration");
    m_curvePrimary->setPen(QPen(Qt::red, 2));
    m_curvePrimary->setRenderHint(QwtPlotItem::RenderAntialiased);

    m_curveSecondary = new QwtPlotCurve("Velocity");
    m_curveSecondary->setPen(QPen(Qt::blue, 2));
    m_curveSecondary->setRenderHint(QwtPlotItem::RenderAntialiased);
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
void LineGraph::addDataPoint(float t, float value, float secondValue)
{
    // Should I make the method take in a generic float array of data values
    // instead of taking in just two values?
    // It would probably make it a pain to in ApplicationFrame to load each 
    // value in an array and then pass it to the method

    m_time = t;

    // XXX this is awful, implement scrolling in a more reasonable fashion
    if (m_time >= 60.0)
    {
        m_time = 59.5;

        m_polyPrimary.translate(-0.5, 0.0);
        m_polyPrimary.pop_front();

        m_polySecondary.translate(-0.5, 0.0);
        m_polySecondary.pop_front();
    }

    m_polyPrimary.push_back(QPointF(m_time, value));
    m_curvePrimary->setData(m_polyPrimary);

    m_polySecondary.push_back(QPointF(m_time, secondValue));
    m_curveSecondary->setData(m_polySecondary);

    m_plot->replot();
}

// -----------------------------------------------------------------------------
void LineGraph::togglePrimaryData(bool flag)
{
    toggleGeneric(m_curvePrimary, &m_primary, flag);
}

// -----------------------------------------------------------------------------
void LineGraph::toggleSecondaryData(bool flag)
{
    toggleGeneric(m_curveSecondary, &m_secondary, flag);
}

// -----------------------------------------------------------------------------
void LineGraph::toggleTertiaryData(bool flag)
{
    toggleGeneric(m_curveTertiary, &m_tertiary, flag);
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

