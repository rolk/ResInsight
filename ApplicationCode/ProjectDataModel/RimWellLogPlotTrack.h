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
class RiuWellLogTrackPlot;

class QwtPlotCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlotTrack : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogPlotTrack();
    virtual ~RimWellLogPlotTrack();

    void setDescription(const QString& description);

    void addCurve(RimWellLogPlotCurve* curve);
    size_t curveCount() { return curves.size(); }

    void recreateViewer();
    void detachAllCurves();

    void loadDataAndUpdate();

    bool availableDepthRange(double* minimumDepth, double* maximumDepth);
    void updateAxisRangesAndReplot();
    void updateXAxisRangeFromCurves();

    RiuWellLogTrackPlot* viewer();
    
    RimWellLogPlotCurve* curveDefinitionFromCurve(const QwtPlotCurve* curve) const;

protected:

    // Overridden PDM methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle* objectToggleField();
    virtual caf::PdmFieldHandle* userDescriptionField();
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

private:
    caf::PdmField<bool> m_show;
    caf::PdmField<QString> m_userName;
    caf::PdmChildArrayField<RimWellLogPlotCurve*> curves;
    caf::PdmField<double> m_visibleXRangeMin;
    caf::PdmField<double> m_visibleXRangeMax;

    QPointer<RiuWellLogTrackPlot> m_viewer;
};