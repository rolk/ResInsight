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

#include "RicShowContributingWellsFeature.h"

#include "RicShowContributingWellsFeatureImpl.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
#include "cafCmdFeatureManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowContributingWellsFeature, "RicShowContributingWellsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowContributingWellsFeature::isCommandEnabled()
{
    std::vector<RimEclipseWell*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    if (collection.size() == 1) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimEclipseWell*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);

    CAF_ASSERT(collection.size() == 1);

    RimEclipseWell* well = collection[0];
    RimEclipseView* eclipseView = nullptr;
    well->firstAncestorOrThisOfTypeAsserted(eclipseView);

    RicShowContributingWellsFeatureImpl::modifyViewToShowContributingWells(eclipseView, well->name(), eclipseView->currentTimeStep());

    RiuMainWindow::instance()->setExpanded(eclipseView, true);
    RiuMainWindow::instance()->selectAsCurrentItem(eclipseView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Show Contributing Wells");
}
