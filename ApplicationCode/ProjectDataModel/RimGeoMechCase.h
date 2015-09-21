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

#include "RimCase.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfObject.h"

class RimGeoMechView;
class RigGeoMechCaseData;
class RifGeoMechReaderInterface;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechCase : public RimCase
{
     CAF_PDM_HEADER_INIT;

public:
    RimGeoMechCase(void);
    virtual ~RimGeoMechCase(void);
    
    void                                    setFileName(const QString& fileName) {m_caseFileName = fileName;}
    QString                                 caseFileName() const  {return m_caseFileName();}
    bool                                    openGeoMechCase(std::string* errorMessage);

    RigGeoMechCaseData*                     geoMechData() { return m_geoMechCaseData.p(); }
    const RigGeoMechCaseData*               geoMechData() const { return m_geoMechCaseData.p(); }

    RimGeoMechView*                         createAndAddReservoirView();

    virtual void                            updateFilePathsFromProjectPath(const QString& projectPath, const QString& oldProjectPath);
    virtual std::vector<RimView*>           views();

    virtual QStringList                     timeStepStrings();


    // Fields:                                        
    caf::PdmChildArrayField<RimGeoMechView*>  geoMechViews;

private:
    virtual void                            initAfterRead();
private:
    cvf::ref<RigGeoMechCaseData>            m_geoMechCaseData;
    caf::PdmField<QString>                  m_caseFileName;
};
