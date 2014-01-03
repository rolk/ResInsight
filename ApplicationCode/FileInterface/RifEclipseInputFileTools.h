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
#include "cvfLibCore.h"
#include <map>

#include <QString>
#include "RifReaderInterface.h"
#include "RigFault.h"


class RigCaseData;
class QFile;


//--------------------------------------------------------------------------------------------------
/// Structure used to cache file position of keywords
//--------------------------------------------------------------------------------------------------
struct RifKeywordAndFilePos
{
    QString keyword;
    qint64  filePos;
};


//==================================================================================================
//
// Class for access to Eclipse "keyword" files using libecl
//
//==================================================================================================
class RifEclipseInputFileTools : public cvf::Object
{
public:
    RifEclipseInputFileTools();
    virtual ~RifEclipseInputFileTools();

    static bool openGridFile(const QString& fileName, RigCaseData* eclipseCase, bool readFaultData);
    
    // Returns map of assigned resultName and Eclipse Keyword.
    static std::map<QString, QString> readProperties(const QString& fileName, RigCaseData* eclipseCase);
    static bool                       readProperty  (const QString& fileName, RigCaseData* eclipseCase, const QString& eclipseKeyWord, const QString& resultName );
    static bool                       readPropertyAtFilePosition (const QString& fileName, RigCaseData* eclipseCase, const QString& eclipseKeyWord, qint64 filePos, const QString& resultName );
    
    
    static void                       readFaultsInGridSection(const QString& fileName, cvf::Collection<RigFault>& faults, std::vector<QString>& filenamesWithFaults);
    static void                       readFaults(const QString& fileName, cvf::Collection<RigFault>& faults, const std::vector< RifKeywordAndFilePos >& fileKeywords);

    static void                       readFaults(QFile &data, qint64 filePos, cvf::Collection<RigFault> &faults, bool* isEditKeywordDetected);
    static void                       findKeywordsOnFile(const QString &fileName, std::vector< RifKeywordAndFilePos >& keywords);

    static const std::vector<QString>& knownPropertyKeywords(); 

    static bool     writePropertyToTextFile(const QString& fileName, RigCaseData* eclipseCase, size_t timeStep, const QString& resultName, const QString& eclipseKeyWord);
    static bool     writeBinaryResultToTextFile(const QString& fileName, RigCaseData* eclipseCase, RifReaderInterface::PorosityModelResultType porosityModel, size_t timeStep, const QString& resultName, const QString& eclipseKeyWord, const double undefinedValue);

    static bool     readFaultsAndParseIncludeStatementsRecursively(QFile& file, qint64 startPos, cvf::Collection<RigFault>& faults, std::vector<QString>& filenamesWithFaults, bool* isEditKeywordDetected);

private:
    static void     writeDataToTextFile(QFile* file, const QString& eclipseKeyWord, const std::vector<double>& resultData);
    static void     findGridKeywordPositions(const std::vector< RifKeywordAndFilePos >& keywords, qint64* coordPos, qint64* zcornPos, qint64* specgridPos, qint64* actnumPos, qint64* mapaxesPos);

    static size_t   findFaultByName(const cvf::Collection<RigFault>& faults, const QString& name);

    static qint64   findKeyword(const QString& keyword, QFile& file, qint64 startPos);

};
