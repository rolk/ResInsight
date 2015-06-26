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
#include "RivReservoirPartMgr.h"
#include "cvfTransform.h"
#include "cafFixedArray.h"
#include "cvfArray.h"
#include "cafPdmObject.h"
#include "RivCellSetEnum.h"

class RimEclipseView;
class RigGridBase;
class RimCellRangeFilterCollection;
class RimEclipsePropertyFilterCollection;
class RigActiveCellInfo;

class RivReservoirViewPartMgr: public cvf::Object
{
public:
    RivReservoirViewPartMgr(RimEclipseView * resv);

    cvf::Transform*             scaleTransform() { return m_scaleTransform.p();}

    void                        clearGeometryCache();
    void                        scheduleGeometryRegen(RivCellSetEnum geometryType);
    cvf::cref<cvf::UByteArray>  cellVisibility(RivCellSetEnum geometryType, size_t gridIndex, size_t frameIndex) const;
   
    void                        appendStaticGeometryPartsToModel (cvf::ModelBasicList* model, RivCellSetEnum geometryType, const std::vector<size_t>& gridIndices);
    void                        appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType, size_t frameIndex, const std::vector<size_t>& gridIndices);

    void                        updateCellColor          (RivCellSetEnum geometryType, cvf::Color4f color);
    void                        updateCellColor          (RivCellSetEnum geometryType, size_t timeStepIndex, 
                                                          cvf::Color4f color);
    void                        updateCellResultColor    (RivCellSetEnum geometryType, size_t timeStepIndex, 
                                                          RimEclipseCellColors* cellResultSlot);
    void                        updateCellEdgeResultColor(RivCellSetEnum geometryType, size_t timeStepIndex, 
														  RimEclipseCellColors* cellResultSlot,
														  RimCellEdgeColors* cellEdgeResultSlot);

    // Faults
    void                        appendFaultsStaticGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType);
    void                        appendFaultsDynamicGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType, size_t frameIndex);
    void                        updateFaultColors(RivCellSetEnum geometryType, size_t timeStepIndex, RimEclipseCellColors* cellResultSlot);
	void                        updateFaultCellEdgeResultColor(	RivCellSetEnum geometryType, size_t timeStepIndex,
															RimEclipseCellColors* cellResultSlot,
															RimCellEdgeColors* cellEdgeResultSlot);

    // Fault labels
    RivCellSetEnum  geometryTypeForFaultLabels(const std::vector<RivCellSetEnum>& geometryTypes) const;
    void                        appendFaultLabelsStaticGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType);
    void                        appendFaultLabelsDynamicGeometryPartsToModel(cvf::ModelBasicList* model, RivCellSetEnum geometryType, size_t frameIndex);

    void                        setFaultForceVisibilityForGeometryType(RivCellSetEnum geometryType, bool forceVisibility);

private:
    void                        createGeometry(RivCellSetEnum geometryType);
    void                        computeVisibility(cvf::UByteArray* cellVisibility, RivCellSetEnum geometryType, RigGridBase* grid, size_t gridIdx);

    void                        createPropertyFilteredNoneWellCellGeometry(size_t frameIndex);
    void                        createPropertyFilteredWellGeometry(size_t frameIndex);

    void                        clearGeometryCache(RivCellSetEnum geomType);


    static void                 computeNativeVisibility  (cvf::UByteArray* cellVisibilities, const RigGridBase* grid, const RigActiveCellInfo* activeCellInfo, const cvf::UByteArray* cellIsInWellStatuses,  bool invalidCellsIsVisible, bool inactiveCellsIsVisible, bool activeCellsIsVisible, bool mainGridIsVisible);
    void                        computeRangeVisibility   (RivCellSetEnum geometryType, cvf::UByteArray* cellVisibilities, const RigGridBase* grid, const cvf::UByteArray* nativeVisibility, const RimCellRangeFilterCollection* rangeFilterColl);
    static void                 computePropertyVisibility(cvf::UByteArray* cellVisibilities, const RigGridBase* grid, size_t timeStepIndex, const cvf::UByteArray* rangeFilterVisibility, RimEclipsePropertyFilterCollection* propFilterColl);
    static void                 copyByteArray(cvf::UByteArray* cellVisibilities, const cvf::UByteArray* cellIsWellStatuses );

    RivReservoirPartMgr *       reservoirPartManager(RivCellSetEnum geometryType, size_t timeStepIndex );


private:
    caf::FixedArray<RivReservoirPartMgr, PROPERTY_FILTERED> m_geometries;
    caf::FixedArray<bool, PROPERTY_FILTERED>                m_geometriesNeedsRegen;

    cvf::Collection<RivReservoirPartMgr>                    m_propFilteredGeometryFrames;
    std::vector<uchar>                                      m_propFilteredGeometryFramesNeedsRegen;

    cvf::Collection<RivReservoirPartMgr>                    m_propFilteredWellGeometryFrames;
    std::vector<uchar>                                      m_propFilteredWellGeometryFramesNeedsRegen;

    cvf::ref<cvf::Transform>                                m_scaleTransform;
    caf::PdmPointer<RimEclipseView>                       m_reservoirView;

};
