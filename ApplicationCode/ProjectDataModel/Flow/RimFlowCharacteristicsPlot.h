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


#include "RimViewWindow.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QPointer>

class RimFlowDiagSolution;
class RimEclipseResultCase;

class RiuFlowCharacteristicsPlot;

namespace caf {
    class PdmOptionItemInfo;
}

namespace cvf {
    class Color3f;
}


//==================================================================================================
///  
///  
//==================================================================================================
class RimFlowCharacteristicsPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimFlowCharacteristicsPlot();
    virtual ~RimFlowCharacteristicsPlot();

    void setFromFlowSolution(RimFlowDiagSolution* flowSolution);

    // RimViewWindow overrides

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;
    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 



protected:
    // RimViewWindow overrides

    virtual void                                    loadDataAndUpdate() override;
    virtual QImage                                  snapshotWindowContent() override;

    // Overridden PDM methods
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

private:
    caf::PdmPtrField<RimEclipseResultCase*>    m_case;
    caf::PdmPtrField<RimFlowDiagSolution*>     m_flowDiagSolution;

    QPointer<RiuFlowCharacteristicsPlot>       m_flowCharPlotWidget;
};
