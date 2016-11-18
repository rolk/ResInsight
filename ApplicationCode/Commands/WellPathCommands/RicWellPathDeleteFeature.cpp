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

#include "RicWellPathDeleteFeature.h"

#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>
#include "RimWellPath.h"

namespace caf
{

CAF_CMD_SOURCE_INIT(RicWellPathDeleteFeature, "RicWellPathDeleteFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathDeleteFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() == 1)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathDeleteFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() == 0) return;

    RimWellPath* wellPath = objects[0];

    RimWellPathCollection* wellPathCollection = NULL;
    wellPath->firstAncestorOrThisOfType(wellPathCollection);

    wellPathCollection->removeWellPath(wellPath);;
    delete wellPath;

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleGeometryRegenAndRedrawViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathDeleteFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Well Path");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

} // end namespace caf
