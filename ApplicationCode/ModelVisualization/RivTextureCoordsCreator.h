/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfArray.h"

class RimResultSlot;
class RigResultAccessor;
class RivResultToTextureMapper;

namespace cvf
{
	class StructGridQuadToCellFaceMapper;
}


class RivTextureCoordsCreator
{
public:
    RivTextureCoordsCreator(RimResultSlot* cellResultSlot, 
                            size_t timeStepIndex,  
                            size_t gridIndex, 
                            const cvf::StructGridQuadToCellFaceMapper* quadMapper);

	bool isValid();

    void createTextureCoords(cvf::Vec2fArray* quadTextureCoords);

private:

    static void createTextureCoords(cvf::Vec2fArray* quadTextureCoords,
                                    const cvf::StructGridQuadToCellFaceMapper* quadMapper,  
                                    const RigResultAccessor* resultAccessor,
                                    const RivResultToTextureMapper* texMapper);
    cvf::cref<cvf::StructGridQuadToCellFaceMapper>	m_quadMapper; 
    cvf::ref<RigResultAccessor>						m_resultAccessor; 
    cvf::ref<RivResultToTextureMapper>				m_texMapper;
};

