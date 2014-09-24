/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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


#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cvfBase.h"
#include "cvfObject.h"

#include "RimDefines.h"

#include <QDateTime>

class RimReservoirCellResultsStorageEntryInfo;
class RigCaseCellResultsData;
class RifReaderInterface;
class RigMainGrid;

class RimReservoirCellResultsStorage : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimReservoirCellResultsStorage();
    virtual ~RimReservoirCellResultsStorage();

    void                            setCellResults(RigCaseCellResultsData* cellResults);
    RigCaseCellResultsData*         cellResults()  { return m_cellResults; }
    const RigCaseCellResultsData*   cellResults() const  { return m_cellResults; }

    size_t                          storedResultsCount();

    void                            setMainGrid(RigMainGrid* mainGrid);

    void                            setReaderInterface(RifReaderInterface* readerInterface);
    RifReaderInterface*             readerInterface();

    void                            computeDepthRelatedResults();
    bool                            isDataPresent(size_t scalarResultIndex) const;

    size_t                          findOrLoadScalarResultForTimeStep(RimDefines::ResultCatType type, const QString& resultName, size_t timeStepIndex);
    size_t                          findOrLoadScalarResult(RimDefines::ResultCatType type, const QString& resultName);
    size_t                          findOrLoadScalarResult(const QString& resultName); ///< Simplified search. Assumes unique names across types.

protected:
    // Overridden methods from PdmObject
    virtual void                    setupBeforeSave();

private:
    void                            computeSOILForTimeStep(size_t timeStepIndex);
    void                            computeRiTransComponent(const QString& riTransComponentResultName);
    void                            computeNncCombRiTrans();

    void                            computeRiMULTComponent(const QString& riMultCompName);
    void                            computeNncCombRiMULT();
    void                            computeRiTRANSbyAreaComponent(const QString& riTransByAreaCompResultName);
    void                            computeNncCombRiTRANSbyArea();

    double                          darchysValue();

    QString                         getValidCacheFileName();
    QString                         getCacheDirectoryPath();
    // Fields
    caf::PdmField<QString>          m_resultCacheFileName;
    caf::PdmPointersField<RimReservoirCellResultsStorageEntryInfo*> 
                                    m_resultCacheMetaData;

    cvf::ref<RifReaderInterface>    m_readerInterface;
    RigCaseCellResultsData*         m_cellResults;
    RigMainGrid*                    m_ownerMainGrid;
};

class RimReservoirCellResultsStorageEntryInfo : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimReservoirCellResultsStorageEntryInfo();
    virtual ~RimReservoirCellResultsStorageEntryInfo();

    caf::PdmField<caf::AppEnum< RimDefines::ResultCatType> > m_resultType;
    caf::PdmField<QString>                                   m_resultName;
    caf::PdmField< std::vector <QDateTime> >                 m_timeStepDates;
    caf::PdmField<qint64>                                    m_filePosition;
};

