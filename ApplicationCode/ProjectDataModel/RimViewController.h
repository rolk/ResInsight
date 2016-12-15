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

#include "cvfBase.h"
#include "cvfObject.h"

#include "RivCellSetEnum.h"

class RimView;
class RimEclipseView;
class RimGeoMechView;
class RimViewLinker;
class RigCaseToCaseCellMapper;
class RimCellRangeFilter;

//==================================================================================================
///  
///  
//==================================================================================================
class RimViewController : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimViewController(void);
    virtual ~RimViewController(void);

    bool                                    isActive();

    RimView*                                managedView();
    void                                    setManagedView(RimView* view);

    RimView*                                masterView();
    RimViewLinker*                          ownerViewLinker();

    const RigCaseToCaseCellMapper*          cellMapper();
    
    bool                                    isCameraLinked();
    bool                                    showCursor() const;
    bool                                    isTimeStepLinked();

    bool                                    isResultColorControlled();
    bool                                    isLegendDefinitionsControlled();
    bool                                    isRangeFiltersControlled();
    
    bool                                    isVisibleCellsOveridden();
    bool                                    isPropertyFilterOveridden();

    void                                    scheduleCreateDisplayModelAndRedrawForDependentView();
    void                                    scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType);
    void                                    updateOverrides();
    void                                    updateOptionSensitivity();
    void                                    removeOverrides();
    void                                    updateDisplayNameAndIcon();

    void                                    updateRangeFilterOverrides(RimCellRangeFilter* changedRangeFilter);
    void                                    applyRangeFilterCollectionByUserChoice();


protected:  // Pdm overridden methods
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &m_name; }
    virtual caf::PdmFieldHandle*            objectToggleField()     { return &m_isActive; }

private:
    void                                    updateCameraLink();
    void                                    updateTimeStepLink();
    void                                    updateResultColorsControl();
    void                                    updateLegendDefinitions();

    bool                                    isMasterAndDepViewDifferentType();
    bool                                    isRangeFilterControlPossible();
    bool                                    isPropertyFilterControlPossible();
    bool                                    isRangeFilterMappingApliccable();

    RimEclipseView*                         managedEclipseView();
    RimGeoMechView*                         managedGeoView();
    static void                             removeOverrides(RimView* view);

    static bool                             askUserToRestoreOriginalRangeFilterCollection(const QString& viewName);
private:
    caf::PdmField<QString>                  m_name;
    caf::PdmPtrField<RimView*>              m_managedView;

    caf::PdmField<bool>                     m_isActive;
    caf::PdmField<bool>                     m_syncCamera;
    caf::PdmField<bool>                     m_showCursor;
    caf::PdmField<bool>                     m_syncTimeStep;


    // Overridden properties
    caf::PdmField<bool>                     m_syncCellResult;
    caf::PdmField<bool>                     m_syncLegendDefinitions;
    
    caf::PdmField<bool>                     m_syncRangeFilters;
    caf::PdmField<bool>                     m_syncVisibleCells;
    caf::PdmField<bool>                     m_syncPropertyFilters;

    QIcon                                   m_originalIcon;
    cvf::ref<RigCaseToCaseCellMapper>       m_caseToCaseCellMapper;
};
