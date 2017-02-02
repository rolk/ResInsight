/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseWellCollection.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigEclipseCaseData.h"
#include "RigSingleWellResultsData.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"

#include "RiuMainWindow.h"

#include "RivReservoirViewPartMgr.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiCheckBoxTristateEditor.h"


namespace caf
{
    // OBSOLETE enum
    template<>
    void RimEclipseWellCollection::WellVisibilityEnum::setUp()
    {
        addItem(RimEclipseWellCollection::PIPES_FORCE_ALL_OFF,       "FORCE_ALL_OFF",      "All Off");
        addItem(RimEclipseWellCollection::PIPES_INDIVIDUALLY,        "ALL_ON",             "Individual");
        addItem(RimEclipseWellCollection::PIPES_OPEN_IN_VISIBLE_CELLS,"OPEN_IN_VISIBLE_CELLS", "Visible cells filtered");
        addItem(RimEclipseWellCollection::PIPES_FORCE_ALL_ON,        "FORCE_ALL_ON",       "All On");
    }
}


namespace caf
{
    // OBSOLETE enum
    template<>
    void RimEclipseWellCollection::WellCellsRangeFilterEnum::setUp()
    {
        addItem(RimEclipseWellCollection::RANGE_ADD_NONE,       "FORCE_ALL_OFF",      "All Off");
        addItem(RimEclipseWellCollection::RANGE_ADD_INDIVIDUAL, "ALL_ON",             "Individually");
        addItem(RimEclipseWellCollection::RANGE_ADD_ALL,        "FORCE_ALL_ON",       "All On");
    }
}

namespace caf
{
    template<>
    void RimEclipseWellCollection::WellFenceEnum::setUp()
    {
        addItem(RimEclipseWellCollection::K_DIRECTION, "K_DIRECTION",    "K - Direction");
        addItem(RimEclipseWellCollection::J_DIRECTION, "J_DIRECTION",    "J - Direction");
        addItem(RimEclipseWellCollection::I_DIRECTION, "I_DIRECTION",    "I - Direction");
        setDefault(RimEclipseWellCollection::K_DIRECTION);
    }
}

namespace caf
{
    template<>
    void RimEclipseWellCollection::WellHeadPositionEnum::setUp()
    {
        addItem(RimEclipseWellCollection::WELLHEAD_POS_ACTIVE_CELLS_BB,    "WELLHEAD_POS_ACTIVE_CELLS_BB", "All Active Cells");
        addItem(RimEclipseWellCollection::WELLHEAD_POS_TOP_COLUMN,         "WELLHEAD_POS_TOP_COLUMN",      "Active Cell Column");
        setDefault(RimEclipseWellCollection::WELLHEAD_POS_TOP_COLUMN);
    }
}

namespace caf
{
    template<>
    void RimEclipseWellCollection::WellPipeCoordEnum::setUp()
    {
        addItem(RimEclipseWellCollection::WELLPIPE_INTERPOLATED,    "WELLPIPE_INTERPOLATED",    "Interpolated");
        addItem(RimEclipseWellCollection::WELLPIPE_CELLCENTER,      "WELLPIPE_CELLCENTER",      "Cell Centers");
        setDefault(RimEclipseWellCollection::WELLPIPE_INTERPOLATED);
    }
}

