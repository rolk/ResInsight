/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigEclipseWellLogExtractor.h"
#include <map>
#include "RigCaseData.h"
#include "RigWellPath.h"
#include "RigResultAccessor.h"
#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"

#include "RigWellLogExtractionTools.h"
#include "RigMainGrid.h"

//==================================================================================================
/// 
//==================================================================================================

RigEclipseWellLogExtractor::RigEclipseWellLogExtractor(const RigCaseData* aCase, const RigWellPath* wellpath)
    : m_caseData(aCase), m_wellPath(wellpath)
{
    calculateIntersection();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseWellLogExtractor::calculateIntersection()
{
    const std::vector<cvf::Vec3d>& nodeCoords =  m_caseData->mainGrid()->nodes();

    bool isCellFaceNormalsOut = m_caseData->mainGrid()->isFaceNormalsOutwards();

    if (!m_wellPath->m_wellPathPoints.size()) return ;

    for (size_t wpp = 0; wpp < m_wellPath->m_wellPathPoints.size() - 1; ++wpp)
    {
        cvf::BoundingBox bb;
        cvf::Vec3d p1 = m_wellPath->m_wellPathPoints[wpp];
        cvf::Vec3d p2 = m_wellPath->m_wellPathPoints[wpp+1];

        bb.add(p1);
        bb.add(p2);

        std::vector<size_t> closeCells = findCloseCells(bb);
        std::vector<HexIntersectionInfo> intersections;

        cvf::Vec3d hexCorners[8];
        for (size_t cIdx = 0; cIdx < closeCells.size(); ++cIdx)
        {
            const RigCell& cell = m_caseData->mainGrid()->cells()[closeCells[cIdx]];
            const caf::SizeTArray8& cornerIndices = cell.cornerIndices();

            hexCorners[0] = nodeCoords[cornerIndices[0]];
            hexCorners[1] = nodeCoords[cornerIndices[1]];
            hexCorners[2] = nodeCoords[cornerIndices[2]];
            hexCorners[3] = nodeCoords[cornerIndices[3]];
            hexCorners[4] = nodeCoords[cornerIndices[4]];
            hexCorners[5] = nodeCoords[cornerIndices[5]];
            hexCorners[6] = nodeCoords[cornerIndices[6]];
            hexCorners[7] = nodeCoords[cornerIndices[7]];

            int intersectionCount = RigHexIntersector::lineHexCellIntersection(p1, p2, hexCorners, closeCells[cIdx], &intersections);
        }

        // Now, with all the intersections of this piece of line, we need to 
        // sort them in order, and set the measured depth and corresponding cell index
        
        // map <WellPathDepthPoint, (CellIdx, intersectionPoint)>
        std::map<WellPathDepthPoint, HexIntersectionInfo > sortedIntersections;

        double md1 = m_wellPath->m_measuredDepths[wpp];
        double md2 = m_wellPath->m_measuredDepths[wpp+1];

        for (size_t intIdx = 0; intIdx < intersections.size(); ++intIdx)
        {
            if (!isCellFaceNormalsOut) intersections[intIdx].m_isIntersectionEntering = !intersections[intIdx].m_isIntersectionEntering ;
            double lenghtAlongLineSegment1 = (intersections[intIdx].m_intersectionPoint - p1).length();
            double lenghtAlongLineSegment2 = (p2 - intersections[intIdx].m_intersectionPoint).length();
            double measuredDepthDiff = md2 - md1;
            double lineLength = lenghtAlongLineSegment1 + lenghtAlongLineSegment2;
            double measuredDepthOfPoint = 0.0;
            if (lineLength > 0.00001)
            {
                measuredDepthOfPoint = md1 + measuredDepthDiff*lenghtAlongLineSegment1/(lineLength);
            }
            else
            {
                measuredDepthOfPoint = md1;
            }
            sortedIntersections.insert(std::make_pair(WellPathDepthPoint(measuredDepthOfPoint, intersections[intIdx].m_isIntersectionEntering),  intersections[intIdx]));  
        }

        // Now populate the return arrays

        std::map<WellPathDepthPoint, HexIntersectionInfo >::iterator it;
        it = sortedIntersections.begin();
        while (it != sortedIntersections.end())
        {
            m_measuredDepth.push_back(it->first.measuredDepth);
            m_trueVerticalDepth.push_back(abs(it->second.m_intersectionPoint[2]));
            m_intersections.push_back(it->second.m_intersectionPoint);
            m_intersectedCells.push_back(it->second.m_hexIndex);
            m_intersectedCellFaces.push_back(it->second.m_face);
            ++it;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseWellLogExtractor::curveData(const RigResultAccessor* resultAccessor, std::vector<double>* values)
{
    CVF_TIGHT_ASSERT(values);
    values->resize(m_intersections.size());// + 1); // Plus one for the end of the wellpath stopping inside a cell

    for (size_t cpIdx = 0; cpIdx < m_intersections.size(); ++cpIdx)
    {
        size_t cellIdx = m_intersectedCells[cpIdx];
        cvf::StructGridInterface::FaceType cellFace = m_intersectedCellFaces[cpIdx];
        (*values)[cpIdx] = resultAccessor->cellFaceScalarGlobIdx(cellIdx, cellFace);
    }

    // What do we do with the endpoint of the wellpath ?
    // Ignore it for now ...
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigEclipseWellLogExtractor::findCloseCells(const cvf::BoundingBox& bb)
{
    std::vector<size_t> closeCells;
    m_caseData->mainGrid()->findIntersectingCells(bb, &closeCells);
    return closeCells;
}


