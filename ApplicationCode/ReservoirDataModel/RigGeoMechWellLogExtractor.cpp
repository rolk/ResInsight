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

//==================================================================================================
/// 
//==================================================================================================
#include "RigGeoMechWellLogExtractor.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPartResultsCollection.h"

#include "RigWellLogExtractionTools.h"
#include "RigWellPath.h"
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::RigGeoMechWellLogExtractor(RigGeoMechCaseData* aCase, const RigWellPath* wellpath)
    :m_caseData(aCase), RigWellLogExtractor(wellpath)
{
    calculateIntersection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::curveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values)
{   
    CVF_TIGHT_ASSERT(values);
    
    if (!resAddr.isValid()) return ;

    const RigFemPart* femPart                 = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;
    const std::vector<float>& resultValues    = m_caseData->femPartResults()->resultValues(resAddr, 0, frameIndex);

    if (!resultValues.size()) return;

    values->resize(m_intersections.size());// + 1); // Plus one for the end of the wellpath stopping inside a cell

    for (size_t cpIdx = 0; cpIdx < m_intersections.size(); ++cpIdx)
    {
        size_t elmIdx = m_intersectedCells[cpIdx];
        RigElementType elmType = femPart->elementType(elmIdx);

        if (!(elmType == HEX8  || elmType == HEX8P)) continue;

        cvf::StructGridInterface::FaceType cellFace = m_intersectedCellFaces[cpIdx];

        int faceNodeCount = 0;
        const int* faceLocalIndices = RigFemTypes::localElmNodeIndicesForFace(elmType, cellFace, &faceNodeCount);
        const int* elmNodeIndices = femPart->connectivities(elmIdx);

        cvf::Vec3d v0(nodeCoords[elmNodeIndices[faceLocalIndices[0]]]);
        cvf::Vec3d v1(nodeCoords[elmNodeIndices[faceLocalIndices[1]]]);
        cvf::Vec3d v2(nodeCoords[elmNodeIndices[faceLocalIndices[2]]]);
        cvf::Vec3d v3(nodeCoords[elmNodeIndices[faceLocalIndices[3]]]);

        size_t resIdx0 = cvf::UNDEFINED_SIZE_T;
        size_t resIdx1 = cvf::UNDEFINED_SIZE_T;
        size_t resIdx2 = cvf::UNDEFINED_SIZE_T;
        size_t resIdx3 = cvf::UNDEFINED_SIZE_T;

        if (resAddr.resultPosType ==  RIG_NODAL)
        {
            resIdx0 = elmNodeIndices[faceLocalIndices[0]];
            resIdx1 = elmNodeIndices[faceLocalIndices[1]];
            resIdx2 = elmNodeIndices[faceLocalIndices[2]];
            resIdx3 = elmNodeIndices[faceLocalIndices[3]];
        }
        else
        {
            resIdx0 = (size_t)femPart->elementNodeResultIdx((int)elmIdx, faceLocalIndices[0]);
            resIdx1 = (size_t)femPart->elementNodeResultIdx((int)elmIdx, faceLocalIndices[1]);
            resIdx2 = (size_t)femPart->elementNodeResultIdx((int)elmIdx, faceLocalIndices[2]);
            resIdx3 = (size_t)femPart->elementNodeResultIdx((int)elmIdx, faceLocalIndices[3]);
        }

        double interpolatedValue = cvf::GeometryTools::interpolateQuad( 
            v0, resultValues[resIdx0],
            v1, resultValues[resIdx1],
            v2, resultValues[resIdx2],
            v3, resultValues[resIdx3],
            m_intersections[cpIdx]
        );  

        (*values)[cpIdx] = interpolatedValue;
    }

    // What do we do with the endpoint of the wellpath ?
    // Ignore it for now ...
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::calculateIntersection()
{
    CVF_ASSERT(m_caseData->femParts()->partCount() == 1);
    
    const RigFemPart* femPart = m_caseData->femParts()->part(0);
    const std::vector<cvf::Vec3f>& nodeCoords =  femPart->nodes().coordinates;

    //double globalMeasuredDepth = 0; // Where do we start ? z - of first well path point ? 

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
        for (size_t ccIdx = 0; ccIdx < closeCells.size(); ++ccIdx)
        {
            RigElementType elmType = femPart->elementType(closeCells[ccIdx]);
            if (!(elmType == HEX8 || elmType == HEX8P)) continue;

            const int* cornerIndices = femPart->connectivities(closeCells[ccIdx]);

            hexCorners[0] = cvf::Vec3d(nodeCoords[cornerIndices[0]]);
            hexCorners[1] = cvf::Vec3d(nodeCoords[cornerIndices[1]]);
            hexCorners[2] = cvf::Vec3d(nodeCoords[cornerIndices[2]]);
            hexCorners[3] = cvf::Vec3d(nodeCoords[cornerIndices[3]]);
            hexCorners[4] = cvf::Vec3d(nodeCoords[cornerIndices[4]]);
            hexCorners[5] = cvf::Vec3d(nodeCoords[cornerIndices[5]]);
            hexCorners[6] = cvf::Vec3d(nodeCoords[cornerIndices[6]]);
            hexCorners[7] = cvf::Vec3d(nodeCoords[cornerIndices[7]]);

            int intersectionCount = RigHexIntersector::lineHexCellIntersection(p1, p2, hexCorners, closeCells[ccIdx], &intersections);
        }

        // Now, with all the intersections of this piece of line, we need to 
        // sort them in order, and set the measured depth and corresponding cell index

        // map <WellPathDepthPoint, (CellIdx, intersectionPoint)>
        std::map<RigMDEnterLeaveIntersectionSorterKey, HexIntersectionInfo > sortedIntersections;

        double md1 = m_wellPath->m_measuredDepths[wpp];
        double md2 = m_wellPath->m_measuredDepths[wpp+1];

        for (size_t intIdx = 0; intIdx < intersections.size(); ++intIdx)
        {
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

            sortedIntersections.insert(std::make_pair(RigMDEnterLeaveIntersectionSorterKey(measuredDepthOfPoint, intersections[intIdx].m_isIntersectionEntering), intersections[intIdx]));
        }

        // Now populate the return arrays

        std::map<RigMDEnterLeaveIntersectionSorterKey, HexIntersectionInfo >::iterator it;
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
std::vector<size_t> RigGeoMechWellLogExtractor::findCloseCells(const cvf::BoundingBox& bb)
{
    std::vector<size_t> closeCells;

    if (m_caseData->femParts()->partCount())
    {
        m_caseData->femParts()->part(0)->findIntersectingCells(bb, &closeCells);
    }
    return closeCells;
}


