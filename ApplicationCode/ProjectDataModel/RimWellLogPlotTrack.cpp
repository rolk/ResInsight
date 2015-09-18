/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimWellLogPlotTrack.h"

#include "RimWellLogPlot.h"
#include "RimWellLogPlotCurve.h"

#include "RiuWellLogTrackPlot.h"
#include "RiuWellLogPlot.h"
#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"
#include "cvfAssert.h"

#include <math.h>

#define RI_LOGPLOTTRACK_MINX_DEFAULT    -10.0
#define RI_LOGPLOTTRACK_MAXX_DEFAULT    100.0


CAF_PDM_SOURCE_INIT(RimWellLogPlotTrack, "WellLogPlotTrack");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrack::RimWellLogPlotTrack()
{
    CAF_PDM_InitObject("Track", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_userName, "TrackDescription", "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_show, "Show", true, "Show track", "", "", "");
    m_show.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&curves, "Curves", "",  "", "", "");
    curves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_visibleXRangeMin, "VisibleXRangeMin", RI_LOGPLOTTRACK_MINX_DEFAULT, "Min", "", "", "");
    CAF_PDM_InitField(&m_visibleXRangeMax, "VisibleXRangeMax", RI_LOGPLOTTRACK_MAXX_DEFAULT, "Max", "", "", "");   
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrack::~RimWellLogPlotTrack()
{
    delete m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::setDescription(const QString& description)
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_show)
    {
        if (m_viewer) m_viewer->setVisible(m_show());
    }
    else if (changedField == &m_visibleXRangeMin || changedField == &m_visibleXRangeMax)
    {
        m_viewer->setAxisScale(QwtPlot::xTop, m_visibleXRangeMin, m_visibleXRangeMax);
        m_viewer->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotTrack::objectToggleField()
{
    return &m_show;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellLogPlotTrack::userDescriptionField()
{
    return &m_userName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::addCurve(RimWellLogPlotCurve* curve)
{
    curves.push_back(curve);

    if (m_viewer)
    {
        curve->setPlot(m_viewer);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogTrackPlot* RimWellLogPlotTrack::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotTrack::availableDepthRange(double* minimumDepth, double* maximumDepth)
{
    double minDepth = HUGE_VAL;
    double maxDepth = -HUGE_VAL;

    size_t curveCount = curves.size();
    if (curveCount < 1)
    {
        return false;
    }

    bool rangeUpdated = false;

    for (size_t cIdx = 0; cIdx < curveCount; cIdx++)
    {
        double minCurveDepth = HUGE_VAL;
        double maxCurveDepth = -HUGE_VAL;

        if (curves[cIdx]->depthRange(&minCurveDepth, &maxCurveDepth))
        {
            if (minCurveDepth < minDepth)
            {
                minDepth = minCurveDepth;
                rangeUpdated = true;
            }

            if (maxCurveDepth > maxDepth)
            {
                maxDepth = maxCurveDepth;
                rangeUpdated = true;
            }
        }
    }

    if (rangeUpdated)
    {
        *minimumDepth = minDepth;
        *maximumDepth = maxDepth;
    }

    return rangeUpdated;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::loadDataAndUpdate()
{
    CVF_ASSERT(m_viewer);

    RimWellLogPlot* wellLogPlot;
    firstAnchestorOrThisOfType(wellLogPlot);
    if (wellLogPlot)
    {
        m_viewer->setDepthTitle(wellLogPlot->depthPlotTitle());
    }

    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->updatePlotData();
    }

    updateXAxisRangeFromCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::recreateViewer()
{
    CVF_ASSERT(m_viewer == NULL);

    m_viewer = new RiuWellLogTrackPlot(this);
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->setPlot(this->m_viewer);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::detachAllCurves()
{
    for (size_t cIdx = 0; cIdx < curves.size(); ++cIdx)
    {
        curves[cIdx]->detachCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::updateAxisRangesAndReplot()
{
    bool rangesChanged = false;

    if (m_viewer)
    {
        RimWellLogPlot* wellLogPlot;
        firstAnchestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            double minimumDepth, maximumDepth;
            wellLogPlot->visibleDepthRange(&minimumDepth, &maximumDepth);

            m_viewer->setDepthRange(minimumDepth, maximumDepth);
            rangesChanged = true;
        }

        // Assume auto-scaling on X-axis as long as curves exist, reset to default if not
        if (curves.size() < 1)
        {
            m_viewer->setAxisScale(QwtPlot::xTop, RI_LOGPLOTTRACK_MINX_DEFAULT, RI_LOGPLOTTRACK_MAXX_DEFAULT);
            rangesChanged = true;
        }

        if (rangesChanged)
        {
            m_viewer->replot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::updateXAxisRangeFromCurves()
{
    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    size_t curveCount = curves.size();
    if (curveCount < 1) return;

    bool rangeUpdated = false;

    for (size_t cIdx = 0; cIdx < curveCount; cIdx++)
    {
        double minCurveValue = HUGE_VAL;
        double maxCurveValue = -HUGE_VAL;

        if (curves[cIdx]->valueRange(&minCurveValue, &maxCurveValue))
        {
            if (minCurveValue < minValue)
            {
                minValue = minCurveValue;
                rangeUpdated = true;
            }

            if (maxCurveValue > maxValue)
            {
                maxValue = maxCurveValue;
                rangeUpdated = true;
            }
        }
    }

    if (rangeUpdated)
    {
        m_visibleXRangeMin = minValue;
        m_visibleXRangeMax = maxValue;

        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCurve* RimWellLogPlotTrack::curveDefinitionFromCurve(const QwtPlotCurve* curve) const
{
    for (size_t idx = 0; idx < curves.size(); idx++)
    {
        if (curves[idx]->plotCurve() == curve)
        {
            return curves[idx];
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotTrack::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_userName);

    caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup("Visible X Axis Range");
    gridGroup->add(&m_visibleXRangeMin);
    gridGroup->add(&m_visibleXRangeMax);
}
