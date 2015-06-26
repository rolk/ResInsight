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

#pragma once

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfObject.h"

// Includes to make Pdm work for cvf::Color and cvf:Mat
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmFieldCvfMat4d.h"

#include "RivReservoirViewPartMgr.h"
#include "RimView.h"

class RigActiveCellInfo;
class RigCaseCellResultsData;
class RigGridBase;
class RigGridCellFaceVisibilityFilter;
class Rim3dOverlayInfoConfig;
class RimEclipseCase;
class RimCellEdgeResultSlot;
class RimEclipsePropertyFilter;
class RimEclipsePropertyFilterCollection;
class RimCellRangeFilter;
class RimCellRangeFilterCollection;
class RimFaultCollection;
class RimFaultResultSlot;
class RimReservoirCellResultsStorage;
class RimReservoirCellResultsStorage;
class RimEclipseCellColors;
class RimWellCollection;
class RiuViewer;
class RivReservoirPipesPartMgr;

namespace cvf
{
    class Transform;
    class ScalarMapperUniformLevels;
    class ModelBasicList;
    class OverlayItem;
}

enum PartRenderMaskEnum
{
    surfaceBit      = 0x00000001,
    meshSurfaceBit  = 0x00000002,
    faultBit        = 0x00000004,
    meshFaultBit    = 0x00000008,
};

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseView : public RimView
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipseView(void);
    virtual ~RimEclipseView(void);

    // Fields containing child objects :

    caf::PdmField<RimEclipseCellColors*>                   cellResult;
    caf::PdmField<RimCellEdgeResultSlot*>           cellEdgeResult;
    caf::PdmField<RimFaultResultSlot*>              faultResultSettings;

    caf::PdmField<RimEclipsePropertyFilterCollection*> propertyFilterCollection;

    caf::PdmField<RimWellCollection*>               wellCollection;
    caf::PdmField<RimFaultCollection*>              faultCollection;

    // Fields

    caf::PdmField<bool>                             showInvalidCells;
    caf::PdmField<bool>                             showInactiveCells;
    caf::PdmField<bool>                             showMainGrid;

    // Access internal objects

    RimReservoirCellResultsStorage*                 currentGridCellResults();
    RigActiveCellInfo*                              currentActiveCellInfo();
    RimEclipseCellColors*                                  currentFaultResultSlot();

    void                                            setEclipseCase(RimEclipseCase* reservoir);
    RimEclipseCase*                                 eclipseCase();

    // Display model generation

    virtual void                                    loadDataAndUpdate();
    bool                                            isTimeStepDependentDataVisible() const;

    virtual void                                    scheduleGeometryRegen(RivCellSetEnum geometryType);
    void                                            scheduleReservoirGridGeometryRegen();
    void                                            schedulePipeGeometryRegen();
    void                                            updateDisplayModelForWellResults();

    const std::vector<RivCellSetEnum>&              visibleGridParts() const { return m_visibleGridParts;}
    cvf::cref<RivReservoirViewPartMgr>              reservoirGridPartManager() const { return m_reservoirGridPartManager.p(); }

    // Does this belong here, really ?
    void                                            calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid);

    // Overridden PDM methods:
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
protected:
    virtual void                                    initAfterRead();
    virtual void                                    defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

private:
    void                                            createDisplayModel();
    void                                            updateDisplayModelVisibility();
    virtual void                                    updateCurrentTimeStep();

    void                                            indicesToVisibleGrids(std::vector<size_t>* gridIndices);
    virtual void                                    updateScaleTransform();
    virtual cvf::Transform*                         scaleTransform();

    virtual void                                    updateStaticCellColors();
    void                                            updateStaticCellColors(RivCellSetEnum geometryType);
    void                                            updateLegends();
    void                                            updateMinMaxValuesAndAddLegendToView(QString legendLabel, RimEclipseCellColors* resultSlot, RigCaseCellResultsData* cellResultsData);
    virtual void                                    resetLegendsInViewer();
    virtual void                                    updateViewerWidgetWindowTitle();

    std::vector<RivCellSetEnum>                     visibleFaultGeometryTypes() const;
    void                                            updateFaultForcedVisibility();
    void                                            updateFaultColors();

    void                                            syncronizeWellsWithResults();
    void                                            addWellPathsToScene(cvf::Scene* scene,
                                                                        const cvf::Vec3d& displayModelOffset, 
                                                                        double characteristicCellSize, 
                                                                        const cvf::BoundingBox& wellPathClipBoundingBox, 
                                                                        cvf::Transform* scaleTransform);

    void                                            clampCurrentTimestep();

    virtual RimCase*                                ownerCase();

    caf::PdmPointer<RimEclipseCase>                 m_reservoir;

    cvf::ref<RivReservoirViewPartMgr>               m_reservoirGridPartManager;
    cvf::ref<RivReservoirPipesPartMgr>              m_pipesPartManager;

    std::vector<RivCellSetEnum>                     m_visibleGridParts;
};

