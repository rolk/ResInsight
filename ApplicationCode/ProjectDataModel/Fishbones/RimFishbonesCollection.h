/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"

class RimFishbonesMultipleSubs;
class RimFishboneWellPathCollection;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimFishbonesCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFishbonesCollection();

    RimFishboneWellPathCollection* wellPathCollection() const;

    caf::PdmChildArrayField<RimFishbonesMultipleSubs*>  fishbonesSubs;

private:
    caf::PdmChildField<RimFishboneWellPathCollection*>  m_wellPathCollection;
};