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

#include "RimDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "RivCellSetEnum.h"

class RimViewLink;
class RiuViewer;
class RimView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimViewLinker : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimViewLinker(void);
    virtual ~RimViewLinker(void);

    void                                    setMainView(RimView* view);
    RimView*                                mainView();

    caf::PdmChildArrayField<RimViewLink*>   viewLinks;

    void                                    applyAllOperations();

    void                                    updateTimeStep(RimView* sourceView, int timeStep);
    void                                    updateScaleZ(RimView* source, double scaleZ);
    void                                    allViewsForCameraSync(RimView* source, std::vector<RimView*>& views);

    void                                    updateCellResult();
    void                                    updateRangeFilters();
    void                                    updatePropertyFilters();

    void                                    scheduleGeometryRegenForDepViews(RivCellSetEnum geometryType);
    void                                    scheduleCreateDisplayModelAndRedrawForDependentViews();

    void                                    configureOverrides();
    void                                    removeOverrides();

    void                                    allViews(std::vector<RimView*>& views);

    void                                    updateUiIcon();
    void                                    setNameAndIcon();

    static void                             applyIconEnabledState(caf::PdmObject* obj, const QIcon& icon, bool disable);
    static void                             findNameAndIconFromView(QString* name, QIcon* icon, RimView* view);

public:
    static QString                          displayNameForView(RimView* view);
    RimViewLink*                            viewLinkFromView(RimView* view);

protected:
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &m_name; }
    virtual void                            initAfterRead();

private:
    bool                                    isActive();

private:
    caf::PdmPtrField<RimView*>              m_mainView;
    caf::PdmField<QString>                  m_name;
    QIcon                                   m_originalIcon;
};