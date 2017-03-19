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

#include "RicSelectSummaryPlotUI.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RiaApplication.h"
#include "RimSummaryPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimSummaryPlot.h"


CAF_PDM_SOURCE_INIT(RicSelectSummaryPlotUI, "RicSelectSummaryPlotUI");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSelectSummaryPlotUI::RicSelectSummaryPlotUI()
{
    CAF_PDM_InitObject("RicSelectSummaryPlotUI", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryPlot,  "SelectedSummaryPlot",                    "Select Summary Plot", "", "", "");
    CAF_PDM_InitField(&m_createNewPlot,                 "CreateNewPlot", false,                   "Create New Plot", "", "", "");
    CAF_PDM_InitField(&m_newSummaryPlotName,            "NewViewName",   QString("Summary Plot"), "New Summary Plot Name", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSelectSummaryPlotUI::setDefaultSummaryPlot(RimSummaryPlot* summaryPlot)
{
    m_selectedSummaryPlot = summaryPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSelectSummaryPlotUI::selectedSummaryPlot() const
{
    return m_selectedSummaryPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSelectSummaryPlotUI::createNewPlot() const
{
    return m_createNewPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSelectSummaryPlotUI::newPlotName() const
{
    return m_newSummaryPlotName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSelectSummaryPlotUI::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_selectedSummaryPlot)
    {
        for (RimSummaryPlot* plot : RicSelectSummaryPlotUI::summaryPlots())
        {
            QIcon icon = plot->uiCapability()->uiIcon();
            QString displayName = plot->description();

            options.push_back(caf::PdmOptionItemInfo(displayName, plot, false, icon));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSelectSummaryPlotUI::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    if (RicSelectSummaryPlotUI::summaryPlots().size() == 0)
    {
        m_createNewPlot = true;
    }

    if (m_createNewPlot)
    {
        m_newSummaryPlotName.uiCapability()->setUiReadOnly(false);
        m_selectedSummaryPlot.uiCapability()->setUiReadOnly(true);
    }
    else
    {
        m_newSummaryPlotName.uiCapability()->setUiReadOnly(true);
        m_selectedSummaryPlot.uiCapability()->setUiReadOnly(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RicSelectSummaryPlotUI::summaryPlots()
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT(mainPlotColl);

    RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();
    CVF_ASSERT(summaryPlotColl);

    std::vector<RimSummaryPlot*> sumPlots;
    summaryPlotColl->descendantsIncludingThisOfType(sumPlots);

    return sumPlots;
}

