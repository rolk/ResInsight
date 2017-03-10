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

#include "RicShowContributingWellsFromPlotFeature.h"

#include "RiaApplication.h"

#include "RicSelectViewUI.h"
#include "RicShowContributingWellsFeatureImpl.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimProject.h"
#include "RimWellAllocationPlot.h"

#include "RiuMainWindow.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowContributingWellsFromPlotFeature, "RicShowContributingWellsFromPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowContributingWellsFromPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFromPlotFeature::onActionTriggered(bool isChecked)
{
    const QString lastUsedViewKey("lastUsedViewKey");

    RimWellAllocationPlot* wellAllocationPlot = RiaApplication::instance()->activeWellAllocationPlot();
    if (!wellAllocationPlot) return;

    RimEclipseResultCase* wellAllocationResultCase = nullptr;
    wellAllocationPlot->flowDiagSolution()->firstAncestorOrThisOfTypeAsserted(wellAllocationResultCase);

    RimEclipseView* defaultSelectedView = nullptr;

    {
        QString lastUsedViewRef = RiaApplication::instance()->cacheDataObject(lastUsedViewKey).toString();
        RimEclipseView* lastUsedView = dynamic_cast<RimEclipseView*>(caf::PdmReferenceHelper::objectFromReference(RiaApplication::instance()->project(), lastUsedViewRef));
        if (lastUsedView)
        {
            RimEclipseResultCase* lastUsedViewResultCase = nullptr;
            lastUsedView->firstAncestorOrThisOfTypeAsserted(lastUsedViewResultCase);

            if (lastUsedViewResultCase == wellAllocationResultCase)
            {
                defaultSelectedView = lastUsedView;
            }
        }

        if (!defaultSelectedView)
        {
            RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
            if (activeView)
            {
                RimEclipseResultCase* activeViewResultCase = nullptr;
                activeView->firstAncestorOrThisOfTypeAsserted(activeViewResultCase);

                if (activeViewResultCase == wellAllocationResultCase)
                {
                    defaultSelectedView = activeView;
                }
                else
                {
                    if (wellAllocationResultCase->views().size() > 0)
                    {
                        defaultSelectedView = dynamic_cast<RimEclipseView*>(wellAllocationResultCase->views()[0]);
                    }
                }
            }
        }
    }
        
    RicSelectViewUI featureUi;
    if (defaultSelectedView)
    {
        featureUi.setView(defaultSelectedView);
    }
    else
    {
        featureUi.setCase(wellAllocationResultCase);
    }

    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &featureUi, "Show Contributing Wells in View", "");
    propertyDialog.resize(QSize(400, 200));
        
    if (propertyDialog.exec() != QDialog::Accepted) return;

    RimEclipseView* viewToManipulate = nullptr;
    if (featureUi.createNewView())
    {
        RimEclipseView* createdView = wellAllocationResultCase->createAndAddReservoirView();
        createdView->name = featureUi.newViewName();

        // Must be run before buildViewItems, as wells are created in this function
        createdView->loadDataAndUpdate();
        wellAllocationResultCase->updateConnectedEditors();

        viewToManipulate = createdView;
    }
    else
    {
        viewToManipulate = featureUi.selectedView();
    }

    CAF_ASSERT(viewToManipulate);

    int timeStep = wellAllocationPlot->timeStep();
    QString wellName = wellAllocationPlot->wellName();

    RicShowContributingWellsFeatureImpl::modifyViewToShowContributingWells(viewToManipulate, wellName, timeStep);

    auto* feature = caf::CmdFeatureManager::instance()->getCommandFeature("RicShowMainWindowFeature");
    feature->actionTriggered(false);

    RiuMainWindow::instance()->setExpanded(viewToManipulate, true);
    RiuMainWindow::instance()->selectAsCurrentItem(viewToManipulate);

    QString refFromProjectToView = caf::PdmReferenceHelper::referenceFromRootToObject(RiaApplication::instance()->project(), viewToManipulate);
    RiaApplication::instance()->setCacheDataObject(lastUsedViewKey, refFromProjectToView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFromPlotFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Show Contributing Wells");
}
