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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimView;
class RimEclipseView;
class RimGeoMechView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimManagedViewConfig : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimManagedViewConfig(void);
    virtual ~RimManagedViewConfig(void);

    caf::PdmField<QString>      name;

    caf::PdmPtrField<RimView*>  managedView;
    
    caf::PdmField<bool>         syncCamera;
    caf::PdmField<bool>         syncCellResult;
    caf::PdmField<bool>         syncTimeStep;
    caf::PdmField<bool>         syncRangeFilters;
    caf::PdmField<bool>         syncPropertyFilters;

    void                        configureOverrides();

protected:
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual void                            initAfterRead();
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &name; }


private:
    void            configureOverridesUpdateDisplayModel();

    RimEclipseView* managedEclipseView();
    RimGeoMechView* managedGeoView();

    void            updateDisplayName();
    QString         displayNameForView(RimView* view);
};
