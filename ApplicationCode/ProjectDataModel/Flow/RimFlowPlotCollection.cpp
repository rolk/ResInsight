/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimFlowPlotCollection.h"

#include "RimWellAllocationPlot.h"
#include "RimFlowCharacteristicsPlot.h"


#include "cvfAssert.h"
#include "cafProgressInfo.h"

CAF_PDM_SOURCE_INIT(RimFlowPlotCollection, "FlowPlotCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::RimFlowPlotCollection()
{
    CAF_PDM_InitObject("Flow Diagnostics Plots", ":/WellAllocPlots16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_flowCharacteristicsPlot, "FlowCharacteristicsPlot", "", "", "", "");
    m_flowCharacteristicsPlot.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_defaultWellAllocPlot, "DefaultFlowPlot", "", "", "", "");
    m_defaultWellAllocPlot.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_storedWellAllocPlots, "FlowPlots", "Stored Plots",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowPlotCollection::~RimFlowPlotCollection()
{
    delete m_defaultWellAllocPlot();

    m_storedWellAllocPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::closeDefaultPlotWindowAndDeletePlots()
{
    if ( m_defaultWellAllocPlot )
    {
        m_defaultWellAllocPlot->removeFromMdiAreaAndDeleteViewWidget();
        delete m_defaultWellAllocPlot();
    }
    m_storedWellAllocPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::loadDataAndUpdate()
{
    caf::ProgressInfo plotProgress(m_storedWellAllocPlots.size() + 1, "");

    if (m_defaultWellAllocPlot) m_defaultWellAllocPlot->loadDataAndUpdate();
    plotProgress.incrementProgress();

    for (RimWellAllocationPlot* p : m_storedWellAllocPlots)
    {
        p->loadDataAndUpdate();
        plotProgress.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimFlowPlotCollection::plotCount() const
{
    size_t plotCount = 0;
    if (m_defaultWellAllocPlot) plotCount = 1;
    plotCount += m_storedWellAllocPlots.size();
    return plotCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowPlotCollection::addWellAllocPlotToStoredPlots(RimWellAllocationPlot* plot)
{
    m_storedWellAllocPlots.push_back(plot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot* RimFlowPlotCollection::defaultWellAllocPlot()
{
    if ( !m_defaultWellAllocPlot() ) 
    {
        m_defaultWellAllocPlot = new RimWellAllocationPlot; 
        m_defaultWellAllocPlot->setDescription("Default Flow Diagnostics Plot");
    }

    this->updateConnectedEditors();

    return m_defaultWellAllocPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowCharacteristicsPlot* RimFlowPlotCollection::defaultFlowCharacteristicsPlot()
{
    if ( !m_flowCharacteristicsPlot() ) 
    {
        m_flowCharacteristicsPlot = new RimFlowCharacteristicsPlot; 
    }

    this->updateConnectedEditors();

    return m_flowCharacteristicsPlot();  
}
