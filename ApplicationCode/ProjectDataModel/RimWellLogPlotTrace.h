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

#include <QPointer>

#include <vector>

class RimWellLogPlotCurve;
class RiuWellLogTracePlot;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlotTrace : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogPlotTrace();
    virtual ~RimWellLogPlotTrace();

    void setViewer(RiuWellLogTracePlot* viewer);
    void addCurve(RimWellLogPlotCurve* curve);

    RiuWellLogTracePlot* viewer();

    caf::PdmChildArrayField<RimWellLogPlotCurve*> curves;

protected:

    // Overridden PDM methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

private:
    virtual caf::PdmFieldHandle* objectToggleField();

private:
    caf::PdmField<bool> show;
    QPointer<RiuWellLogTracePlot> m_viewer;
};
