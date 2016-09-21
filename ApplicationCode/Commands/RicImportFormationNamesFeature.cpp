/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicImportFormationNamesFeature.h"

#include "RimFormationNamesCollection.h"
#include "RiaApplication.h"
#include "RimProject.h"
#include "RimOilField.h"
#include "RiuMainWindow.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportFormationNamesFeature, "RicImportFormationNamesFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportFormationNamesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("BINARY_GRID");
    QStringList fileNames = QFileDialog::getOpenFileNames(RiuMainWindow::instance(), "Import Formation Names", defaultDir, "Formation Names description File (*.lyr);;All Files (*.*)");

    if (fileNames.isEmpty()) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("BINARY_GRID", QFileInfo(fileNames.last()).absolutePath());

    // Find or create the FomationNamesCollection

    RimProject* proj = RiaApplication::instance()->project();
    RimFormationNamesCollection* fomNameColl = proj->activeOilField()->formationNamesCollection();
    if (!fomNameColl)
    {
        fomNameColl = new RimFormationNamesCollection;
        proj->activeOilField()->formationNamesCollection = fomNameColl;
    }

    // For each file, find existing Formation names item, or create new

    fomNameColl->importFiles(fileNames);
    proj->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FormationCollection16x16.png"));
    actionToSetup->setText("Import Formation Names");
}