CAF_PDM_SOURCE_INIT(RimEclipseWellCollection, "Wells");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWellCollection::RimEclipseWellCollection()
{
    CAF_PDM_InitObject("Simulation Wells", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&isActive,              "Active",        true,   "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&showWellsIntersectingVisibleCells, "ShowWellsIntersectingVisibleCells", true, "Show Wells Intersecting Visible Cells", "", "", "");

    // Appearance
    CAF_PDM_InitFieldNoDefault(&m_showWellHead,        "ShowWellHeadTristate",      "Show Well Head", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showWellLabel,       "ShowWellLabelTristate",     "Show Well Label", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showWellPipe,        "ShowWellPipe",              "Show Well Pipe", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_showWellSpheres,     "ShowWellSpheres",           "Show Well Spheres", "", "", "");

    m_showWellHead.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellHead.xmlCapability()->setIOReadable(false);
    m_showWellHead.xmlCapability()->setIOWritable(false);

    m_showWellLabel.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellLabel.xmlCapability()->setIOReadable(false);
    m_showWellLabel.xmlCapability()->setIOWritable(false);

    m_showWellPipe.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellPipe.xmlCapability()->setIOReadable(false);
    m_showWellPipe.xmlCapability()->setIOWritable(false);

    m_showWellSpheres.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellSpheres.xmlCapability()->setIOReadable(false);
    m_showWellSpheres.xmlCapability()->setIOWritable(false);

    // Scaling
    CAF_PDM_InitField(&wellHeadScaleFactor, "WellHeadScale",            1.0,    "Well Head Scale Factor", "", "", "");
    CAF_PDM_InitField(&pipeScaleFactor,     "WellPipeRadiusScale",      0.1,    "Well Pipe Scale Factor", "", "", "");
    CAF_PDM_InitField(&spheresScaleFactor,  "CellCenterSphereScale",    0.2,    "Well Sphere Scale Factor", "", "", "");

    // Color
    cvf::Color3f defWellLabelColor = RiaApplication::instance()->preferences()->defaultWellLabelColor();
    CAF_PDM_InitField(&wellLabelColor,      "WellLabelColor",   defWellLabelColor, "Well Label Color",  "", "", "");

    CAF_PDM_InitField(&showConnectionStatusColors, "ShowConnectionStatusColors", true, "Show Connection Status Colors Along Well", "", "", "");

    cvf::Color3f defaultApplyColor = cvf::Color3f::YELLOW;
    CAF_PDM_InitField(&m_wellColorForApply, "WellColorForApply", defaultApplyColor, "Single Well Color", "", "", "");

    CAF_PDM_InitField(&m_applySingleColorToWells, "ApplySingleColorToWells", false, "", "", "", "");
    m_applySingleColorToWells.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applySingleColorToWells.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_applySingleColorToWells.xmlCapability()->setIOReadable(false);
    m_applySingleColorToWells.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_applyIndividualColorsToWells, "ApplyIndividualColorsToWells", false, "", "", "", "");
    m_applyIndividualColorsToWells.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applyIndividualColorsToWells.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_applyIndividualColorsToWells.xmlCapability()->setIOReadable(false);
    m_applyIndividualColorsToWells.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&pipeCrossSectionVertexCount, "WellPipeVertexCount", 12, "Pipe vertex count", "", "", "");
    pipeCrossSectionVertexCount.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&wellPipeCoordType,           "WellPipeCoordType", WellPipeCoordEnum(WELLPIPE_INTERPOLATED), "Type", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_showWellCells,       "ShowWellCellsTristate", "Show Well Cells", "", "", "");
    m_showWellCells.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellCells.xmlCapability()->setIOReadable(false);
    m_showWellCells.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&wellCellFenceType,   "DefaultWellFenceDirection", WellFenceEnum(K_DIRECTION), "Well Fence Direction", "", "", "");

    CAF_PDM_InitField(&wellCellTransparencyLevel, "WellCellTransparency", 0.5, "Well Cell Transparency", "", "", "");
    CAF_PDM_InitField(&isAutoDetectingBranches, "IsAutoDetectingBranches", true, "Branch Detection", "", "Toggle wether the well pipe visualization will try to detect when a part of the well \nis really a branch, and thus is starting from wellhead", "");
    CAF_PDM_InitField(&wellHeadPosition,    "WellHeadPosition", WellHeadPositionEnum(WELLHEAD_POS_TOP_COLUMN), "Well Head Position On Top Of",  "", "", "");

    CAF_PDM_InitFieldNoDefault(&wells, "Wells", "Wells",  "", "", "");
    wells.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_showWellCellFence, "ShowWellCellFenceTristate", "Show Well Cell Fence", "", "", "");
    m_showWellCellFence.uiCapability()->setUiEditorTypeName(caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName());
    m_showWellCellFence.xmlCapability()->setIOReadable(false);
    m_showWellCellFence.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&obsoleteField_wellPipeVisibility,  "GlobalWellPipeVisibility", WellVisibilityEnum(PIPES_INDIVIDUALLY), "Global well pipe visibility",  "", "", "");
    obsoleteField_wellPipeVisibility.uiCapability()->setUiHidden(true);
    obsoleteField_wellPipeVisibility.xmlCapability()->setIOWritable(false);
    
    CAF_PDM_InitField(&obsoleteField_wellCellsToRangeFilterMode,  "GlobalWellCellVisibility", WellCellsRangeFilterEnum(RANGE_ADD_INDIVIDUAL),  "Add cells to range filter", "", "", "");
    obsoleteField_wellCellsToRangeFilterMode.uiCapability()->setUiHidden(true);
    obsoleteField_wellCellsToRangeFilterMode.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&obsoleteField_showWellHead,              "ShowWellHead",     true, "Show Well Head", "", "", "");
    CAF_PDM_InitField(&obsoleteField_showWellLabel,             "ShowWellLabel",    true, "Show Well Label", "", "", "");
    CAF_PDM_InitField(&obsoleteField_showWellCellFence,         "ShowWellFences",   false,  "Show Well Cell Fence", "", "", "");

    obsoleteField_showWellHead.uiCapability()->setUiHidden(true);
    obsoleteField_showWellLabel.uiCapability()->setUiHidden(true);
    obsoleteField_showWellCellFence.uiCapability()->setUiHidden(true);

    obsoleteField_showWellHead.xmlCapability()->setIOWritable(false);
    obsoleteField_showWellLabel.xmlCapability()->setIOWritable(false);
    obsoleteField_showWellCellFence.xmlCapability()->setIOWritable(false);

    m_reservoirView = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWellCollection::~RimEclipseWellCollection()
{
   wells.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::setShowWellCellsState(bool enable)
{
    for (RimEclipseWell* w : wells)
    {
        w->showWellCells = enable;
    }

    updateConnectedEditors();

    if (m_reservoirView)
    {
        m_reservoirView->scheduleGeometryRegen(VISIBLE_WELL_CELLS);
        m_reservoirView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseWellCollection::showWellCells()
{
    if (m_showWellCells().isFalse())
    {
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseWell* RimEclipseWellCollection::findWell(QString name)
{
    for (size_t i = 0; i < this->wells().size(); ++i)
    {
        if (this->wells()[i]->name() == name)
        {
            return this->wells()[i];
        }
    }
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseWellCollection::hasVisibleWellCells()
{
    if (!this->isActive()) return false;
    if (this->wells().size() == 0 ) return false;

    bool hasCells = false;
    for (size_t i = 0 ; !hasCells && i < this->wells().size(); ++i)
    {
        RimEclipseWell* well = this->wells()[i];
        if ( well && well->wellResults() && ((well->showWell() && well->showWellCells())) )
        {
            for (size_t tIdx = 0; !hasCells &&  tIdx < well->wellResults()->m_wellCellsTimeSteps.size(); ++tIdx )
            {
                const RigWellResultFrame& wellResultFrame = well->wellResults()->m_wellCellsTimeSteps[tIdx];
                for (size_t wsIdx = 0; !hasCells &&  wsIdx < wellResultFrame.m_wellResultBranches.size(); ++wsIdx)
                {
                    if (wellResultFrame.m_wellResultBranches[wsIdx].m_branchResultPoints.size() > 0  ) hasCells = true; 
                }
            }
        }
    }

    if (!hasCells) return false;

    // Todo: Handle range filter intersection

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Used to know if we need animation of time steps due to the wells
//--------------------------------------------------------------------------------------------------
bool RimEclipseWellCollection::hasVisibleWellPipes() 
{
    if (!this->isActive()) return false;
    if (this->wells().size() == 0 ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&isActive == changedField)
    {
        this->updateUiIconFromToggleField();
    }

    if (&m_showWellLabel == changedField)
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellLabel = !(m_showWellLabel().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellHead == changedField)
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellHead = !(m_showWellHead().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellPipe == changedField)
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellPipe = !(m_showWellPipe().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellSpheres == changedField)
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellSpheres = !(m_showWellSpheres().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellCells == changedField)
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellCells = !(m_showWellCells().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (&m_showWellCellFence == changedField)
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellCellFence = !(m_showWellCellFence().isFalse());
            w->updateConnectedEditors();
        }
    }

    if (m_reservoirView)
    {
        if (   &isActive == changedField
            || &m_showWellLabel == changedField
            || &m_showWellCells == changedField
            || &m_showWellCellFence == changedField
            || &wellCellFenceType == changedField)
        {
            m_reservoirView->scheduleGeometryRegen(VISIBLE_WELL_CELLS);
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (&wellCellTransparencyLevel == changedField)
        {
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (  &spheresScaleFactor == changedField
                || &m_showWellSpheres == changedField
                || &showConnectionStatusColors == changedField)
        {
            m_reservoirView->schedulePipeGeometryRegen();
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
        else if (  &pipeCrossSectionVertexCount == changedField 
                || &pipeScaleFactor == changedField 
                || &wellHeadScaleFactor == changedField 
                || &m_showWellHead == changedField
                || &isAutoDetectingBranches == changedField
                || &wellHeadPosition == changedField
                || &wellLabelColor == changedField
                || &showWellsIntersectingVisibleCells == changedField
                || &wellPipeCoordType == changedField
                || &m_showWellPipe == changedField)
        {
            m_reservoirView->schedulePipeGeometryRegen();
            m_reservoirView->scheduleCreateDisplayModelAndRedraw();
        }
    }

    if (&m_applyIndividualColorsToWells == changedField)
    {
        assignDefaultWellColors();

        if (m_reservoirView) m_reservoirView->scheduleCreateDisplayModelAndRedraw();

        m_applyIndividualColorsToWells = false;
    }

    if (&m_applySingleColorToWells == changedField)
    {
        cvf::Color3f col = m_wellColorForApply();

        for (size_t i = 0; i < wells.size(); i++)
        {
            wells[i]->wellPipeColor = col;
            wells[i]->updateConnectedEditors();
        }

        if (m_reservoirView) m_reservoirView->scheduleCreateDisplayModelAndRedraw();

        m_applySingleColorToWells = false;
    }

    if (&m_showWellCells == changedField)
    {
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::assignDefaultWellColors()
{
    // The wells are sorted, use ordering of single well results data to assign colors

    RimEclipseCase* rimEclipseCase = nullptr;
    this->firstAncestorOrThisOfType(rimEclipseCase);
    if (!rimEclipseCase) return;

    if (!rimEclipseCase->reservoirData()) return;

    cvf::Collection<RigSingleWellResultsData> wellResults = rimEclipseCase->reservoirData()->wellResults();

    for (size_t wIdx = 0; wIdx < wellResults.size(); ++wIdx)
    {
        RimEclipseWell* well = this->findWell(wellResults[wIdx]->m_wellName);
        if (well)
        {
            cvf::Color3f col = cycledPaletteColor(wIdx);

            well->wellPipeColor = col;
            well->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::setReservoirView(RimEclipseView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    updateStateForVisibilityCheckboxes();

    uiOrdering.add(&showWellsIntersectingVisibleCells);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Visibility");
    appearanceGroup->add(&m_showWellLabel);
    appearanceGroup->add(&m_showWellHead);
    appearanceGroup->add(&m_showWellPipe);
    appearanceGroup->add(&m_showWellSpheres);

    caf::PdmUiGroup* sizeScalingGroup = uiOrdering.addNewGroup("Size Scaling");
    sizeScalingGroup->add(&wellHeadScaleFactor);
    sizeScalingGroup->add(&pipeScaleFactor);
    sizeScalingGroup->add(&spheresScaleFactor);

    caf::PdmUiGroup* colorGroup = uiOrdering.addNewGroup("Color");
    colorGroup->add(&wellLabelColor);
    colorGroup->add(&m_applyIndividualColorsToWells);

    colorGroup->add(&m_wellColorForApply);
    colorGroup->add(&m_applySingleColorToWells);

    colorGroup->add(&showConnectionStatusColors);

    caf::PdmUiGroup* wellPipeGroup = uiOrdering.addNewGroup("Well Pipe Geometry");
    wellPipeGroup->add(&wellPipeCoordType);
    wellPipeGroup->add(&isAutoDetectingBranches);

    caf::PdmUiGroup* advancedGroup = uiOrdering.addNewGroup("Advanced");
    advancedGroup->add(&wellCellTransparencyLevel);
    advancedGroup->add(&wellHeadPosition);

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup("Well Cells");
    filterGroup->add(&obsoleteField_wellCellsToRangeFilterMode);
    filterGroup->add(&m_showWellCells);
    filterGroup->add(&m_showWellCellFence);
    filterGroup->add(&wellCellFenceType);

    m_showWellCellFence.uiCapability()->setUiReadOnly(!showWellCells());
    wellCellFenceType.uiCapability()->setUiReadOnly(!showWellCells());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::updateStateForVisibilityCheckboxes()
{
    size_t showLabelCount = 0;
    size_t showWellHeadCount = 0;
    size_t showPipeCount = 0;
    size_t showSphereCount = 0;
    size_t showWellCellsCount = 0;
    size_t showWellCellFenceCount = 0;

    for (RimEclipseWell* w : wells)
    {
        if (w->showWellLabel())     showLabelCount++;
        if (w->showWellHead())      showWellHeadCount++;
        if (w->showWellPipe())      showPipeCount++;
        if (w->showWellSpheres())   showSphereCount++;
        if (w->showWellCells())     showWellCellsCount++;
        if (w->showWellCellFence()) showWellCellFenceCount++;
    }

    updateStateFromEnabledChildCount(showLabelCount, &m_showWellLabel);
    updateStateFromEnabledChildCount(showWellHeadCount, &m_showWellHead);
    updateStateFromEnabledChildCount(showPipeCount, &m_showWellPipe);
    updateStateFromEnabledChildCount(showSphereCount, &m_showWellSpheres);
    updateStateFromEnabledChildCount(showWellCellsCount, &m_showWellCells);
    updateStateFromEnabledChildCount(showWellCellFenceCount, &m_showWellCellFence);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::updateStateFromEnabledChildCount(size_t enabledChildCount, caf::PdmField<caf::Tristate>* fieldToUpdate)
{
    caf::Tristate tristate;

    if (enabledChildCount == 0)
    {
        tristate = caf::Tristate::State::False;
    }
    else if (enabledChildCount == wells.size())
    {
        tristate = caf::Tristate::State::True;
    }
    else
    {
        tristate = caf::Tristate::State::PartiallyTrue;
    }

    fieldToUpdate->setValue(tristate);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEclipseWellCollection::objectToggleField()
{
    return &isActive;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::initAfterRead()
{
    if (obsoleteField_wellPipeVisibility() == PIPES_OPEN_IN_VISIBLE_CELLS)
    {
        showWellsIntersectingVisibleCells = true;
    }
    else if (obsoleteField_wellPipeVisibility() == PIPES_FORCE_ALL_OFF)
    {
        showWellsIntersectingVisibleCells = false;

        for (RimEclipseWell* w : wells)
        {
            w->showWellPipe = false;
        }
    }
    else if (obsoleteField_wellPipeVisibility() == PIPES_FORCE_ALL_ON)
    {
        showWellsIntersectingVisibleCells = false;

        for (RimEclipseWell* w : wells)
        {
            w->showWellPipe = true;
        }
    }

    if (obsoleteField_wellCellsToRangeFilterMode() == RANGE_ADD_NONE)
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellCells = false;
        }
    }
    else if (obsoleteField_wellCellsToRangeFilterMode() == RANGE_ADD_ALL)
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellCells = true;
        }
    }

    if (!obsoleteField_showWellLabel())
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellLabel = false;
        }
    }

    if (!obsoleteField_showWellHead())
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellHead = false;
        }
    }

    if (obsoleteField_showWellCellFence())
    {
        for (RimEclipseWell* w : wells)
        {
            w->showWellCellFence = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (&m_applyIndividualColorsToWells == field)
    {
        caf::PdmUiPushButtonEditorAttribute* editorAttr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (editorAttr)
        {
            editorAttr->m_buttonText = "Apply Individual Well Colors";
        }
    }

    if (&m_applySingleColorToWells == field)
    {
        caf::PdmUiPushButtonEditorAttribute* editorAttr = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (editorAttr)
        {
            QColor col;
            col.setRgbF(m_wellColorForApply().r(), m_wellColorForApply().g(), m_wellColorForApply().b());

            QPixmap pixmap(100, 100);
            pixmap.fill(col);

            QIcon colorIcon(pixmap);

            editorAttr->m_buttonIcon = colorIcon;
            editorAttr->m_buttonText = "Apply Single Well Color";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::ubyte>& RimEclipseWellCollection::resultWellGeometryVisibilities(size_t frameIndex)
{
    calculateWellGeometryVisibility(frameIndex);
    return m_framesOfResultWellPipeVisibilities[frameIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::scheduleIsWellPipesVisibleRecalculation()
{
    m_framesOfResultWellPipeVisibilities.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::calculateWellGeometryVisibility(size_t frameIndex)
{
    if (m_framesOfResultWellPipeVisibilities.size() > frameIndex && m_framesOfResultWellPipeVisibilities[frameIndex].size()) return;

    if (m_framesOfResultWellPipeVisibilities.size() <= frameIndex)
        m_framesOfResultWellPipeVisibilities.resize(frameIndex+1);

    if (m_framesOfResultWellPipeVisibilities[frameIndex].size() <= wells().size())
        m_framesOfResultWellPipeVisibilities[frameIndex].resize(wells().size(), false); 
    
    for (const RimEclipseWell* well : wells())
    {
        bool wellPipeVisible = well->isWellPipeVisible(frameIndex);
        bool wellSphereVisible = well->isWellSpheresVisible(frameIndex);

        m_framesOfResultWellPipeVisibilities[frameIndex][well->resultWellIndex()] = wellPipeVisible || wellSphereVisible;
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: Consider creating a factory for colors, see also RimSummaryCurveAppearanceCalculator
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimEclipseWellCollection::cycledPaletteColor(size_t colorIndex)
{
    static const size_t colorCount = 15;
    static const cvf::ubyte colorData[][3] =
    {
      {  0,  112, 136 }, // Dark Green-Blue
      { 202,   0,   0 }, // Red
      { 78,  204,   0 }, // Clear Green
      { 236, 118,   0 }, // Orange
      {  0 ,   0,   0 }, // Black
      { 56,   56, 255 }, // Vivid Blue
      { 248,   0, 170 }, // Magenta
      { 169,   2, 240 }, // Purple
      { 0,   221, 221 }, // Turquoise
      { 201, 168, 206 }, // Light Violet
      { 0,   205,  68 }, // Bluish Green
      { 236, 188,   0 }, // Mid Yellow
      { 51,  204, 255 },  // Bluer Turquoise
      { 164, 193,   0 }, // Mid Yellowish Green
      { 0,   143, 239 }, // Dark Light Blue
    };

    size_t paletteIdx = colorIndex % colorCount;

    cvf::Color3ub ubColor(colorData[paletteIdx][0], colorData[paletteIdx][1], colorData[paletteIdx][2]);
    cvf::Color3f cvfColor(ubColor);

    return cvfColor;
}

bool lessEclipseWell(const caf::PdmPointer<RimEclipseWell>& w1,  const caf::PdmPointer<RimEclipseWell>& w2)
{
    if (w1.notNull() && w2.notNull())
        return (w1->name() < w2->name());
    else if (w1.notNull())
        return true;
    else
        return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseWellCollection::sortWellsByName()
{
   std::sort(wells.begin(), wells.end(), lessEclipseWell);
}
