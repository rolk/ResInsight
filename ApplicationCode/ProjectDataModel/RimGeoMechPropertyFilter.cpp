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

#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechResultDefinition.h"

#include "cvfMath.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cvfAssert.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RimGeoMechView.h"

/*
#include "RigCaseCellResultsData.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimResultDefinition.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cvfAssert.h"
*/


CAF_PDM_SOURCE_INIT(RimGeoMechPropertyFilter, "GeoMechPropertyFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilter::RimGeoMechPropertyFilter()
    : m_parentContainer(NULL)
{
    CAF_PDM_InitObject("GeoMech Property Filter", ":/CellFilter_Values.png", "", "");

    CAF_PDM_InitFieldNoDefault(&resultDefinition, "ResultDefinition", "Result definition", "", "", "");
    resultDefinition = new RimGeoMechResultDefinition();
    resultDefinition->setOwnerPropertyFilter(this);

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    resultDefinition.setUiHidden(true);

    CAF_PDM_InitField(&lowerBound, "LowerBound", 0.0, "Min", "", "", "");
    lowerBound.setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&upperBound, "UpperBound", 0.0, "Max", "", "", "");
    upperBound.setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    updateIconState();

    m_minimumResultValue = cvf::UNDEFINED_DOUBLE;
    m_maximumResultValue = cvf::UNDEFINED_DOUBLE;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilter::~RimGeoMechPropertyFilter()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (   &lowerBound == changedField 
        || &upperBound == changedField
        || &isActive == changedField
        || &filterMode == changedField)
    {
        this->updateIconState();
        ((RimView*)resultDefinition->reservoirView())->scheduleGeometryRegen(PROPERTY_FILTERED);
        resultDefinition->reservoirView()->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::setParentContainer(RimGeoMechPropertyFilterCollection* parentContainer)
{
    m_parentContainer = parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilterCollection* RimGeoMechPropertyFilter::parentContainer()
{
    return m_parentContainer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::updateDefaultValues()
{
    CVF_ASSERT(m_parentContainer);

    computeResultValueRange();

    lowerBound = m_minimumResultValue;
    upperBound = m_maximumResultValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
{
    uiOrdering.add(&name);

    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Result");
    group1->add(&(resultDefinition->m_resultPositionTypeUiField));
    group1->add(&(resultDefinition->m_resultVariableUiField));

    uiOrdering.add(&isActive);
    uiOrdering.add(&filterMode);

    uiOrdering.add(&lowerBound);
    uiOrdering.add(&upperBound);
    uiOrdering.add(&filterMode);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (m_minimumResultValue == cvf::UNDEFINED_DOUBLE || m_maximumResultValue == cvf::UNDEFINED_DOUBLE)
    {
        return;
    }

    if (field == &lowerBound || field == &upperBound)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (!myAttr)
        {
            return;
        }

        myAttr->m_minimum = m_minimumResultValue;
        myAttr->m_maximum = m_maximumResultValue;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechPropertyFilter::computeResultValueRange()
{
    CVF_ASSERT(m_parentContainer);

    double min = 0.0;
    double max = 0.0;

    RigFemResultAddress resultAddress = resultDefinition->resultAddress();
    if (resultAddress.isValid() && resultDefinition->ownerCaseData())
    {
        resultDefinition->ownerCaseData()->femPartResults()->minMaxScalarValues(resultAddress, &min, &max);
    }

    m_maximumResultValue = max;
    m_minimumResultValue = min;

    lowerBound.setUiName(QString("Min (%1)").arg(min));
    upperBound.setUiName(QString("Max (%1)").arg(max));
}
