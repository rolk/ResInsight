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

#include "RicCloseCaseFeature.h"

#include "RiaApplication.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicCloseCaseFeature, "RicCloseCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseCaseFeature::isCommandEnabled()
{
    return selectedEclipseCase() != NULL || selectedGeoMechCase() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::onActionTriggered(bool isChecked)
{
    RimEclipseCase* eclipseCase = selectedEclipseCase();
    RimGeoMechCase* geoMechCase = selectedGeoMechCase();
    if (eclipseCase)
    {
        std::vector<RimEclipseCase*> casesToBeDeleted;
        casesToBeDeleted.push_back(eclipseCase);
        
        if (userConfirmedGridCaseGroupChange(casesToBeDeleted))
        {
            deleteEclipseCase(eclipseCase);
        }
    }
    else if (geoMechCase)
    {
        deleteGeoMechCase(geoMechCase);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Close");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicCloseCaseFeature::selectedEclipseCase() const
{
    std::vector<RimEclipseCase*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RicCloseCaseFeature::selectedGeoMechCase() const
{
    std::vector<RimGeoMechCase*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::removeCaseFromAllGroups(RimEclipseCase* eclipseCase)
{
    CVF_ASSERT(eclipseCase);

    RimProject* proj = RiaApplication::instance()->project();
    RimOilField* activeOilField = proj ? proj->activeOilField() : NULL;
    RimEclipseCaseCollection* analysisModels = (activeOilField) ? activeOilField->analysisModels() : NULL;
    if (analysisModels)
    {
        analysisModels->removeCaseFromAllGroups(eclipseCase);
        analysisModels->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::deleteEclipseCase(RimEclipseCase* eclipseCase)
{
    CVF_ASSERT(eclipseCase);

    RimCaseCollection* caseCollection = eclipseCase->parentCaseCollection();
    if (caseCollection)
    {
        if (RimIdenticalGridCaseGroup::isStatisticsCaseCollection(caseCollection))
        {
            RimIdenticalGridCaseGroup* caseGroup = caseCollection->parentCaseGroup();
            CVF_ASSERT(caseGroup);

            caseGroup->statisticsCaseCollection()->reservoirs.removeChildObject(eclipseCase);
            caseGroup->updateConnectedEditors();
        }
        else
        {
            removeCaseFromAllGroups(eclipseCase);
        }
    }
    else
    {
        removeCaseFromAllGroups(eclipseCase);
    }

    delete eclipseCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::deleteGeoMechCase(RimGeoMechCase* geoMechCase)
{
    CVF_ASSERT(geoMechCase);

    RimProject* proj = RiaApplication::instance()->project();
    RimOilField* activeOilField = proj ? proj->activeOilField() : NULL;
    RimGeoMechModels* models = (activeOilField) ? activeOilField->geoMechModels() : NULL;
    if (models)
    {
        models->cases.removeChildObject(geoMechCase);
        models->updateConnectedEditors();
    }

    delete geoMechCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseCaseFeature::hasAnyStatisticsResults(RimIdenticalGridCaseGroup* gridCaseGroup)
{
    CVF_ASSERT(gridCaseGroup);

    for (size_t i = 0; i < gridCaseGroup->statisticsCaseCollection()->reservoirs().size(); i++)
    {
        RimEclipseStatisticsCase* rimStaticsCase = dynamic_cast<RimEclipseStatisticsCase*>(gridCaseGroup->statisticsCaseCollection()->reservoirs[i]);
        if (rimStaticsCase)
        {
            if (rimStaticsCase->hasComputedStatistics())
            {
                return true;
            }
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseCaseFeature::userConfirmedGridCaseGroupChange(const std::vector<RimEclipseCase*>& casesToBeDeleted)
{
    std::vector<RimIdenticalGridCaseGroup*> gridCaseGroups;

    for (size_t i = 0; i < casesToBeDeleted.size(); i++)
    {
        RimIdenticalGridCaseGroup* gridCaseGroup = NULL;
        casesToBeDeleted[i]->firstAnchestorOrThisOfType(gridCaseGroup);

        if (gridCaseGroup && hasAnyStatisticsResults(gridCaseGroup))
        {
            gridCaseGroups.push_back(gridCaseGroup);
        }
    }

    if (gridCaseGroups.size() > 0)
    {
        RiuMainWindow* mainWnd = RiuMainWindow::instance();

        QMessageBox msgBox(mainWnd);
        msgBox.setIcon(QMessageBox::Question);

        QString questionText;
        if (gridCaseGroups.size() == 1)
        {
            questionText = QString("This operation will invalidate statistics results in grid case group\n\"%1\".\n").arg(gridCaseGroups[0]->name());
            questionText += "Computed results in this group will be deleted if you continue.";
        }
        else
        {
            questionText = "This operation will invalidate statistics results in grid case groups\n";
            for (size_t i = 0; i < gridCaseGroups.size(); i++)
            {
                questionText += QString("\"%1\"\n").arg(gridCaseGroups[i]->name());
            }

            questionText += "Computed results in these groups will be deleted if you continue.";
        }

        msgBox.setText(questionText);
        msgBox.setInformativeText("Do you want to continue?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        int ret = msgBox.exec();
        if (ret == QMessageBox::No)
        {
            return false;
        }
    }

    return true;
}
