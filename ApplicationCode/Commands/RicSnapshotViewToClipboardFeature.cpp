/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicSnapshotViewToClipboardFeature.h"

#include "RiaApplication.h"

#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimViewWindow.h"
#include "RimWellLogPlot.h"
#include "RiuMainPlotWindow.h"
#include "RiuWellLogPlot.h"

#include "cafUtils.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiSubWindow>


CAF_CMD_SOURCE_INIT(RicSnapshotViewToClipboardFeature, "RicSnapshotViewToClipboardFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSnapshotViewToClipboardFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToClipboardFeature::onActionTriggered(bool isChecked)
{
    RimViewWindow* viewWindow = RiaApplication::activeViewWindow();

    if (viewWindow)
    {
        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
            QImage image = viewWindow->snapshotWindowContent();
            if (!image.isNull())
            {
                clipboard->setImage(image);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToClipboardFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Snapshot To Clipboard");
    actionToSetup->setIcon(QIcon(":/SnapShot.png"));
}




CAF_CMD_SOURCE_INIT(RicSnapshotViewToFileFeature, "RicSnapshotViewToFileFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveSnapshotAs(const QString& fileName, RimViewWindow* viewWindow)
{
    if (viewWindow)
    {
        QImage image = viewWindow->snapshotWindowContent();
        if (!image.isNull())
        {
            if (image.save(fileName))
            {
                qDebug() << "Saved snapshot image to " << fileName;
            }
            else
            {
                qDebug() << "Error when trying to save snapshot image to " << fileName;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSnapshotViewToFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

    QString startPath;
    if (!proj->fileName().isEmpty())
    {
        QFileInfo fi(proj->fileName());
        startPath = fi.absolutePath();
    }
    else
    {
        startPath = app->lastUsedDialogDirectory("IMAGE_SNAPSHOT");
    }

    startPath += "/image.png";

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"), startPath, tr("Image files (*.bmp *.png * *.jpg)"));
    if (fileName.isEmpty())
    {
        return;
    }

    // Remember the directory to next time
    app->setLastUsedDialogDirectory("IMAGE_SNAPSHOT", QFileInfo(fileName).absolutePath());

    RimViewWindow* viewWindow = app->activeViewWindow();
    RicSnapshotViewToFileFeature::saveSnapshotAs(fileName, viewWindow);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Snapshot To File");
    actionToSetup->setIcon(QIcon(":/SnapShotSave.png"));
}





CAF_CMD_SOURCE_INIT(RicSnapshotAllPlotsToFileFeature, "RicSnapshotAllPlotsToFileFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::saveAllPlots()
{
    RiaApplication* app = RiaApplication::instance();

    RiuMainPlotWindow* mainPlotWindow = app->mainPlotWindow();
    if (!mainPlotWindow) return;

    RimProject* proj = app->project();
    if (!proj) return;

    // Save images in snapshot catalog relative to project directory
    QString snapshotFolderName = app->createAbsolutePathFromProjectRelativePath("snapshots");

    QDir snapshotPath(snapshotFolderName);
    if (!snapshotPath.exists())
    {
        if (!snapshotPath.mkpath(".")) return;
    }

    const QString absSnapshotPath = snapshotPath.absolutePath();

    // Well log plots
    {
        std::vector<RimWellLogPlot*> wellLogPlots;
        proj->descendantsIncludingThisOfType(wellLogPlots);
        for (RimWellLogPlot* wellLogPlot : wellLogPlots)
        {
            if (wellLogPlot && wellLogPlot->viewWidget())
            {
                QString fileName = wellLogPlot->description();
                fileName.replace(" ", "_");

                QString absoluteFileName = caf::Utils::constructFullFileName(absSnapshotPath, fileName, ".png");

                RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, wellLogPlot);
            }
        }
    }

    // Summary plots
    {
        std::vector<RimSummaryPlot*> summaryPlots;
        proj->descendantsIncludingThisOfType(summaryPlots);
        for (RimSummaryPlot* summaryPlot : summaryPlots)
        {
            if (summaryPlot && summaryPlot->viewWidget())
            {
                QString fileName = summaryPlot->description();
                fileName.replace(" ", "_");

                QString absoluteFileName = caf::Utils::constructFullFileName(absSnapshotPath, fileName, ".png");

                RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, summaryPlot);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSnapshotAllPlotsToFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::onActionTriggered(bool isChecked)
{
    QWidget* currentActiveWidget = nullptr;
    if (RiaApplication::activeViewWindow())
    {
        currentActiveWidget = RiaApplication::activeViewWindow()->viewWidget();
    }

    RicSnapshotAllPlotsToFileFeature::saveAllPlots();

    if (currentActiveWidget)
    {
        RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
        if (mainPlotWindow)
        {
            mainPlotWindow->setActiveViewer(currentActiveWidget);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Snapshot All Plots To File");
    actionToSetup->setIcon(QIcon(":/SnapShotSave.png"));
}

