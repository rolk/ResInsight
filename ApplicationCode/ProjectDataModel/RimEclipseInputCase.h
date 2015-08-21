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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RimEclipseCase.h"

class RifReaderInterface;
class RimEclipseInputPropertyCollection;
class RimEclipseInputProperty;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimEclipseInputCase : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseInputCase();
    virtual ~RimEclipseInputCase();

    // Fields
    caf::PdmField<RimEclipseInputPropertyCollection*> m_inputPropertyCollection;

    // File open methods
    void                        openDataFileSet(const QStringList& fileNames);
    void                        loadAndSyncronizeInputProperties();

    void                        removeProperty(RimEclipseInputProperty* inputProperty);

    // RimCase overrides
    virtual bool                openEclipseGridFile(); // Find grid file among file set. Read, Find read and validate property date. Syncronize child property sets.

    // Overrides from RimCase
    virtual QString             locationOnDisc() const;
    virtual QString             gridFileName() const { return m_gridFileName();}

    virtual void                updateFilePathsFromProjectPath(const QString& projectPath, const QString& oldProjectPath);

private:
    cvf::ref<RifReaderInterface> createMockModel(QString modelName);

    // Fields
    caf::PdmField<std::vector<QString> >       m_additionalFileNames;
    caf::PdmField<QString>                     m_gridFileName;

};
