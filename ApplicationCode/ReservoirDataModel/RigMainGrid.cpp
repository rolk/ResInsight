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

#include "RIStdInclude.h"

#include "RigMainGrid.h"
#include "RigReservoirCellResults.h"

#include "cvfAssert.h"

RigMainGrid::RigMainGrid(void)
    : RigGridBase(this),
        m_activeCellPositionMin(cvf::Vec3st::UNDEFINED),
        m_activeCellPositionMax(cvf::Vec3st::UNDEFINED),
        m_validCellPositionMin(cvf::Vec3st::UNDEFINED),
        m_validCellPositionMax(cvf::Vec3st::UNDEFINED),
        m_activeCellCount(cvf::UNDEFINED_SIZE_T)
{
	m_results = new RigReservoirCellResults(this);

    m_activeCellsBoundingBox.add(cvf::Vec3d::ZERO);
    m_gridIndex = 0;
}


RigMainGrid::~RigMainGrid(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::addLocalGrid(RigLocalGrid* localGrid)
{
    m_localGrids.push_back(localGrid);
    localGrid->setGridIndex(m_localGrids.size()); // Maingrid itself has grid index 0
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::initAllSubGridsParentGridPointer()
{
    initSubGridParentPointer();
    size_t i;
    for (i = 0; i < m_localGrids.size(); ++i)
    {
       m_localGrids[i]->initSubGridParentPointer(); 
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::initAllSubCellsMainGridCellIndex()
{
    initSubCellsMainGridCellIndex();
    size_t i;
    for (i = 0; i < m_localGrids.size(); ++i)
    {
        m_localGrids[i]->initSubCellsMainGridCellIndex(); 
    }
}

//--------------------------------------------------------------------------------------------------
/// Calculates the number of active cells in the complete reservoir.
/// Caches the result, so subsequent calls are fast
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::numActiveCells()
{
    if (m_activeCellCount != cvf::UNDEFINED_SIZE_T) return m_activeCellCount;

    if (m_cells.size() == 0) return 0;

    size_t numActiveCells = 0;

    size_t i;
    for (i = 0; i < m_cells.size(); i++)
    {
        if (m_cells[i].active()) numActiveCells++;
    }

    m_activeCellCount = numActiveCells;
    return m_activeCellCount;
}





//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::activeCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const
{
    min = m_activeCellPositionMin;
    max = m_activeCellPositionMax;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigMainGrid::activeCellsBoundingBox() const
{
    return m_activeCellsBoundingBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::validCellsBoundingBox(cvf::Vec3st& min, cvf::Vec3st& max) const
{
    min = m_validCellPositionMin;
    max = m_validCellPositionMax;
}



//--------------------------------------------------------------------------------------------------
/// Helper class used to find min/max range for valid and active cells
//--------------------------------------------------------------------------------------------------
class CellRangeBB
{
public:
    CellRangeBB()
        : m_min(cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T),
        m_max(cvf::Vec3st::ZERO)
    {

    }

    void add(size_t i, size_t j, size_t k)
    {
        if (i < m_min.x()) m_min.x() = i;
        if (j < m_min.y()) m_min.y() = j;
        if (k < m_min.z()) m_min.z() = k;

        if (i > m_max.x()) m_max.x() = i;
        if (j > m_max.y()) m_max.y() = j;
        if (k > m_max.z()) m_max.z() = k;
    }

public:
    cvf::Vec3st m_min;
    cvf::Vec3st m_max;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::computeActiveAndValidCellRanges()
{
    CellRangeBB validBB;
    CellRangeBB activeBB;

    size_t idx;
    for (idx = 0; idx < cellCount(); idx++)
    {
        const RigCell& c = cell(idx);
        
        size_t i, j, k;
        ijkFromCellIndex(idx, &i, &j, &k);

        if (!c.isInvalid())
        {
            validBB.add(i, j, k);
        }

        if (c.active())
        {
            activeBB.add(i, j, k);
        }
    }

    m_validCellPositionMin = validBB.m_min;
    m_validCellPositionMax = validBB.m_max;

    m_activeCellPositionMin = activeBB.m_min;
    m_activeCellPositionMax = activeBB.m_max;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigMainGrid::displayModelOffset() const
{
    return m_activeCellsBoundingBox.min();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigMainGrid::computeBoundingBox()
{
    m_activeCellsBoundingBox.reset();

    if (m_nodes.size() == 0)
    {
        m_activeCellsBoundingBox.add(cvf::Vec3d::ZERO);
    }
    else
    {
        size_t i;
        for (i = 0; i < cellCount(); i++)
        {
            const RigCell& c = cell(i);
            if (c.active())
            {
                const caf::SizeTArray8& indices = c.cornerIndices();

                size_t idx;
                for (idx = 0; idx < 8; idx++)
                {
                    m_activeCellsBoundingBox.add(m_nodes[indices[idx]]);
                }
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Initialize pointers from grid to parent grid
/// Compute cell ranges for active and valid cells
/// Compute bounding box in world coordinates based on node coordinates
//--------------------------------------------------------------------------------------------------
void RigMainGrid::computeCachedData()
{
    initAllSubGridsParentGridPointer();
    initAllSubCellsMainGridCellIndex();
    computeActiveAndValidCellRanges();
    computeBoundingBox();
}

//--------------------------------------------------------------------------------------------------
///
///
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::calculateActiveCellInfo(std::vector<qint32> &gridNumber,
                                          std::vector<qint32> &cellI,
                                          std::vector<qint32> &cellJ,
                                          std::vector<qint32> &cellK,
                                          std::vector<qint32> &parentGridNumber,
                                          std::vector<qint32> &hostCellI,
                                          std::vector<qint32> &hostCellJ,
                                          std::vector<qint32> &hostCellK)
{
    size_t numActiveCells = this->numActiveCells();

    gridNumber.clear();
    cellI.clear();
    cellJ.clear();
    cellK.clear();
    parentGridNumber.clear();
    hostCellI.clear();
    hostCellJ.clear();
    hostCellK.clear();

    gridNumber.reserve(numActiveCells);
    cellI.reserve(numActiveCells);
    cellJ.reserve(numActiveCells);
    cellK.reserve(numActiveCells);
    parentGridNumber.reserve(numActiveCells);
    hostCellI.reserve(numActiveCells);
    hostCellJ.reserve(numActiveCells);
    hostCellK.reserve(numActiveCells);

    for (size_t cIdx = 0; cIdx < m_cells.size(); ++cIdx)
    {
        if (m_cells[cIdx].active())
        {
            RigGridBase* grid = m_cells[cIdx].hostGrid();
            CVF_ASSERT(grid != NULL);
            size_t cellIndex = m_cells[cIdx].cellIndex();
            
            size_t i, j, k;
            grid->ijkFromCellIndex(cellIndex, &i, &j, &k);

            size_t pi, pj, pk;
            RigGridBase* parentGrid = NULL;

            if (grid->isMainGrid())
            {
                pi = i;
                pj = j;
                pk = k;
                parentGrid = grid;
            }
            else
            {
                size_t parentCellIdx = m_cells[cIdx].parentCellIndex();
                parentGrid = (static_cast<RigLocalGrid*>(grid))->parentGrid();
                CVF_ASSERT(parentGrid != NULL);
                parentGrid->ijkFromCellIndex(parentCellIdx, &pi, &pj, &pk);
            }

            gridNumber.push_back(grid->gridIndex());
            cellI.push_back(i);
            cellJ.push_back(j);
            cellK.push_back(k);
            parentGridNumber.push_back(parentGrid->gridIndex());
            hostCellI.push_back(pi);
            hostCellJ.push_back(pj);
            hostCellK.push_back(pk);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with index \a localGridIndex. Main Grid itself has index 0. First LGR starts on 1
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridByIndex(size_t localGridIndex)
{
    if (localGridIndex == 0) return this;
    CVF_ASSERT(localGridIndex - 1 < m_localGrids.size()) ;
    return m_localGrids[localGridIndex-1].p();
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with index \a localGridIndex. Main Grid itself has index 0. First LGR starts on 1
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigMainGrid::gridByIndex(size_t localGridIndex) const
{
    if (localGridIndex == 0) return this;
    CVF_ASSERT(localGridIndex - 1 < m_localGrids.size()) ;
    return m_localGrids[localGridIndex-1].p();
}
