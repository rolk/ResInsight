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

#include "RimCase.h"

#include "cafPdmObjectFactory.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include "RimProject.h"
#include "RiaApplication.h"
#include "RimOilField.h"
#include "RimFormationNamesCollection.h"
#include "RimFormationNames.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimCase, "RimCase");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase::RimCase()
{
    CAF_PDM_InitField(&caseUserDescription, "CaseUserDescription",  QString(), "Case name", "", "" ,"");

    CAF_PDM_InitField(&caseId, "CaseId", -1, "Case ID", "", "" ,"");
    caseId.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&activeFormationNames, "DefaultFormationNames", "Formation Names File", "", "", "");
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase::~RimCase()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimCase::displayModelOffset() const
{
    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCase::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    if(fieldNeedingOptions == &activeFormationNames)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj && proj->activeOilField() && proj->activeOilField()->formationNamesCollection())
        {
            for(RimFormationNames* fnames : proj->activeOilField()->formationNamesCollection()->formationNamesList())
            {
                optionList.push_back(caf::PdmOptionItemInfo(fnames->fileNameWoPath(),
                                                            QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(fnames)),
                                                            false,
                                                            fnames->uiCapability()->uiIcon()));
            }
        }

        if(optionList.size() > 0)
        {
            optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }

    return optionList;
}
