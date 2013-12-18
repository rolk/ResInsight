/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"
#include "RivReservoirPartMgr.h"

#include "cvfStructGrid.h"
#include "cvfModelBasicList.h"

#include "RigCaseData.h"
#include "RivGridPartMgr.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::clearAndSetReservoir(const RigCaseData* eclipseCase, const RimFaultCollection* faultCollection)
{
    m_allGrids.clear();

    if (eclipseCase)
    {
        std::vector<const RigGridBase*> grids;
        eclipseCase->allGrids(&grids);
        for (size_t i = 0; i < grids.size() ; ++i)
        {
            m_allGrids.push_back(new RivGridPartMgr(grids[i], i, faultCollection));
        }

        // Faults read from file are present only on main grid
        m_faultsPartMgr = new RivReservoirFaultsPartMgr(eclipseCase->mainGrid(), faultCollection);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::setTransform(cvf::Transform* scaleTransform)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->setTransform(scaleTransform);
    }

    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->setTransform(scaleTransform);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::setCellVisibility(size_t gridIndex, cvf::UByteArray* cellVisibilities)
{
    CVF_ASSERT(gridIndex < m_allGrids.size());
    m_allGrids[gridIndex]->setCellVisibility(cellVisibilities);

    if (gridIndex == 0)
    {
        CVF_ASSERT(m_faultsPartMgr.notNull());
        m_faultsPartMgr->setCellVisibility(cellVisibilities);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RivReservoirPartMgr::cellVisibility(size_t gridIdx)
{
    CVF_ASSERT(gridIdx < m_allGrids.size()); 
    return  m_allGrids[gridIdx]->cellVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateCellColor(cvf::Color4f color)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->updateCellColor(color);
    }

    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->updateCellColor(color);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->updateCellResultColor(timeStepIndex, cellResultSlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateCellEdgeResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot, RimCellEdgeResultSlot* cellEdgeResultSlot)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->updateCellEdgeResultColor(timeStepIndex, cellResultSlot, cellEdgeResultSlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendGridPartsToModel(cvf::ModelBasicList* model)
{
    for (size_t i = 0; i < m_allGrids.size() ; ++i)
    {
        m_allGrids[i]->appendPartsToModel(model);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendGridPartsToModel(cvf::ModelBasicList* model, const std::vector<size_t>& gridIndices)
{
    for (size_t i = 0; i < gridIndices.size() ; ++i)
    {
        if (gridIndices[i] < m_allGrids.size())
        {
            m_allGrids[gridIndices[i]]->appendPartsToModel(model);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::updateFaultCellResultColor(size_t timeStepIndex, RimResultSlot* cellResultSlot)
{
    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->updateCellResultColor(timeStepIndex, cellResultSlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivReservoirPartMgr::appendFaultPartsToModel(cvf::ModelBasicList* model)
{
    if (m_faultsPartMgr.notNull())
    {
        m_faultsPartMgr->appendPartsToModel(model);
    }
}

