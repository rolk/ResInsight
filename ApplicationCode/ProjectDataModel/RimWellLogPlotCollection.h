/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"
#include "cvfCollection.h"

class RimWellLogPlot;
class RigEclipseWellLogExtractor;
class RigGeoMechWellLogExtractor;
class RimGeoMechCase;
class RigCaseData;
class RigGeoMechCaseData;
class RigWellPath;
class RimWellPath;
class RimEclipseCase;
class RiuWellLogPlot;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlotCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogPlotCollection();
    virtual ~RimWellLogPlotCollection();
    
    RigEclipseWellLogExtractor* findOrCreateExtractor(RimWellPath* wellPath, RimEclipseCase* eclCase);
    RigGeoMechWellLogExtractor* findOrCreateExtractor(RimWellPath* wellPath, RimGeoMechCase* eclCase);

    void removeExtractors(const RigWellPath* wellPath);
    void removeExtractors(const RigCaseData* caseData);
    void removeExtractors(const RigGeoMechCaseData* caseData);

    RimWellLogPlot* wellLogPlotFromViewer(RiuWellLogPlot* viewer) const;

    caf::PdmChildArrayField<RimWellLogPlot*> wellLogPlots;
private:
    cvf::Collection<RigEclipseWellLogExtractor> m_extractors;
    cvf::Collection<RigGeoMechWellLogExtractor> m_geomExtractors;
};
