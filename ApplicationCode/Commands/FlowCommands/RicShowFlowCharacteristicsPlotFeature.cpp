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

#include "RicShowFlowCharacteristicsPlotFeature.h"

#include "RiaApplication.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimView.h"

#include "RiuMainPlotWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowFlowCharacteristicsPlotFeature, "RicShowFlowCharacteristicsPlotFeature");

RimEclipseResultCase* activeEclipseResultCase()
{
    RimView * activeView = RiaApplication::instance()->activeReservoirView();

    auto eclView = dynamic_cast<RimEclipseView*>(activeView);

    if (!eclView) return nullptr;

    auto eclCase = dynamic_cast<RimEclipseResultCase*>(eclView->ownerCase());

     return eclCase;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowFlowCharacteristicsPlotFeature::isCommandEnabled()
{
    RimEclipseResultCase* eclCase = activeEclipseResultCase();

    if (!eclCase) return false;

    if (!eclCase->defaultFlowDiagSolution()) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowFlowCharacteristicsPlotFeature::onActionTriggered(bool isChecked)
{
    RimEclipseResultCase* eclCase = activeEclipseResultCase();

    if (eclCase &&  eclCase->defaultFlowDiagSolution())
    {
        if (RiaApplication::instance()->project())
        {
            RimFlowPlotCollection* flowPlotColl = RiaApplication::instance()->project()->mainPlotCollection->flowPlotCollection();
            if (flowPlotColl)
            {
                RiuMainPlotWindow* plotwindow = RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();

                flowPlotColl->defaultFlowCharacteristicsPlot()->setFromFlowSolution(eclCase->defaultFlowDiagSolution());
                flowPlotColl->defaultFlowCharacteristicsPlot()->updateConnectedEditors();

                // Make sure the summary plot window is created and visible
                plotwindow->selectAsCurrentItem(flowPlotColl->defaultFlowCharacteristicsPlot());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowFlowCharacteristicsPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/WellAllocPlot16x16.png"));
    actionToSetup->setText("Plot Flow Characteristics");
}
