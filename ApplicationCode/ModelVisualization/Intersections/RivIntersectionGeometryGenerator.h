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

#pragma once

#include "RivHexGridIntersectionTools.h"

#include "cafPdmPointer.h"

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class RigMainGrid;
class RigActiveCellInfo;
class RigResultAccessor;
class RimCrossSection;

namespace cvf
{
    class ScalarMapper;
    class DrawableGeo;
}


class RivIntersectionGeometryGenerator : public cvf::Object
{
public:
    RivIntersectionGeometryGenerator(const RimCrossSection* crossSection,
                                     std::vector<std::vector<cvf::Vec3d> > &polylines, 
                                     const cvf::Vec3d& extrusionDirection, 
                                     const RivIntersectionHexGridInterface* grid );

    ~RivIntersectionGeometryGenerator();

    bool                        isAnyGeometryPresent() const;
 
    // Generate geometry
    cvf::ref<cvf::DrawableGeo>  generateSurface();
    cvf::ref<cvf::DrawableGeo>  createMeshDrawable();
    cvf::ref<cvf::DrawableGeo>  createLineAlongPolylineDrawable();
    cvf::ref<cvf::DrawableGeo>  createPointsFromPolylineDrawable();

    // Mapping between cells and geometry
    const std::vector<size_t>&           triangleToCellIndex() const;
    const std::vector<RivIntersectionVertexWeights>& triangleVxToCellCornerInterpolationWeights() const;

    const RimCrossSection* crossSection() const;

private:
    void                        calculateArrays();
    static void                 adjustPolyline(const std::vector<cvf::Vec3d>& polyLine, 
                                               const cvf::Vec3d extrDir,
                                               std::vector<cvf::Vec3d>* adjustedPolyline);

    cvf::cref<RivIntersectionHexGridInterface>      m_hexGrid;
    const std::vector<std::vector<cvf::Vec3d> >     m_polyLines;
    cvf::Vec3d                  m_extrusionDirection;

    // Output arrays
    cvf::ref<cvf::Vec3fArray>   m_triangleVxes;
    cvf::ref<cvf::Vec3fArray>   m_cellBorderLineVxes;
    std::vector<size_t>         m_triangleToCellIdxMap;
    std::vector<RivIntersectionVertexWeights> m_triVxToCellCornerWeights;

    const RimCrossSection*      m_crossSection;
};

