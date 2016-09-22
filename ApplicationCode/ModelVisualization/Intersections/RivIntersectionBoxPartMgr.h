/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RivIntersectionBoxGeometryGenerator.h"

#include "cvfBase.h"
#include "cvfObject.h"


namespace cvf
{
    class ModelBasicList;
    class Transform;
    class Part;
}

class RigMainGrid;
class RigResultAccessor;
class RimCellEdgeColors;
class RimEclipseCellColors;
class RimIntersectionBox;

//==================================================================================================
///
///
//==================================================================================================

class RivIntersectionBoxPartMgr : public cvf::Object
{
public:
    RivIntersectionBoxPartMgr(const RimIntersectionBox* intersectionBox);

    void applySingleColorEffect();
    void updateCellResultColor(size_t timeStepIndex);


    void appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);
    void appendMeshLinePartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);

private:
    void updatePartEffect();
    void generatePartGeometry();
    void computeData();

    static void calculateEclipseTextureCoordinates(cvf::Vec2fArray* textureCoords, 
                                                   const std::vector<size_t>& triangleToCellIdxMap, 
                                                   const RigResultAccessor* resultAccessor, 
                                                   const cvf::ScalarMapper* mapper);

    static void calculateGeoMechTextureCoords(cvf::Vec2fArray* textureCoords, 
                                              const std::vector<RivIntersectionVertexWeights> &vertexWeights, 
                                              const std::vector<float> &resultValues, 
                                              bool isElementNodalResult, 
                                              const RigFemPart* femPart, 
                                              const cvf::ScalarMapper* mapper);

    cvf::ref<RivIntersectionHexGridInterface> createHexGridInterface();
private:
    const RimIntersectionBox*   m_rimIntersectionBox;

    cvf::Color3f                m_defaultColor;

    cvf::ref<cvf::Part>         m_intersectionBoxFaces;
    cvf::ref<cvf::Part>         m_intersectionBoxGridLines;
    cvf::ref<cvf::Vec2fArray>   m_intersectionBoxFacesTextureCoords;

    cvf::ref<RivIntersectionBoxGeometryGenerator> m_intersectionBoxGenerator;
};
