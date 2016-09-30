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

#include "RiuViewerCommands.h"

#include "RiaApplication.h"

#include "RicViewerEventInterface.h"
#include "RicEclipsePropertyFilterNewExec.h"
#include "RicGeoMechPropertyFilterNewExec.h"
#include "RicRangeFilterNewExec.h"
#include "WellPathCommands/RicWellPathViewerEventHandler.h"

#include "RigCaseData.h"
#include "RigFault.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigGeoMechCaseData.h"

#include "RimCellEdgeColors.h"
#include "RimContextCommandBuilder.h"
#include "RimIntersection.h"
#include "RimDefines.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimFault.h"
#include "RimFaultCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimIntersectionBox.h"
#include "RimLegendConfig.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewController.h"
#include "RimWellPath.h"

#include "RiuMainWindow.h"
#include "RiuSelectionColors.h"
#include "RiuSelectionManager.h"
#include "RiuViewer.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivFemPickSourceInfo.h"
#include "RivIntersectionBoxSourceInfo.h"
#include "RivIntersectionSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivTernarySaturationOverlayItem.h"
#include "RivWellPathSourceInfo.h"
#include "RivWellPipeSourceInfo.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include "cvfDrawableGeo.h"
#include "cvfHitItemCollection.h"
#include "cvfOverlayAxisCross.h"
#include "cvfOverlayScalarMapperLegend.h"
#include "cvfPart.h"

#include <QMenu>
#include <QMouseEvent>
#include <QStatusBar>



