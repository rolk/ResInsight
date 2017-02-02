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

#include "RivSimWellPipesPartMgr.h"

#include "RigEclipseCaseData.h"
#include "RigCell.h"
#include "RigMainGrid.h"
#include "RigSimulationWellCenterLineCalculator.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimReservoirCellResultsStorage.h"

#include "RivPipeGeometryGenerator.h"
#include "RivWellPathSourceInfo.h"
#include "RivSimWellPipeSourceInfo.h"

#include "cafEffectGenerator.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRay.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfTransform.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivSimWellPipesPartMgr::RivSimWellPipesPartMgr(RimEclipseView* reservoirView, RimEclipseWell* well)
{
    m_rimReservoirView = reservoirView;
    m_rimWell      = well;
    m_needsTransformUpdate = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivSimWellPipesPartMgr::~RivSimWellPipesPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::buildWellPipeParts()
{
    if (m_rimReservoirView.isNull()) return;

    m_wellBranches.clear();

    m_pipeBranchesCLCoords.clear();
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    m_rimWell->calculateWellPipeStaticCenterLine(m_pipeBranchesCLCoords, pipeBranchesCellIds);

    double characteristicCellSize = m_rimReservoirView->mainGrid()->characteristicIJCellSize();
    double pipeRadius = m_rimReservoirView->wellCollection()->pipeScaleFactor() * m_rimWell->pipeScaleFactor() * characteristicCellSize;


    for (size_t brIdx = 0; brIdx < pipeBranchesCellIds.size(); ++brIdx)
    {
        cvf::ref<RivSimWellPipeSourceInfo> sourceInfo = new RivSimWellPipeSourceInfo(m_rimWell, brIdx);

        m_wellBranches.push_back(RivPipeBranchData());
        RivPipeBranchData& pbd = m_wellBranches.back();

        pbd.m_cellIds = pipeBranchesCellIds[brIdx];

        pbd.m_pipeGeomGenerator = new RivPipeGeometryGenerator;

        pbd.m_pipeGeomGenerator->setRadius(pipeRadius);
        pbd.m_pipeGeomGenerator->setCrossSectionVertexCount(m_rimReservoirView->wellCollection()->pipeCrossSectionVertexCount());
        pbd.m_pipeGeomGenerator->setPipeColor( m_rimWell->wellPipeColor());

        cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray;
        cvfCoords->assign(m_pipeBranchesCLCoords[brIdx]);
        
        // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.
        const RigMainGrid* mainGrid = m_rimReservoirView->mainGrid();

        for (size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx)
        {
            cvf::Vec4d transfCoord = m_scaleTransform->worldTransform()* cvf::Vec4d((*cvfCoords)[cIdx] - mainGrid->displayModelOffset(), 1);
            (*cvfCoords)[cIdx][0] = transfCoord[0];
            (*cvfCoords)[cIdx][1] = transfCoord[1];
            (*cvfCoords)[cIdx][2] = transfCoord[2];
        }

        pbd.m_pipeGeomGenerator->setPipeCenterCoords(cvfCoords.p());
        pbd.m_surfaceDrawable = pbd.m_pipeGeomGenerator->createPipeSurface();
        pbd.m_centerLineDrawable = pbd.m_pipeGeomGenerator->createCenterLine();

        if (pbd.m_surfaceDrawable.notNull())
        {
            pbd.m_surfacePart = new cvf::Part;
            pbd.m_surfacePart->setDrawable(pbd.m_surfaceDrawable.p());

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(m_rimWell->wellPipeColor()), caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            pbd.m_surfacePart->setEffect(eff.p());
            
            pbd.m_surfacePart->setSourceInfo(sourceInfo.p());
        }

        if (pbd.m_centerLineDrawable.notNull())
        {
            pbd.m_centerLinePart = new cvf::Part;
            pbd.m_centerLinePart->setDrawable(pbd.m_centerLineDrawable.p());

            caf::MeshEffectGenerator gen(m_rimWell->wellPipeColor());
            cvf::ref<cvf::Effect> eff = gen.generateCachedEffect();

            pbd.m_centerLinePart->setEffect(eff.p());
        }
    }

    m_needsTransformUpdate = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivSimWellPipesPartMgr::RivPipeBranchData* RivSimWellPipesPartMgr::pipeBranchData(size_t branchIndex)
{
    if (branchIndex < m_wellBranches.size())
    {
        size_t i = 0;

        auto brIt = m_wellBranches.begin();
        while (i < branchIndex)
        {
            brIt++;
        }

        return &(*brIt);
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (m_rimReservoirView.isNull()) return;
    if (m_rimWell.isNull()) return;
    if (!m_rimWell->isWellPipeVisible(frameIndex)) return;

    if (m_needsTransformUpdate) buildWellPipeParts();

    std::list<RivPipeBranchData>::iterator it;
    for (it = m_wellBranches.begin(); it != m_wellBranches.end(); ++it)
    {
        if (it->m_surfacePart.notNull())
        {
            model->addPart(it->m_surfacePart.p());
        }
        if (it->m_centerLinePart.notNull())
        {
            model->addPart(it->m_centerLinePart.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::updatePipeResultColor(size_t frameIndex)
{
    if (m_rimWell == NULL) return;

    RigSingleWellResultsData* wRes = m_rimWell->wellResults();
    if (wRes == NULL) return;

    if (!wRes->hasWellResult(frameIndex)) return; // Or reset colors or something

    const double closed = -0.1, producing = 1.5, water = 2.5, hcInjection = 3.5; // Closed set to -0.1 instead of 0.5 to workaround bug in the scalar mapper.

    std::list<RivPipeBranchData>::iterator brIt;
    const RigWellResultFrame& wResFrame = wRes->wellResultFrame(frameIndex);

    std::vector<double> wellCellStates;

    // Setup a scalar mapper
    cvf::ref<cvf::ScalarMapperDiscreteLinear> scalarMapper = new cvf::ScalarMapperDiscreteLinear;
    {
        cvf::Color3ubArray legendColors;
        legendColors.resize(4);
        legendColors[0] = cvf::Color3ub(m_rimWell->wellPipeColor());
        legendColors[1] = cvf::Color3::GREEN;
        legendColors[2] = cvf::Color3::BLUE;
        legendColors[3] = cvf::Color3::RED;
        scalarMapper->setColors(legendColors);
        scalarMapper->setRange(0.0, 4.0);
        scalarMapper->setLevelCount(4, true);
    }

    caf::ScalarMapperEffectGenerator surfEffGen(scalarMapper.p(), caf::PO_1);
    cvf::ref<cvf::Effect> scalarMapperSurfaceEffect = surfEffGen.generateUnCachedEffect();

    caf::ScalarMapperMeshEffectGenerator meshEffGen(scalarMapper.p());
    cvf::ref<cvf::Effect> scalarMapperMeshEffect = meshEffGen.generateUnCachedEffect();

    for (brIt = m_wellBranches.begin(); brIt != m_wellBranches.end(); ++brIt)
    {
        // Initialize well states to "closed" state
        wellCellStates.clear();
        wellCellStates.resize(brIt->m_cellIds.size(), closed);

        RimEclipseWellCollection* wellColl = nullptr;
        if (m_rimWell)
        {
            m_rimWell->firstAncestorOrThisOfType(wellColl);
        }

        if (wellColl && wellColl->showConnectionStatusColors())
        {
            const std::vector <RigWellResultPoint>& cellIds =  brIt->m_cellIds;
            for (size_t wcIdx = 0; wcIdx < cellIds.size(); ++wcIdx)
            {
                // we need a faster lookup, I guess
                const RigWellResultPoint* wResCell = NULL;
            
                if (cellIds[wcIdx].isCell())
                {
                    wResCell = wResFrame.findResultCell(cellIds[wcIdx].m_gridIndex, cellIds[wcIdx].m_gridCellIndex);
                }

                if (wResCell == NULL) 
                {
                    // We cant find any state. This well cell is closed.
                }
                else
                {
                    double cellState = closed;

                    if (wResCell->m_isOpen)
                    {
                        switch (wResFrame.m_productionType)
                        {
                        case RigWellResultFrame::PRODUCER:
                            cellState = producing;
                            break;
                        case RigWellResultFrame::OIL_INJECTOR:
                            cellState = hcInjection;
                            break;
                        case RigWellResultFrame::GAS_INJECTOR:
                            cellState = hcInjection;
                            break;
                        case RigWellResultFrame::WATER_INJECTOR:
                            cellState = water;
                            break;
                        case RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE:
                            cellState = closed;
                            break;
                        }
                    }

                    wellCellStates[wcIdx] = cellState;
                }
            }
        }

        // Find or create texture coords array for pipe surface

        if (brIt->m_surfaceDrawable.notNull())
        {
            cvf::ref<cvf::Vec2fArray> surfTexCoords = const_cast<cvf::Vec2fArray*>(brIt->m_surfaceDrawable->textureCoordArray());

            if (surfTexCoords.isNull())
            {
                surfTexCoords = new cvf::Vec2fArray;
            }

            brIt->m_pipeGeomGenerator->pipeSurfaceTextureCoords( surfTexCoords.p(), wellCellStates, scalarMapper.p());
            brIt->m_surfaceDrawable->setTextureCoordArray( surfTexCoords.p());

            brIt->m_surfacePart->setEffect(scalarMapperSurfaceEffect.p());
        }

        // Find or create texture coords array for pipe center line
        if (brIt->m_centerLineDrawable.notNull())
        {
            cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>(brIt->m_centerLineDrawable->textureCoordArray());

            if (lineTexCoords.isNull())
            {
                lineTexCoords = new cvf::Vec2fArray;
            }

            // Calculate new texture coordinates
            brIt->m_pipeGeomGenerator->centerlineTextureCoords( lineTexCoords.p(), wellCellStates, scalarMapper.p());

            // Set the new texture coordinates

            brIt->m_centerLineDrawable->setTextureCoordArray( lineTexCoords.p());

            // Set effects

            brIt->m_centerLinePart->setEffect(scalarMapperMeshEffect.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::findGridIndexAndCellIndex(size_t branchIndex, size_t triangleIndex, size_t* gridIndex, size_t* cellIndex)
{
    CVF_ASSERT(branchIndex < m_wellBranches.size());

    RivPipeBranchData* branchData = pipeBranchData(branchIndex);
    if (branchData)
    {
        size_t segmentIndex = branchData->m_pipeGeomGenerator->segmentIndexFromTriangleIndex(triangleIndex);

        *gridIndex = branchData->m_cellIds[segmentIndex].m_gridIndex;
        *cellIndex = branchData->m_cellIds[segmentIndex].m_gridCellIndex;
    }
    else
    {
        *gridIndex = cvf::UNDEFINED_SIZE_T;
        *cellIndex = cvf::UNDEFINED_SIZE_T;
    }
}

