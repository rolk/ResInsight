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

#include "RivIntersectionSourceInfo.h"

#include "RivIntersectionGeometryGenerator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionSourceInfo::RivIntersectionSourceInfo(RivIntersectionGeometryGenerator* geometryGenerator)
    : m_crossSectionGeometryGenerator(geometryGenerator)
{
    CVF_ASSERT(m_crossSectionGeometryGenerator.notNull());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivIntersectionSourceInfo::triangleToCellIndex() const
{
    CVF_ASSERT(m_crossSectionGeometryGenerator.notNull());

    return m_crossSectionGeometryGenerator->triangleToCellIndex();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimCrossSection* RivIntersectionSourceInfo::crossSection() const
{
    return m_crossSectionGeometryGenerator->crossSection();
}
