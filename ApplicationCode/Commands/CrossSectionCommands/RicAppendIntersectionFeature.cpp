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

#include "RicAppendIntersectionFeature.h"

#include "RimIntersection.h"
#include "RimIntersectionCollection.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicAppendIntersectionFeature, "RicAppendIntersectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAppendIntersectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionFeature::onActionTriggered(bool isChecked)
{
    std::vector<caf::PdmObjectHandle*> collection;
    caf::SelectionManager::instance()->objectsByType(&collection);
    CVF_ASSERT(collection.size() == 1);

    RimIntersectionCollection* crossSectionCollection = NULL;
    collection[0]->firstAncestorOrThisOfType(crossSectionCollection);

    CVF_ASSERT(crossSectionCollection);

    RicAppendIntersectionFeatureCmd* cmd = new RicAppendIntersectionFeatureCmd(crossSectionCollection);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicAppendIntersectionFeatureCmd::RicAppendIntersectionFeatureCmd(RimIntersectionCollection* crossSectionCollection)
    : CmdExecuteCommand(NULL),
    m_crossSectionCollection(crossSectionCollection)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicAppendIntersectionFeatureCmd::~RicAppendIntersectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicAppendIntersectionFeatureCmd::name()
{
    return "New Intersection";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionFeatureCmd::redo()
{
    CVF_ASSERT(m_crossSectionCollection);

    RimIntersection* crossSection = new RimIntersection();
    crossSection->name = QString("Intersection");
    m_crossSectionCollection->appendCrossSection(crossSection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionFeatureCmd::undo()
{
}