//==================================================================================================
//
// RiaViewerCommands
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewerCommands::RiuViewerCommands(RiuViewer* ownerViewer) 
    : QObject(ownerViewer), 
      m_viewer(ownerViewer), 
      m_currentGridIdx(-1),
      m_currentCellIndex(-1),
      m_currentPickPositionInDomainCoords(cvf::Vec3d::UNDEFINED)
{
    {
        caf::CmdFeature* cmdFeature = caf::CmdFeatureManager::instance()->getCommandFeature("RicNewPolylineIntersectionFeature");
        CVF_ASSERT(cmdFeature);

        m_viewerEventHandlers.push_back(dynamic_cast<RicViewerEventInterface*>(cmdFeature));
    }

    {
        m_viewerEventHandlers.push_back(dynamic_cast<RicViewerEventInterface*>(RicWellPathViewerEventHandler::instance()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewerCommands::~RiuViewerCommands()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::setOwnerView(RimView * owner)
{
    m_reservoirView = owner;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::displayContextMenu(QMouseEvent* event)
{
    m_currentGridIdx = cvf::UNDEFINED_SIZE_T;
    m_currentCellIndex = cvf::UNDEFINED_SIZE_T;
    m_currentPickedObject = nullptr;

    int winPosX = event->x();
    int winPosY = event->y();

    QMenu menu;

    uint firstPartTriangleIndex = cvf::UNDEFINED_UINT;
    cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);

    cvf::Part* firstHitPart = NULL;
    cvf::Part* nncFirstHitPart = NULL;

    m_currentPickPositionInDomainCoords = cvf::Vec3d::UNDEFINED;

    cvf::HitItemCollection hitItems;
    if (m_viewer->rayPick(winPosX, winPosY, &hitItems))
    {
        extractIntersectionData(hitItems, &localIntersectionPoint, &firstHitPart, &firstPartTriangleIndex, &nncFirstHitPart, NULL);

        cvf::Vec3d displayModelOffset = cvf::Vec3d::ZERO;

        RimView* activeView = RiaApplication::instance()->activeReservoirView();
        CVF_ASSERT(activeView);

        RimCase* rimCase = NULL;
        activeView->firstAncestorOrThisOfType(rimCase);
        if (rimCase)
        {
            displayModelOffset = rimCase->displayModelOffset();
        }

        m_currentPickPositionInDomainCoords = localIntersectionPoint + displayModelOffset;
    }

    if (firstHitPart && firstPartTriangleIndex != cvf::UNDEFINED_UINT)
    {
        const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
        const RivFemPickSourceInfo* femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>(firstHitPart->sourceInfo());
        const RivIntersectionSourceInfo* crossSectionSourceInfo = dynamic_cast<const RivIntersectionSourceInfo*>(firstHitPart->sourceInfo());
        const RivIntersectionBoxSourceInfo* intersectionBoxSourceInfo = dynamic_cast<const RivIntersectionBoxSourceInfo*>(firstHitPart->sourceInfo());

        if (rivSourceInfo || femSourceInfo || crossSectionSourceInfo || intersectionBoxSourceInfo)
        {
            if (rivSourceInfo)
            {
                if (!rivSourceInfo->hasCellFaceMapping()) return;

                // Set the data regarding what was hit

                m_currentGridIdx = rivSourceInfo->gridIndex();
                m_currentCellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex(firstPartTriangleIndex);
                m_currentFaceIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace(firstPartTriangleIndex);
            }
            else if (femSourceInfo)
            {
                m_currentGridIdx = femSourceInfo->femPartIndex();
                m_currentCellIndex = femSourceInfo->triangleToElmMapper()->elementIndex(firstPartTriangleIndex);
            }
            else if (crossSectionSourceInfo)
            {
                findCellAndGridIndex(crossSectionSourceInfo, firstPartTriangleIndex, &m_currentCellIndex, &m_currentGridIdx);
                m_currentFaceIndex = cvf::StructGridInterface::NO_FACE;
                m_currentPickedObject = const_cast<RimIntersection*>(crossSectionSourceInfo->crossSection());

                menu.addAction(QString("Hide intersection"), this, SLOT(slotHideIntersection()));
            }
            else if (intersectionBoxSourceInfo)
            {
                findCellAndGridIndex(intersectionBoxSourceInfo, firstPartTriangleIndex, &m_currentCellIndex, &m_currentGridIdx);
                m_currentFaceIndex = cvf::StructGridInterface::NO_FACE;

                m_currentPickedObject = const_cast<RimIntersectionBox*>(intersectionBoxSourceInfo->intersectionBox());
            }

            // IJK -slice commands

            RimViewController* viewController = NULL;
            if (m_reservoirView) viewController = m_reservoirView->viewController();    

            if (!viewController || !viewController->isRangeFiltersControlled())
            {
                menu.addAction(QIcon(":/CellFilter_Range.png"), QString("I-slice range filter"), this, SLOT(slotRangeFilterI()));
                menu.addAction(QIcon(":/CellFilter_Range.png"), QString("J-slice range filter"), this, SLOT(slotRangeFilterJ()));
                menu.addAction(QIcon(":/CellFilter_Range.png"), QString("K-slice range filter"), this, SLOT(slotRangeFilterK()));

                menu.addSeparator();

                menu.addAction(caf::CmdFeatureManager::instance()->action("RicIntersectionBoxXSliceFeature"));
                menu.addAction(caf::CmdFeatureManager::instance()->action("RicIntersectionBoxYSliceFeature"));
                menu.addAction(caf::CmdFeatureManager::instance()->action("RicIntersectionBoxZSliceFeature"));
            
                menu.addSeparator();
            }

            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
            if (eclipseView)
            {
                RimEclipseCellColors* cellColors = eclipseView->cellResult().p();
                if (cellColors)
                {
                    QAction* propertyAction = new QAction(QIcon(":/CellFilter_Values.png"), QString("Add property filter"), this);
                    connect(propertyAction, SIGNAL(triggered()), SLOT(slotAddEclipsePropertyFilter()));

                    bool isPerCellFaceResult = RimDefines::isPerCellFaceResult(cellColors->resultVariable());
                    if (isPerCellFaceResult)
                    {
                        propertyAction->setEnabled(false);
                    }

                    if (!viewController || !viewController->isPropertyFilterOveridden())
                    {
                        menu.addAction(propertyAction);
                    }
                }

                // Hide faults command
                const RigCaseData* reservoir = eclipseView->eclipseCase()->reservoirData();
                const RigFault* fault = reservoir->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
                if (fault)
                {
                    menu.addSeparator();

                    QString faultName = fault->name();
                    menu.addAction(QString("Hide ") + faultName, this, SLOT(slotHideFault()));
                }
            }

            RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
            if (geoMechView)
            {
                RimGeoMechCellColors* cellColors = geoMechView->cellResult().p();
                if (cellColors)
                {
                     if (!viewController || !viewController->isPropertyFilterOveridden())
                    {
                        menu.addAction(QIcon(":/CellFilter_Values.png"), QString("Add property filter"), this, SLOT(slotAddGeoMechPropertyFilter()));
                    }
                }
            }
        }
    }

    QStringList commandIds;

    // Well log curve creation commands
    if (firstHitPart && firstHitPart->sourceInfo())
    {
        const RivWellPathSourceInfo* wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>(firstHitPart->sourceInfo());
        if (wellPathSourceInfo)
        {
            RimWellPath* wellPath = wellPathSourceInfo->wellPath();
            if (wellPath)
            {
                caf::SelectionManager::instance()->setSelectedItem(wellPath);

                commandIds << "RicNewWellLogFileCurveFeature";
                commandIds << "RicNewWellLogCurveExtractionFeature";
                commandIds << "RicNewWellPathIntersectionFeature";
            }
        }

        const RivEclipseWellSourceInfo* eclipseWellSourceInfo = dynamic_cast<const RivEclipseWellSourceInfo*>(firstHitPart->sourceInfo());
        if (eclipseWellSourceInfo)
        {
            RimEclipseWell* well = eclipseWellSourceInfo->well();
            if (well)
            {
                caf::SelectionManager::instance()->setSelectedItem(well);

                commandIds << "RicNewSimWellIntersectionFeature";
            }
        }

    }

    // View Link commands
    if (!firstHitPart)
    {
        commandIds << "RicLinkViewFeature";
        commandIds << "RicUnLinkViewFeature";
        commandIds << "RicShowLinkOptionsFeature";
        commandIds << "RicSetMasterViewFeature";
    }

    commandIds << "RicNewPolylineIntersectionFeature";

    RimContextCommandBuilder::appendCommandsToMenu(commandIds, &menu);

    if (menu.actions().size() > 0)
    {
        menu.exec(event->globalPos());
    }
}

//--------------------------------------------------------------------------------------------------
/// Todo: Move this to a command instead
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotRangeFilterI()
{
    createSliceRangeFilter(0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotRangeFilterJ()
{
    createSliceRangeFilter(1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotRangeFilterK()
{
    createSliceRangeFilter(2);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::createSliceRangeFilter(int ijOrk)
{
    RimView* eclipseView = m_reservoirView.p();
    if (!eclipseView) return;

    size_t i, j, k;
    ijkFromCellIndex(m_currentGridIdx, m_currentCellIndex, &i, &j, &k);

    RimCellRangeFilterCollection* rangeFilterCollection = eclipseView->rangeFilterCollection();

    RicRangeFilterNewExec* filterExec = new RicRangeFilterNewExec(rangeFilterCollection);

    if (ijOrk == 0){
        filterExec->m_iSlice = true;
        filterExec->m_iSliceStart = CVF_MAX(static_cast<int>(i + 1), 1);
    }
    else if (ijOrk == 1){
        filterExec->m_jSlice = true;
        filterExec->m_jSliceStart = CVF_MAX(static_cast<int>(j + 1), 1);

    }
    else if (ijOrk == 2){
        filterExec->m_kSlice = true;
        filterExec->m_kSliceStart = CVF_MAX(static_cast<int>(k + 1), 1);
    }

    caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);

    eclipseView->setSurfaceDrawstyle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotHideFault()
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    if(!eclipseView) return;

    const RigCaseData* reservoir = eclipseView->eclipseCase()->reservoirData();
    const RigFault* fault = reservoir->mainGrid()->findFaultFromCellIndexAndCellFace(m_currentCellIndex, m_currentFaceIndex);
    if (fault)
    {
        QString faultName = fault->name();

        RimFault* rimFault = eclipseView->faultCollection()->findFaultByName(faultName);
        if (rimFault)
        {
            caf::PdmUiFieldHandle* uiFieldHandle = rimFault->showFault.uiCapability();
            if (uiFieldHandle)
            {
                uiFieldHandle->setValueFromUi(!rimFault->showFault);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotAddEclipsePropertyFilter()
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    if (eclipseView)
    {
        RimEclipsePropertyFilterCollection* filterCollection = eclipseView->eclipsePropertyFilterCollection();
        RicEclipsePropertyFilterNewExec* propCmdExe = new RicEclipsePropertyFilterNewExec(filterCollection);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(propCmdExe);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotAddGeoMechPropertyFilter()
{
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
    if (geoMechView)
    {
        RimGeoMechPropertyFilterCollection* filterCollection = geoMechView->geoMechPropertyFilterCollection();
        RicGeoMechPropertyFilterNewExec* propCmdExe = new RicGeoMechPropertyFilterNewExec(filterCollection);
        caf::CmdExecCommandManager::instance()->processExecuteCommand(propCmdExe);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::slotHideIntersection()
{
    RimIntersection* rimIntersection = dynamic_cast<RimIntersection*>(currentPickedObject());
    if (rimIntersection)
    {
        rimIntersection->isActive = false;
        rimIntersection->updateConnectedEditors();

        if (m_reservoirView)
        {
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::handlePickAction(int winPosX, int winPosY, Qt::KeyboardModifiers keyboardModifiers)
{
    if (handleOverlayItemPicking(winPosX, winPosY))
    {
        return;
    }

    size_t gridIndex = cvf::UNDEFINED_SIZE_T;
    size_t cellIndex = cvf::UNDEFINED_SIZE_T;
    size_t nncIndex = cvf::UNDEFINED_SIZE_T;
    cvf::StructGridInterface::FaceType face = cvf::StructGridInterface::NO_FACE;
    cvf::Vec3d localIntersectionPoint(cvf::Vec3d::ZERO);

    // Extract all the above information from the pick
    {
        cvf::Part* firstHitPart = NULL;
        uint firstPartTriangleIndex = cvf::UNDEFINED_UINT;

        cvf::Part* firstNncHitPart = NULL;
        uint nncPartTriangleIndex = cvf::UNDEFINED_UINT;

        cvf::HitItemCollection hitItems;
        if (m_viewer->rayPick(winPosX, winPosY, &hitItems))
        {
            extractIntersectionData(hitItems, &localIntersectionPoint, &firstHitPart, &firstPartTriangleIndex, &firstNncHitPart, &nncPartTriangleIndex);

            cvf::ref<RicViewerEventObject> eventObj = new RicViewerEventObject(localIntersectionPoint, firstHitPart, firstPartTriangleIndex);
            for (size_t i = 0; i < m_viewerEventHandlers.size(); i++)
            {
                if (m_viewerEventHandlers[i]->handleEvent(eventObj.p()))
                {
                    return;
                }
            }
        }

        if (firstHitPart && firstHitPart->sourceInfo())
        {
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstHitPart->sourceInfo());
            const RivFemPickSourceInfo* femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>(firstHitPart->sourceInfo());
            const RivIntersectionSourceInfo* crossSectionSourceInfo = dynamic_cast<const RivIntersectionSourceInfo*>(firstHitPart->sourceInfo());
            const RivIntersectionBoxSourceInfo* intersectionBoxSourceInfo = dynamic_cast<const RivIntersectionBoxSourceInfo*>(firstHitPart->sourceInfo());

            if (rivSourceInfo)
            {
                gridIndex = rivSourceInfo->gridIndex();
                if (rivSourceInfo->hasCellFaceMapping())
                {
                    CVF_ASSERT(rivSourceInfo->m_cellFaceFromTriangleMapper.notNull());

                    cellIndex = rivSourceInfo->m_cellFaceFromTriangleMapper->cellIndex(firstPartTriangleIndex);
                    face = rivSourceInfo->m_cellFaceFromTriangleMapper->cellFace(firstPartTriangleIndex);
                }
            }
            else if (femSourceInfo)
            {
                gridIndex = femSourceInfo->femPartIndex();
                cellIndex = femSourceInfo->triangleToElmMapper()->elementIndex(firstPartTriangleIndex);
            }
            else if (crossSectionSourceInfo)
            {
                findCellAndGridIndex(crossSectionSourceInfo, firstPartTriangleIndex, &cellIndex, &gridIndex);
            }
            else if (intersectionBoxSourceInfo)
            {
                findCellAndGridIndex(intersectionBoxSourceInfo, firstPartTriangleIndex, &cellIndex, &gridIndex);
            }
        }

        if (firstNncHitPart && firstNncHitPart->sourceInfo())
        {
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(firstNncHitPart->sourceInfo());
            if (rivSourceInfo)
            {
                if (nncPartTriangleIndex < rivSourceInfo->m_NNCIndices->size())
                {
                    nncIndex = rivSourceInfo->m_NNCIndices->get(nncPartTriangleIndex);
                }
            }
        }
    }

    if (cellIndex == cvf::UNDEFINED_SIZE_T)
    {
        RiuSelectionManager::instance()->deleteAllItems();
    }
    else 
    {
        bool appendToSelection = false;
        if (keyboardModifiers & Qt::ControlModifier)
        {
            appendToSelection = true;
        }

        cvf::Color3f curveColor = RiuSelectionColors::curveColorFromTable();
        if (RiuSelectionManager::instance()->isEmpty() || !appendToSelection)
        {
            curveColor = RiuSelectionColors::singleCurveColor();
        }

        RiuSelectionItem* selItem = NULL;
        {
            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
            if (eclipseView)
            {
                selItem = new RiuEclipseSelectionItem(eclipseView, gridIndex, cellIndex, nncIndex, curveColor, face, localIntersectionPoint);
            }

            RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
            if (geomView)
            {
                selItem = new RiuGeoMechSelectionItem(geomView, gridIndex, cellIndex, curveColor, localIntersectionPoint);
            }
        }

        if (appendToSelection)
        {
            RiuSelectionManager::instance()->appendItemToSelection(selItem);
        }
        else
        {
            RiuSelectionManager::instance()->setSelectedItem(selItem);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RiuViewerCommands::lastPickPositionInDomainCoords() const
{
    return m_currentPickPositionInDomainCoords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RiuViewerCommands::currentPickedObject() const
{
    return m_currentPickedObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::findCellAndGridIndex(const RivIntersectionSourceInfo* crossSectionSourceInfo, cvf::uint firstPartTriangleIndex, size_t* cellIndex, size_t* gridIndex)
{
    CVF_ASSERT(cellIndex && gridIndex);

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
    if (eclipseView)
    {
        size_t globalCellIndex = crossSectionSourceInfo->triangleToCellIndex()[firstPartTriangleIndex];

        const RigCell& cell = eclipseView->eclipseCase()->reservoirData()->mainGrid()->globalCellArray()[globalCellIndex];
        *cellIndex = cell.gridLocalCellIndex();
        *gridIndex = cell.hostGrid()->gridIndex();
    }
    else if (geomView)
    {
        *cellIndex = crossSectionSourceInfo->triangleToCellIndex()[firstPartTriangleIndex];
        *gridIndex = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::findCellAndGridIndex(const RivIntersectionBoxSourceInfo* intersectionBoxSourceInfo, cvf::uint firstPartTriangleIndex, size_t* cellIndex, size_t* gridIndex)
{
    CVF_ASSERT(cellIndex && gridIndex);

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
    if (eclipseView)
    {
        size_t globalCellIndex = intersectionBoxSourceInfo->triangleToCellIndex()[firstPartTriangleIndex];

        const RigCell& cell = eclipseView->eclipseCase()->reservoirData()->mainGrid()->globalCellArray()[globalCellIndex];
        *cellIndex = cell.gridLocalCellIndex();
        *gridIndex = cell.hostGrid()->gridIndex();
    }
    else if (geomView)
    {
        *cellIndex = intersectionBoxSourceInfo->triangleToCellIndex()[firstPartTriangleIndex];
        *gridIndex = 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// Perform picking and return the index of the face that was hit, if a drawable geo was hit
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::extractIntersectionData(const cvf::HitItemCollection& hitItems, 
                                        cvf::Vec3d* localIntersectionPoint, cvf::Part** firstPart, uint* firstPartFaceHit, 
                                        cvf::Part** nncPart, uint* nncPartFaceHit)
{
    CVF_ASSERT(hitItems.count() > 0);

    double pickDepthThresholdSquared = 0.05 *0.05;
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());

        if (eclipseView && eclipseView->eclipseCase())
        {
            double characteristicCellSize = eclipseView->eclipseCase()->reservoirData()->mainGrid()->characteristicIJCellSize();
            pickDepthThresholdSquared = characteristicCellSize / 100.0;
            pickDepthThresholdSquared = pickDepthThresholdSquared * pickDepthThresholdSquared;
        }
    }

    const cvf::HitItem* firstNonNncHitItem = NULL;
    cvf::Vec3d firstItemIntersectionPoint = hitItems.item(0)->intersectionPoint();

    // Check if we have a close hit item with NNC data

    for (size_t i = 0; i < hitItems.count(); i++)
    {
        const cvf::HitItem* hitItemCandidate = hitItems.item(i);
        cvf::Vec3d diff = firstItemIntersectionPoint - hitItemCandidate->intersectionPoint();

        const cvf::Part* pickedPartCandidate = hitItemCandidate->part();
        bool isNncpart = false;

        if (pickedPartCandidate && pickedPartCandidate->sourceInfo())
        {
            const RivSourceInfo* rivSourceInfo = dynamic_cast<const RivSourceInfo*>(pickedPartCandidate->sourceInfo());
            if (rivSourceInfo && rivSourceInfo->hasNNCIndices())
            {

                // Hit items are ordered by distance from eye
                if (diff.lengthSquared() < pickDepthThresholdSquared)
                {
                    *nncPart = const_cast<cvf::Part*>(pickedPartCandidate);

                    const cvf::HitDetailDrawableGeo* detail = dynamic_cast<const cvf::HitDetailDrawableGeo*>(hitItemCandidate->detail());
                    if (detail && nncPartFaceHit)
                    {
                        *nncPartFaceHit = detail->faceIndex();
                    }

                    isNncpart = true;
                }
            }
        }

        if (!isNncpart && !firstNonNncHitItem)
        {
            firstNonNncHitItem = hitItemCandidate;
            firstItemIntersectionPoint = firstNonNncHitItem->intersectionPoint();
        }

        if (firstNonNncHitItem && *nncPart)
        {
            break;
        }
    }

    if (firstNonNncHitItem)
    {
        const cvf::Part* pickedPart = firstNonNncHitItem->part();
        CVF_ASSERT(pickedPart);
        *firstPart = const_cast<cvf::Part*>(pickedPart);

        const cvf::Transform* xf = pickedPart->transform();
        cvf::Vec3d globalPickedPoint = firstNonNncHitItem->intersectionPoint();

        if (localIntersectionPoint)
        {
            if (xf)
            {
                *localIntersectionPoint = globalPickedPoint.getTransformedPoint(xf->worldTransform().getInverted());
            }
            else
            {
                *localIntersectionPoint = globalPickedPoint;
            }
        }

        if (firstPartFaceHit)
        {
            const cvf::HitDetailDrawableGeo* detail = dynamic_cast<const cvf::HitDetailDrawableGeo*>(firstNonNncHitItem->detail());
            if (detail)
            {
                *firstPartFaceHit = detail->faceIndex();
            }
        }
    }
    else
    {
        if (localIntersectionPoint && nncPart && *nncPart)
        {
            cvf::Vec3d globalPickedPoint = firstItemIntersectionPoint;

            const cvf::Transform* xf = (*nncPart)->transform();
            if (xf)
            {
                *localIntersectionPoint = globalPickedPoint.getTransformedPoint(xf->worldTransform().getInverted());
            }
            else
            {
                *localIntersectionPoint = globalPickedPoint;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuViewerCommands::ijkFromCellIndex(size_t gridIdx, size_t cellIndex,  size_t* i, size_t* j, size_t* k)
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
    RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());


    if (eclipseView && eclipseView->eclipseCase())
    {
        eclipseView->eclipseCase()->reservoirData()->grid(gridIdx)->ijkFromCellIndex(cellIndex, i, j, k);
    }
    
    if (geomView && geomView->geoMechCase())
    {
        geomView->geoMechCase()->geoMechData()->femParts()->part(gridIdx)->structGrid()->ijkFromCellIndex(cellIndex, i, j, k);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuViewerCommands::handleOverlayItemPicking(int winPosX, int winPosY)
{
    cvf::OverlayItem* pickedOverlayItem = m_viewer->overlayItem(winPosX, winPosY);
    if (pickedOverlayItem)
    {
        caf::PdmObject* objToSelect = NULL;

        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_reservoirView.p());
        if (eclipseView)
        {
            if (eclipseView->cellResult()->legendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->cellResult()->legendConfig();
            }
            else if (eclipseView->cellResult()->ternaryLegendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->cellResult()->ternaryLegendConfig();
            }
            else if (eclipseView->faultResultSettings()->customFaultResult()->legendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->faultResultSettings()->customFaultResult()->legendConfig();
            }
            else if (eclipseView->faultResultSettings()->customFaultResult()->ternaryLegendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->faultResultSettings()->customFaultResult()->ternaryLegendConfig();
            }
            else if (eclipseView->cellEdgeResult()->legendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = eclipseView->cellEdgeResult()->legendConfig();
            }
        }

        RimGeoMechView* geomView = dynamic_cast<RimGeoMechView*>(m_reservoirView.p());
        if (geomView)
        {
            if (geomView->cellResult()->legendConfig()->legend() == pickedOverlayItem)
            {
                objToSelect = geomView->cellResult()->legendConfig();
            }
        }

        if (objToSelect)
        {
            RiuMainWindow::instance()->selectAsCurrentItem(objToSelect);

            return true;
        }
    }

    return false;
}
