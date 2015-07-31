/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimCellRangeFilter.h"
#include "cafPdmChildArrayField.h"

class RigActiveCellInfo;
class RigGridBase;
class RimView;
class RigFemPartCollection;

//==================================================================================================
///  
///  
//==================================================================================================
class RimCellRangeFilterCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCellRangeFilterCollection();
    virtual ~RimCellRangeFilterCollection();

    // Fields
    caf::PdmField<bool> isActive;
    caf::PdmChildArrayField<RimCellRangeFilter*> rangeFilters;

    // Methods
    RimCellRangeFilter*             createAndAppendRangeFilter();
    void                            remove(RimCellRangeFilter* rangeFilter);

    void                            compoundCellRangeFilter(cvf::CellRangeFilter* cellRangeFilter, size_t gridIndex) const;
    bool                            hasActiveFilters() const;
    bool                            hasActiveIncludeFilters() const;

    void                            setReservoirView(RimView* reservoirView);

    RimView*                        reservoirView();
    const cvf::StructGridInterface* gridByIndex(int gridIndex) const;
    int                             gridCount() const;
    QString                         gridName(int gridIndex) const;

    RigActiveCellInfo*              activeCellInfo() const;

    // Overridden methods
    virtual void                    fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );
    virtual caf::PdmFieldHandle*    objectToggleField();

protected:
    // Overridden methods
    virtual void                    initAfterRead();


private:
    RimEclipseView*                 eclipseView() const;
    RigMainGrid*                    mainGrid() const;
    RigFemPartCollection*           femPartColl() const;

    RimView*                        m_reservoirView;
};
