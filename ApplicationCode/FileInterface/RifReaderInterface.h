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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfLibCore.h"

#include "cafPdmPointer.h"

#include <QString>
#include <QStringList>
#include <QDateTime>


class RigCaseData;
class RifReaderSettings;


//==================================================================================================
//
// Data interface base class
//
//==================================================================================================
class RifReaderInterface : public cvf::Object
{
public:
    enum PorosityModelResultType
    {
        MATRIX_RESULTS,
        FRACTURE_RESULTS
    };

public:
    RifReaderInterface()            { }
    virtual ~RifReaderInterface()   { }

    void                        setReaderSetting(RifReaderSettings* settings);

    bool                        isFaultImportEnabled();
    bool                        isImportOfCompleteMswDataEnabled();
    bool                        isNNCsEnabled();

    virtual bool                open(const QString& fileName, RigCaseData* eclipseCase) = 0;
    virtual void                close() = 0;
   
    virtual bool                staticResult(const QString& result, PorosityModelResultType matrixOrFracture, std::vector<double>* values) = 0;
    virtual bool                dynamicResult(const QString& result, PorosityModelResultType matrixOrFracture, size_t stepIndex, std::vector<double>* values) = 0;

    virtual std::vector<QDateTime>  timeSteps() { std::vector<QDateTime> timeSteps; return timeSteps; }

    void                        setFilenamesWithFaults(const std::vector<QString>& filenames)   { m_filenamesWithFaults = filenames; }
    std::vector<QString>        filenamesWithFaults()                                           { return m_filenamesWithFaults; }


private:
    std::vector<QString>                m_filenamesWithFaults;
    caf::PdmPointer<RifReaderSettings>  m_settings;
};
