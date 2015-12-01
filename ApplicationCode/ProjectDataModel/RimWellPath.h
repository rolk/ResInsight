/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmChildField.h"
#include "cafAppEnum.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    


class RimProject;
class RivWellPathPartMgr;
class RimWellLogFile;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPath : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimWellPath();
    virtual ~RimWellPath();

    void                                setLogFileInfo(RimWellLogFile* logFileInfo);

    virtual caf::PdmFieldHandle*        userDescriptionField();
    virtual caf::PdmFieldHandle*        objectToggleField();

    virtual void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );
    
    caf::PdmField<QString>              name;
  
    caf::PdmField<QString>              filepath;
    caf::PdmField<int>                  wellPathIndexInFile; // -1 means none.

    caf::PdmField<bool>                 showWellPathLabel;
    
    caf::PdmField<bool>                 showWellPath;
    caf::PdmField<cvf::Color3f>         wellPathColor;
    caf::PdmField<double>               wellPathRadiusScaleFactor;

    caf::PdmChildField<RimWellLogFile*> m_wellLogFile;

    RigWellPath*                        wellPathGeometry() { return m_wellPath.p(); }
    RivWellPathPartMgr*                 partMgr();

    bool                                readWellPathFile(QString * errorMessage, RifWellPathAsciiFileReader* asciiReader);
    void                                updateFilePathsFromProjectPath();



private:

    void                                setWellPathGeometry(RigWellPath* wellPathModel) { m_wellPath = wellPathModel; }
    void                                readJsonWellPathFile();
    void                                readAsciiWellPathFile(RifWellPathAsciiFileReader* asciiReader);
    QString                             surveyType() { return m_surveyType; }
    void                                setSurveyType(QString surveyType);

    virtual void                        defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    bool                                isStoredInCache();
    QString                             getCacheFileName();
    QString                             getCacheDirectoryPath();

    virtual void                        setupBeforeSave();
    caf::PdmField<QString>              id;
    caf::PdmField<QString>              sourceSystem;
    caf::PdmField<QString>              utmZone;
    caf::PdmField<QString>              updateDate;
    caf::PdmField<QString>              updateUser;
 
    caf::PdmField<QString>              m_surveyType;

    cvf::ref<RigWellPath>               m_wellPath;
    cvf::ref<RivWellPathPartMgr>        m_wellPathPartMgr;
};
