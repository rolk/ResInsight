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

#include "RimReservoirCellResultsStorage.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigCell.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimTools.h"

#include "cafProgressInfo.h"

#include "cvfGeometryTools.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

CAF_PDM_SOURCE_INIT(RimReservoirCellResultsStorage, "ReservoirCellResultStorage");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage::RimReservoirCellResultsStorage()
    : m_cellResults(NULL), 
    m_ownerMainGrid(NULL)
{
    CAF_PDM_InitObject("Cacher", "", "", "");

    CAF_PDM_InitField(&m_resultCacheFileName, "ResultCacheFileName",  QString(), "UiDummyname", "", "" ,"");
    m_resultCacheFileName.setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_resultCacheMetaData, "ResultCacheEntries", "UiDummyname", "", "", "");
    m_resultCacheMetaData.setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage::~RimReservoirCellResultsStorage()
{
    m_resultCacheMetaData.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// This override populates the metainfo regarding the cell results data in the RigCaseCellResultsData
/// object. This metainfo will then be written to the project file when saving, and thus read on project file open.
/// This method then writes the actual double arrays to the data file in a simple format:
/// MagicNumber<uint32>, Version<uint32>, ResultVariables< Array < TimeStep< CellDataArraySize<uint64>, CellData< Array<double > > > >
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::setupBeforeSave()
{
    m_resultCacheMetaData.deleteAllChildObjects();
    QString newValidCacheFileName = getValidCacheFileName();

    // Delete the storage file

    QFileInfo storageFileInfo(newValidCacheFileName);
    if (storageFileInfo.exists())
    {
        QDir storageDir = storageFileInfo.dir();
        storageDir.remove(storageFileInfo.fileName()); 
    }

    if (!m_cellResults) return;

    const std::vector<RigCaseCellResultsData::ResultInfo>&  resInfo = m_cellResults->infoForEachResultIndex();

    bool hasResultsToStore = false;
    for (size_t rIdx = 0; rIdx < resInfo.size(); ++rIdx) 
    {
        if (resInfo[rIdx].m_needsToBeStored) 
        {
            hasResultsToStore = true; 
            break;
        }
    }

    if(resInfo.size() && hasResultsToStore)
    {
        QDir::root().mkpath(getCacheDirectoryPath());

        QFile cacheFile(newValidCacheFileName);

        if (!cacheFile.open(QIODevice::WriteOnly)) 
        {
            qWarning() << "Saving project: Can't open the cache file : " + newValidCacheFileName; 
            return;
        }

        m_resultCacheFileName = newValidCacheFileName;

        QDataStream stream(&cacheFile);
        stream.setVersion(QDataStream::Qt_4_6);
        stream << (quint32)0xCEECAC4E; // magic number
        stream << (quint32)1; // Version number. Increment if needing to extend the format in ways that can not be handled generically by the reader

        caf::ProgressInfo progInfo(resInfo.size(), "Saving generated and imported properties");

        for (size_t rIdx = 0; rIdx < resInfo.size(); ++rIdx)
        {
            // If there is no data, we do not store anything for the current result variable
            // (Even not the metadata, of cause)
            size_t timestepCount = m_cellResults->cellScalarResults(resInfo[rIdx].m_gridScalarResultIndex).size();

            if (timestepCount && resInfo[rIdx].m_needsToBeStored)
            {
                progInfo.setProgressDescription(resInfo[rIdx].m_resultName);

                // Create and setup the cache information for this result
                RimReservoirCellResultsStorageEntryInfo*  cacheEntry = new RimReservoirCellResultsStorageEntryInfo;
                m_resultCacheMetaData.push_back(cacheEntry);

                cacheEntry->m_resultType = resInfo[rIdx].m_resultType;
                cacheEntry->m_resultName = resInfo[rIdx].m_resultName;
                cacheEntry->m_timeStepDates = resInfo[rIdx].m_timeStepDates;

                // Take note of the file position for fast lookup later
                cacheEntry->m_filePosition = cacheFile.pos();

                // Write all the scalar values for each time step to the stream, 
                // starting with the number of values 
                for (size_t tsIdx = 0; tsIdx < resInfo[rIdx].m_timeStepDates.size() ; ++tsIdx)
                {
                    const std::vector<double>* data = NULL;
                    if (tsIdx < timestepCount)
                    {
                        data = &(m_cellResults->cellScalarResults(resInfo[rIdx].m_gridScalarResultIndex, tsIdx));
                    }

                    if (data && data->size())
                    {

                        stream << (quint64)(data->size());
                        for (size_t cIdx = 0; cIdx < data->size(); ++cIdx)
                        {
                            stream << (*data)[cIdx];
                        }
                    }
                    else
                    {
                        stream << (quint64)0;
                    }
                }
            }

            progInfo.incrementProgress();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimReservoirCellResultsStorage::getValidCacheFileName()
{
    QString cacheFileName;
    if (m_resultCacheFileName().isEmpty())
    {
        QString newCacheDirPath =  getCacheDirectoryPath();
        QUuid guid = QUuid::createUuid();
        cacheFileName = newCacheDirPath + "/" + guid.toString();
    }
    else
    {
        // Make the path correct related to the possibly new project filename
        QString newCacheDirPath =  getCacheDirectoryPath();
        QFileInfo oldCacheFile(m_resultCacheFileName());

        cacheFileName = newCacheDirPath + "/" + oldCacheFile.fileName();
    }
    return cacheFileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimReservoirCellResultsStorage::getCacheDirectoryPath()
{
    QString cacheDirPath = RimTools::getCacheRootDirectoryPathFromProject();
    cacheDirPath += "_cache";
    return cacheDirPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::setReaderInterface(RifReaderInterface* readerInterface)
{
    m_readerInterface = readerInterface;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderInterface* RimReservoirCellResultsStorage::readerInterface()
{
    return m_readerInterface.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsStorage::findOrLoadScalarResult(const QString& resultName)
{
    if (!m_cellResults) return cvf::UNDEFINED_SIZE_T;

    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    scalarResultIndex = this->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, resultName);

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::GENERATED, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::INPUT_PROPERTY, resultName);
    }

    return scalarResultIndex;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsStorage::findOrLoadScalarResult(RimDefines::ResultCatType type, const QString& resultName)
{
    if (!m_cellResults) return cvf::UNDEFINED_SIZE_T;

    size_t scalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T) return cvf::UNDEFINED_SIZE_T;

    // Load dependency data sets

    if (type == RimDefines::STATIC_NATIVE)
    {
        if (resultName == RimDefines::combinedTransmissibilityResultName())
        {
            this->findOrLoadScalarResult(type, "TRANX");
            this->findOrLoadScalarResult(type, "TRANY");
            this->findOrLoadScalarResult(type, "TRANZ");
        }
        else if (resultName == RimDefines::combinedMultResultName())
        {
            this->findOrLoadScalarResult(type, "MULTX");
            this->findOrLoadScalarResult(type, "MULTX-");
            this->findOrLoadScalarResult(type, "MULTY");
            this->findOrLoadScalarResult(type, "MULTY-");
            this->findOrLoadScalarResult(type, "MULTZ");
            this->findOrLoadScalarResult(type, "MULTZ-");
        }
        else if (resultName == RimDefines::combinedRiTranResultName())
        {
            computeRiTransComponent(RimDefines::riTranXResultName());
            computeRiTransComponent(RimDefines::riTranYResultName());
            computeRiTransComponent(RimDefines::riTranZResultName());
            computeNncCombRiTrans();
        }
        else if (resultName == RimDefines::riTranXResultName()
            || resultName == RimDefines::riTranYResultName()
            || resultName == RimDefines::riTranZResultName())
        {
            computeRiTransComponent(resultName);
        }
        else if (resultName == RimDefines::combinedRiMultResultName())
        {
            computeRiMULTComponent(RimDefines::riMultXResultName());
            computeRiMULTComponent(RimDefines::riMultYResultName());
            computeRiMULTComponent(RimDefines::riMultZResultName());
            computeNncCombRiTrans();
            computeNncCombRiMULT();
        }
        else if (resultName == RimDefines::riMultXResultName()
              || resultName == RimDefines::riMultYResultName()
              || resultName == RimDefines::riMultZResultName())
        {
            computeRiMULTComponent(resultName);
        }
        else if (resultName == RimDefines::combinedRiAreaNormTranResultName())
        {
            computeRiTRANSbyAreaComponent(RimDefines::riAreaNormTranXResultName());
            computeRiTRANSbyAreaComponent(RimDefines::riAreaNormTranYResultName());
            computeRiTRANSbyAreaComponent(RimDefines::riAreaNormTranZResultName());
            computeNncCombRiTRANSbyArea();
        }
        else if (resultName == RimDefines::riAreaNormTranXResultName()
              || resultName == RimDefines::riAreaNormTranYResultName()
              || resultName == RimDefines::riAreaNormTranZResultName())
        {
            computeRiTRANSbyAreaComponent(resultName);
        }
    }


    if (isDataPresent(scalarResultIndex))
    {
        return scalarResultIndex;
    }

    if (resultName == "SOIL")
    {
        if (m_cellResults->mustBeCalculated(scalarResultIndex))
        {
            // Trigger loading of SWAT, SGAS to establish time step count if no data has been loaded from file at this point
            findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SWAT");
            findOrLoadScalarResult(RimDefines::DYNAMIC_NATIVE, "SGAS");

            m_cellResults->cellScalarResults(scalarResultIndex).resize(m_cellResults->maxTimeStepCount());
            for (size_t timeStepIdx = 0; timeStepIdx < m_cellResults->maxTimeStepCount(); timeStepIdx++)
            {
                std::vector<double>& values = m_cellResults->cellScalarResults(scalarResultIndex)[timeStepIdx];
                if (values.size() == 0)
                {
                    computeSOILForTimeStep(timeStepIdx);
                }
            }

            return scalarResultIndex;
        }
    }

    if (type == RimDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if (m_readerInterface.notNull())
    {
        // Add one more result to result container
        size_t timeStepCount = m_cellResults->infoForEachResultIndex()[scalarResultIndex].m_timeStepDates.size();

        bool resultLoadingSucess = true;

        if (type == RimDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            m_cellResults->cellScalarResults(scalarResultIndex).resize(timeStepCount);

            size_t i;
            for (i = 0; i < timeStepCount; i++)
            {
                std::vector<double>& values = m_cellResults->cellScalarResults(scalarResultIndex)[i];
                if (!m_readerInterface->dynamicResult(resultName, RifReaderInterface::MATRIX_RESULTS, i, &values))
                {
                    resultLoadingSucess = false;
                }
            }
        }
        else if (type == RimDefines::STATIC_NATIVE)
        {
            m_cellResults->cellScalarResults(scalarResultIndex).resize(1);

            std::vector<double>& values = m_cellResults->cellScalarResults(scalarResultIndex)[0];
            if (!m_readerInterface->staticResult(resultName, RifReaderInterface::MATRIX_RESULTS, &values))
            {
                resultLoadingSucess = false;
            }
        }

        if (!resultLoadingSucess)
        {
            // Remove last scalar result because loading of result failed
            m_cellResults->cellScalarResults(scalarResultIndex).clear();
        }
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// This method is intended to be used for multicase cross statistical calculations, when 
/// we need process one timestep at a time, freeing memory as we go.
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsStorage::findOrLoadScalarResultForTimeStep(RimDefines::ResultCatType type, const QString& resultName, size_t timeStepIndex)
{
    if (!m_cellResults) return cvf::UNDEFINED_SIZE_T;

    // Special handling for SOIL
    if (type == RimDefines::DYNAMIC_NATIVE && resultName.toUpper() == "SOIL")
    {
        size_t soilScalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);

        if (m_cellResults->mustBeCalculated(soilScalarResultIndex))
        {
            std::vector<double>& values = m_cellResults->cellScalarResults(soilScalarResultIndex)[timeStepIndex];
            if (values.size() == 0)
            {
                computeSOILForTimeStep(timeStepIndex);
            }

            return soilScalarResultIndex;
        }
    }

    size_t scalarResultIndex = m_cellResults->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T) return cvf::UNDEFINED_SIZE_T;

    if (type == RimDefines::GENERATED)
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if (m_readerInterface.notNull())
    {
        size_t timeStepCount = m_cellResults->infoForEachResultIndex()[scalarResultIndex].m_timeStepDates.size();

        bool resultLoadingSucess = true;

        if (type == RimDefines::DYNAMIC_NATIVE && timeStepCount > 0)
        {
            m_cellResults->cellScalarResults(scalarResultIndex).resize(timeStepCount);

            std::vector<double>& values = m_cellResults->cellScalarResults(scalarResultIndex)[timeStepIndex];
            if (values.size() == 0)
            {
                if (!m_readerInterface->dynamicResult(resultName, RifReaderInterface::MATRIX_RESULTS, timeStepIndex, &values))
                {
                    resultLoadingSucess = false;
                }
            }
        }
        else if (type == RimDefines::STATIC_NATIVE)
        {
            m_cellResults->cellScalarResults(scalarResultIndex).resize(1);

            std::vector<double>& values = m_cellResults->cellScalarResults(scalarResultIndex)[0];
            if (!m_readerInterface->staticResult(resultName, RifReaderInterface::MATRIX_RESULTS, &values))
            {
                resultLoadingSucess = false;
            }
        }

        if (!resultLoadingSucess)
        {
            // Error logging
            CVF_ASSERT(false);
        }
    }

    return scalarResultIndex;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeSOILForTimeStep(size_t timeStepIndex)
{
    size_t scalarIndexSWAT = findOrLoadScalarResultForTimeStep(RimDefines::DYNAMIC_NATIVE, "SWAT", timeStepIndex);
    size_t scalarIndexSGAS = findOrLoadScalarResultForTimeStep(RimDefines::DYNAMIC_NATIVE, "SGAS", timeStepIndex);

    // Early exit if none of SWAT or SGAS is present
    if (scalarIndexSWAT == cvf::UNDEFINED_SIZE_T && scalarIndexSGAS == cvf::UNDEFINED_SIZE_T)
    {
        return;
    }

    CVF_ASSERT(m_cellResults);

    size_t soilResultValueCount = 0;
    size_t soilTimeStepCount = 0;

    if (scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
    {
        std::vector<double>& swatForTimeStep = m_cellResults->cellScalarResults(scalarIndexSWAT, timeStepIndex);
        if (swatForTimeStep.size() > 0)
        {
            soilResultValueCount = swatForTimeStep.size();
            soilTimeStepCount = m_cellResults->infoForEachResultIndex()[scalarIndexSWAT].m_timeStepDates.size();
        }
    }

    if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T)
    {
        std::vector<double>& sgasForTimeStep = m_cellResults->cellScalarResults(scalarIndexSGAS, timeStepIndex);
        if (sgasForTimeStep.size() > 0)
        {
            soilResultValueCount = qMax(soilResultValueCount, sgasForTimeStep.size());

            size_t sgasTimeStepCount = m_cellResults->infoForEachResultIndex()[scalarIndexSGAS].m_timeStepDates.size();
            soilTimeStepCount = qMax(soilTimeStepCount, sgasTimeStepCount);
        }
    }
    
    // Make sure memory is allocated for the new SOIL results

    size_t soilResultScalarIndex = m_cellResults->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, "SOIL");
    m_cellResults->cellScalarResults(soilResultScalarIndex).resize(soilTimeStepCount);

    if (m_cellResults->cellScalarResults(soilResultScalarIndex, timeStepIndex).size() > 0)
    {
        // Data is computed and allocated, nothing more to do
        return;
    }

    m_cellResults->cellScalarResults(soilResultScalarIndex, timeStepIndex).resize(soilResultValueCount);

    std::vector<double>* swatForTimeStep = NULL;
    std::vector<double>* sgasForTimeStep = NULL;

    if (scalarIndexSWAT != cvf::UNDEFINED_SIZE_T)
    {
        swatForTimeStep = &(m_cellResults->cellScalarResults(scalarIndexSWAT, timeStepIndex));
        if (swatForTimeStep->size() == 0)
        {
            swatForTimeStep = NULL;
        }
    }

    if (scalarIndexSGAS != cvf::UNDEFINED_SIZE_T)
    {
        sgasForTimeStep = &(m_cellResults->cellScalarResults(scalarIndexSGAS, timeStepIndex));
        if (sgasForTimeStep->size() == 0)
        {
            sgasForTimeStep = NULL;
        }
    }

    std::vector<double>& soilForTimeStep = m_cellResults->cellScalarResults(soilResultScalarIndex, timeStepIndex);

#pragma omp parallel for
    for (int idx = 0; idx < static_cast<int>(soilResultValueCount); idx++)
    {
        double soilValue = 1.0;
        if (sgasForTimeStep)
        {
            soilValue -= sgasForTimeStep->at(idx);
        }

        if (swatForTimeStep)
        {
            soilValue -= swatForTimeStep->at(idx);
        }

        soilForTimeStep[idx] = soilValue;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeDepthRelatedResults()
{
    if (!m_cellResults) return;

    size_t depthResultGridIndex  = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DEPTH");
    size_t dxResultGridIndex     = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DX");
    size_t dyResultGridIndex     = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DY");
    size_t dzResultGridIndex     = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "DZ");
    size_t topsResultGridIndex   = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "TOPS");
    size_t bottomResultGridIndex = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "BOTTOM");

    bool computeDepth = false;
    bool computeDx = false;
    bool computeDy = false;
    bool computeDz = false;
    bool computeTops = false;
    bool computeBottom = false;

    size_t resultValueCount = m_ownerMainGrid->cells().size();

    if (depthResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        depthResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DEPTH", false, resultValueCount);
        computeDepth = true;
    }

    if (dxResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dxResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DX", false, resultValueCount);
        computeDx = true;
    }

    if (dyResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dyResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DY", false, resultValueCount);
        computeDy = true;
    }

    if (dzResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        dzResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "DZ", false, resultValueCount);
        computeDz = true;
    }

    if (topsResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        topsResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "TOPS", false, resultValueCount);
        computeTops = true;
    }

    if (bottomResultGridIndex == cvf::UNDEFINED_SIZE_T)
    {
        bottomResultGridIndex = m_cellResults->addStaticScalarResult(RimDefines::STATIC_NATIVE, "BOTTOM", false, resultValueCount);
        computeBottom = true;
    }

    std::vector< std::vector<double> >& depth   = m_cellResults->cellScalarResults(depthResultGridIndex);
    std::vector< std::vector<double> >& dx      = m_cellResults->cellScalarResults(dxResultGridIndex);
    std::vector< std::vector<double> >& dy      = m_cellResults->cellScalarResults(dyResultGridIndex);
    std::vector< std::vector<double> >& dz      = m_cellResults->cellScalarResults(dzResultGridIndex);
    std::vector< std::vector<double> >& tops    = m_cellResults->cellScalarResults(topsResultGridIndex);
    std::vector< std::vector<double> >& bottom  = m_cellResults->cellScalarResults(bottomResultGridIndex);

    size_t cellIdx = 0;
    for (cellIdx = 0; cellIdx < m_ownerMainGrid->cells().size(); cellIdx++)
    {
        const RigCell& cell = m_ownerMainGrid->cells()[cellIdx];

        if (computeDepth)
        {
            depth[0][cellIdx] = cvf::Math::abs(cell.center().z());
        }

        if (computeDx)
        {
            cvf::Vec3d cellWidth = cell.faceCenter(cvf::StructGridInterface::NEG_I) - cell.faceCenter(cvf::StructGridInterface::POS_I);
            dx[0][cellIdx] =  cvf::Math::abs(cellWidth.x());
        }

        if (computeDy)
        {
            cvf::Vec3d cellWidth = cell.faceCenter(cvf::StructGridInterface::NEG_J) - cell.faceCenter(cvf::StructGridInterface::POS_J);
            dy[0][cellIdx] =  cvf::Math::abs(cellWidth.y());
        }

        if (computeDz)
        {
            cvf::Vec3d cellWidth = cell.faceCenter(cvf::StructGridInterface::NEG_K) - cell.faceCenter(cvf::StructGridInterface::POS_K);
            dz[0][cellIdx] = cvf::Math::abs(cellWidth.z());
        }

        if (computeTops)
        {
            tops[0][cellIdx] = cvf::Math::abs(cell.faceCenter(cvf::StructGridInterface::NEG_K).z());
        }

        if (computeBottom)
        {
            bottom[0][cellIdx] = cvf::Math::abs(cell.faceCenter(cvf::StructGridInterface::POS_K).z());
        }
    }
}

namespace RigTransmissibilityCalcTools
{
    void calculateConnectionGeometry(const RigCell& c1, const RigCell& c2, const std::vector<cvf::Vec3d>& nodes,
        cvf::StructGridInterface::FaceType faceId, cvf::Vec3d* faceAreaVec)
    {
        CVF_TIGHT_ASSERT(faceAreaVec);

        *faceAreaVec = cvf::Vec3d::ZERO;

        std::vector<size_t> polygon;
        std::vector<cvf::Vec3d> intersections;
        caf::SizeTArray4 face1;
        caf::SizeTArray4 face2;
        c1.faceIndices(faceId, &face1);
        c2.faceIndices(cvf::StructGridInterface::oppositeFace(faceId), &face2);

        bool foundOverlap = cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads(
            &polygon,
            &intersections,
            (cvf::EdgeIntersectStorage<size_t>*)NULL,
            cvf::wrapArrayConst(&nodes),
            face1.data(),
            face2.data(),
            1e-6);


        if (foundOverlap)
        {
            std::vector<cvf::Vec3d> realPolygon;

            for (size_t pIdx = 0; pIdx < polygon.size(); ++pIdx)
            {
                if (polygon[pIdx] < nodes.size())
                    realPolygon.push_back(nodes[polygon[pIdx]]);
                else
                    realPolygon.push_back(intersections[polygon[pIdx] - nodes.size()]);
            }

            // Polygon area vector

            *faceAreaVec = cvf::GeometryTools::polygonAreaNormal3D(realPolygon);

        }

    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    double halfCellTransmissibility(double perm, double ntg, const cvf::Vec3d& centerToFace, const cvf::Vec3d& faceAreaVec)
    {
        return perm*ntg*(faceAreaVec*centerToFace) / (centerToFace*centerToFace);
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    double newtran(double cdarchy, double mult, double halfCellTrans, double neighborHalfCellTrans)
    {
        if (cvf::Math::abs(halfCellTrans) < 1e-15 || cvf::Math::abs(neighborHalfCellTrans) < 1e-15)
        {
            return 0.0;
        }

        double result = cdarchy * mult / ((1 / halfCellTrans) + (1 / neighborHalfCellTrans));
        CVF_TIGHT_ASSERT(result == result);
        return result;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    typedef size_t(*ResultIndexFunction)(const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex);

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------

    size_t directReservoirCellIndex(const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex)
    {
        return reservoirCellIndex;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------

    size_t reservoirActiveCellIndex(const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex)
    {
        return activeCellinfo->cellResultIndex(reservoirCellIndex);
    }
}

using namespace RigTransmissibilityCalcTools;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeRiTransComponent(const QString& riTransComponentResultName)
{
    if (!m_cellResults) return;

    // Set up which component to compute

    cvf::StructGridInterface::FaceType faceId = cvf::StructGridInterface::NO_FACE;
    QString permCompName;

    if (riTransComponentResultName == RimDefines::riTranXResultName())
    {
        permCompName = "PERMX";
        faceId = cvf::StructGridInterface::POS_I;
    }
    else if (riTransComponentResultName == RimDefines::riTranYResultName())
    {
        permCompName = "PERMY";
        faceId = cvf::StructGridInterface::POS_J;
    }
    else if (riTransComponentResultName == RimDefines::riTranZResultName())
    {
        permCompName = "PERMZ";
        faceId = cvf::StructGridInterface::POS_K;
    }
    else
    {
        CVF_ASSERT(false);
    }

    double cdarchy = darchysValue();

    // Get the needed result indices we depend on

    size_t permResultIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, permCompName);
    size_t ntgResultIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "NTG");

    // Get the result index of the output

    size_t riTransResultIdx = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, riTransComponentResultName);
    CVF_ASSERT(riTransResultIdx != cvf::UNDEFINED_SIZE_T);

    // Get the result count, to handle that one of them might be globally defined

    size_t permxResultValueCount = m_cellResults->cellScalarResults(permResultIdx)[0].size();
    size_t ntgResultValueCount = m_cellResults->cellScalarResults(ntgResultIdx)[0].size();

    size_t resultValueCount = CVF_MIN(permxResultValueCount, ntgResultValueCount);

    // Get all the actual result values

    std::vector<double> & permResults = m_cellResults->cellScalarResults(permResultIdx)[0];
    std::vector<double> & ntgResults = m_cellResults->cellScalarResults(ntgResultIdx)[0];
    std::vector<double> & riTransResults = m_cellResults->cellScalarResults(riTransResultIdx)[0];

    // Set up output container to correct number of results

    riTransResults.resize(resultValueCount);

    // Prepare how to index the result values:

    bool isPermUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(permResultIdx);
    bool isNtgUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(ntgResultIdx);
    bool isTransUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(riTransResultIdx);

    // Set up result index function pointers

    ResultIndexFunction riTranIdxFunc = isTransUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
    ResultIndexFunction permIdxFunc = isPermUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
    ResultIndexFunction ntgIdxFunc = isNtgUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;


    const RigActiveCellInfo* activeCellInfo = m_cellResults->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes = m_ownerMainGrid->nodes();
    bool isFaceNormalsOutwards = m_ownerMainGrid->isFaceNormalsOutwards();

    for (size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->cells().size(); nativeResvCellIndex++)
    {
        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t tranResIdx = (*riTranIdxFunc)(activeCellInfo, nativeResvCellIndex);

        if (tranResIdx == cvf::UNDEFINED_SIZE_T) continue;

        const RigCell& nativeCell = m_ownerMainGrid->cells()[nativeResvCellIndex];
        RigGridBase* grid = nativeCell.hostGrid();

        size_t gridLocalNativeCellIndex = nativeCell.gridLocalCellIndex();

        size_t i, j, k, gridLocalNeighborCellIdx;

        grid->ijkFromCellIndex(gridLocalNativeCellIndex, &i, &j, &k);

        if (grid->cellIJKNeighbor(i, j, k, faceId, &gridLocalNeighborCellIdx))
        {
            size_t neighborResvCellIdx = grid->reservoirCellIndex(gridLocalNeighborCellIdx);
            const RigCell& neighborCell = m_ownerMainGrid->cells()[neighborResvCellIdx];

            // Do nothing if neighbor cell has no results
            size_t neighborCellPermResIdx = (*permIdxFunc)(activeCellInfo, neighborResvCellIdx);
            if (neighborCellPermResIdx == cvf::UNDEFINED_SIZE_T) continue;

            // Connection geometry

            const RigFault* fault = grid->mainGrid()->findFaultFromCellIndexAndCellFace(nativeResvCellIndex, faceId);
            bool isOnFault = fault;


            cvf::Vec3d faceAreaVec;
            cvf::Vec3d faceCenter;

            if (isOnFault)
            {
                calculateConnectionGeometry(nativeCell, neighborCell, nodes, faceId, &faceAreaVec);
            }
            else
            {
                
                faceAreaVec = nativeCell.faceNormalWithAreaLenght(faceId);
            }

            if (!isFaceNormalsOutwards) faceAreaVec = -faceAreaVec;

            double halfCellTrans = 0;
            double neighborHalfCellTrans = 0;

            // Native cell half cell transm
            {
                cvf::Vec3d centerToFace = nativeCell.faceCenter(faceId) - nativeCell.center();

                size_t permResIdx = (*permIdxFunc)(activeCellInfo, nativeResvCellIndex);
                double perm = permResults[permResIdx];

                double ntg = 1.0;
                if (faceId != cvf::StructGridInterface::POS_K)
                {
                    size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, nativeResvCellIndex);
                    ntg = ntgResults[ntgResIdx];
                }

                halfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, faceAreaVec);
            }

            // Neighbor cell half cell transm
            {
                cvf::Vec3d centerToFace = neighborCell.faceCenter(cvf::StructGridInterface::oppositeFace(faceId)) - neighborCell.center();

                double perm = permResults[neighborCellPermResIdx];

                double ntg = 1.0;
                if (faceId != cvf::StructGridInterface::POS_K)
                {
                    size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, neighborResvCellIdx);
                    ntg = ntgResults[ntgResIdx];
                }

                neighborHalfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, -faceAreaVec);
            }

            riTransResults[tranResIdx] = newtran(cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans);
        }

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeNncCombRiTrans()
{
    if (!m_cellResults) return;

    size_t riCombTransScalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedRiTranResultName());
    if (m_ownerMainGrid->nncData()->connectionScalarResult(riCombTransScalarResultIndex)) return;

    double cdarchy = darchysValue();

    // Get the needed result indices we depend on

    size_t permXResultIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMX");
    size_t permYResultIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMY");
    size_t permZResultIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "PERMZ");

    size_t ntgResultIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, "NTG");

    // Get the result count, to handle that one of them might be globally defined

    size_t permxResultValueCount = m_cellResults->cellScalarResults(permXResultIdx)[0].size();
    size_t ntgResultValueCount = m_cellResults->cellScalarResults(ntgResultIdx)[0].size();

    size_t resultValueCount = CVF_MIN(permxResultValueCount, ntgResultValueCount);

    // Get all the actual result values

    std::vector<double> & permXResults = m_cellResults->cellScalarResults(permXResultIdx)[0];
    std::vector<double> & permYResults = m_cellResults->cellScalarResults(permYResultIdx)[0];
    std::vector<double> & permZResults = m_cellResults->cellScalarResults(permZResultIdx)[0];
    std::vector<double> & ntgResults = m_cellResults->cellScalarResults(ntgResultIdx)[0];
    std::vector<double> & riCombTransResults = m_ownerMainGrid->nncData()->makeConnectionScalarResult(riCombTransScalarResultIndex);


    // Prepare how to index the result values:

    bool isPermXUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(permXResultIdx);
    bool isPermYUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(permYResultIdx);
    bool isPermZUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(permZResultIdx);
    bool isNtgUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(ntgResultIdx);
 

    // Set up result index function pointers

    ResultIndexFunction permXIdxFunc = isPermXUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
    ResultIndexFunction permYIdxFunc = isPermYUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
    ResultIndexFunction permZIdxFunc = isPermZUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
    ResultIndexFunction ntgIdxFunc = isNtgUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;


    const RigActiveCellInfo* activeCellInfo = m_cellResults->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes = m_ownerMainGrid->nodes();
    bool isFaceNormalsOutwards = m_ownerMainGrid->isFaceNormalsOutwards();

    // NNC calculation
    std::vector<RigConnection>& nncConnections = m_ownerMainGrid->nncData()->connections();
    for (size_t connIdx = 0; connIdx < nncConnections.size(); connIdx++)
    {
        size_t nativeResvCellIndex = nncConnections[connIdx].m_c1GlobIdx;
        size_t neighborResvCellIdx = nncConnections[connIdx].m_c2GlobIdx;
        cvf::StructGridInterface::FaceType faceId = nncConnections[connIdx].m_c1Face;

        ResultIndexFunction permIdxFunc = NULL;
        std::vector<double> * permResults;

        switch (faceId)
        {
        case cvf::StructGridInterface::POS_I:
        case cvf::StructGridInterface::NEG_I:
            permIdxFunc = permXIdxFunc;
            permResults = &permXResults;
            break;
        case cvf::StructGridInterface::POS_J:
        case cvf::StructGridInterface::NEG_J:
            permIdxFunc = permYIdxFunc;
            permResults = &permYResults;
            break;
        case cvf::StructGridInterface::POS_K:
        case cvf::StructGridInterface::NEG_K:
            permIdxFunc = permZIdxFunc;
            permResults = &permZResults;
            break;
        }

        if (!permIdxFunc) continue;

        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t nativeCellPermResIdx = (*permIdxFunc)(activeCellInfo, nativeResvCellIndex);
        if (nativeCellPermResIdx == cvf::UNDEFINED_SIZE_T) continue;

        // Do nothing if neighbor cell has no results
        size_t neighborCellPermResIdx = (*permIdxFunc)(activeCellInfo, neighborResvCellIdx);
        if (neighborCellPermResIdx == cvf::UNDEFINED_SIZE_T) continue;


        const RigCell& nativeCell = m_ownerMainGrid->cells()[nativeResvCellIndex];
        const RigCell& neighborCell = m_ownerMainGrid->cells()[neighborResvCellIdx];


        // Connection geometry

        cvf::Vec3d faceAreaVec = cvf::Vec3d::ZERO;;
        cvf::Vec3d faceCenter = cvf::Vec3d::ZERO;;

        // Polygon center
        const std::vector<cvf::Vec3d>& realPolygon = nncConnections[connIdx].m_polygon;
        for (size_t pIdx = 0; pIdx < realPolygon.size(); ++pIdx)
        {
            faceCenter += realPolygon[pIdx];
        }

        faceCenter *= 1.0 / realPolygon.size();

        // Polygon area vector

        faceAreaVec = cvf::GeometryTools::polygonAreaNormal3D(realPolygon);

        if (!isFaceNormalsOutwards) faceAreaVec = -faceAreaVec;

        double halfCellTrans = 0;
        double neighborHalfCellTrans = 0;

        // Native cell half cell transm
        {
            cvf::Vec3d centerToFace = nativeCell.faceCenter(faceId) - nativeCell.center();

            double perm = (*permResults)[nativeCellPermResIdx];

            double ntg = 1.0;
            if (faceId != cvf::StructGridInterface::POS_K)
            {
                size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, nativeResvCellIndex);
                ntg = ntgResults[ntgResIdx];
            }

            halfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, faceAreaVec);
        }

        // Neighbor cell half cell transm
        {
            cvf::Vec3d centerToFace = neighborCell.faceCenter(cvf::StructGridInterface::oppositeFace(faceId)) - neighborCell.center();

            double perm = (*permResults)[neighborCellPermResIdx];

            double ntg = 1.0;
            if (faceId != cvf::StructGridInterface::POS_K)
            {
                size_t ntgResIdx = (*ntgIdxFunc)(activeCellInfo, neighborResvCellIdx);
                ntg = ntgResults[ntgResIdx];
            }

            neighborHalfCellTrans = halfCellTransmissibility(perm, ntg, centerToFace, -faceAreaVec);
        }

        double newtranTemp = newtran(cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans);
        riCombTransResults[connIdx] = newtranTemp;
    }

}


double riMult(double transResults, double riTransResults)
{
    if (transResults == HUGE_VAL || riTransResults == HUGE_VAL) return HUGE_VAL;

    // To make 0.0 values give 1.0 in mult value
    if (cvf::Math::abs (riTransResults) < 1e-12)
    {
        if (cvf::Math::abs (transResults)  < 1e-12)
        {
            return 1.0;
        }

        return HUGE_VAL;
    }


    double result = transResults / riTransResults;

    return result;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeRiMULTComponent(const QString& riMultCompName)
{
    if (!m_cellResults) return;

    // Set up which component to compute

    QString riTransCompName;
    QString transCompName;

    if (riMultCompName == RimDefines::riMultXResultName())
    {
        riTransCompName = RimDefines::riTranXResultName();
        transCompName = "TRANX";
    }
    else if (riMultCompName == RimDefines::riMultYResultName())
    {
        riTransCompName = RimDefines::riTranYResultName();
        transCompName = "TRANY";
    }
    else if (riMultCompName == RimDefines::riMultZResultName())
    {
        riTransCompName = RimDefines::riTranZResultName();
        transCompName = "TRANZ";
    }
    else
    {
        CVF_ASSERT(false);
    }

    // Get the needed result indices we depend on

    size_t transResultIdx   = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, transCompName);
    size_t riTransResultIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, riTransCompName);

    // Get the result index of the output

    size_t riMultResultIdx = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, riMultCompName);
    CVF_ASSERT(riMultResultIdx != cvf::UNDEFINED_SIZE_T);

    // Get the result count, to handle that one of them might be globally defined

    CVF_ASSERT(m_cellResults->cellScalarResults(riTransResultIdx)[0].size() == m_cellResults->cellScalarResults(transResultIdx)[0].size());

    size_t resultValueCount = m_cellResults->cellScalarResults(transResultIdx)[0].size();

    // Get all the actual result values

    std::vector<double> & riTransResults = m_cellResults->cellScalarResults(riTransResultIdx)[0];
    std::vector<double> & transResults = m_cellResults->cellScalarResults(transResultIdx)[0];

    std::vector<double> & riMultResults = m_cellResults->cellScalarResults(riMultResultIdx)[0];

    // Set up output container to correct number of results

    riMultResults.resize(resultValueCount);

    const RigActiveCellInfo* activeCellInfo = m_cellResults->activeCellInfo();
        
    for (size_t vIdx = 0; vIdx < transResults.size(); ++vIdx)
    {
        riMultResults[vIdx] = riMult(transResults[vIdx], riTransResults[vIdx]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeNncCombRiMULT()
{
    if (!m_cellResults) return;

    size_t riCombMultScalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedRiMultResultName());
    size_t riCombTransScalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedRiTranResultName());
    size_t combTransScalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedTransmissibilityResultName());

    if (m_ownerMainGrid->nncData()->connectionScalarResult(riCombMultScalarResultIndex)) return;

    std::vector<double> & riMultResults = m_ownerMainGrid->nncData()->makeConnectionScalarResult(riCombMultScalarResultIndex);
    const std::vector<double> * riTransResults = m_ownerMainGrid->nncData()->connectionScalarResult(riCombTransScalarResultIndex);
    const std::vector<double> * transResults = m_ownerMainGrid->nncData()->connectionScalarResult(combTransScalarResultIndex);

    for (size_t nncConIdx = 0; nncConIdx < riMultResults.size(); ++nncConIdx)
    {
        riMultResults[nncConIdx] = riMult((*transResults)[nncConIdx], (*riTransResults)[nncConIdx]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeRiTRANSbyAreaComponent(const QString& riTransByAreaCompResultName)
{
    if (!m_cellResults) return;

    // Set up which component to compute

    cvf::StructGridInterface::FaceType faceId = cvf::StructGridInterface::NO_FACE;
    QString transCompName;

    if (riTransByAreaCompResultName == RimDefines::riAreaNormTranXResultName())
    {
        transCompName = "TRANX";
        faceId = cvf::StructGridInterface::POS_I;
    }
    else if (riTransByAreaCompResultName == RimDefines::riAreaNormTranYResultName())
    {
        transCompName = "TRANY";
        faceId = cvf::StructGridInterface::POS_J;
    }
    else if (riTransByAreaCompResultName == RimDefines::riAreaNormTranZResultName())
    {
        transCompName = "TRANZ";
        faceId = cvf::StructGridInterface::POS_K;
    }
    else
    {
        CVF_ASSERT(false);
    }

    // Get the needed result indices we depend on

    size_t tranCompScResIdx = findOrLoadScalarResult(RimDefines::STATIC_NATIVE, transCompName);

    // Get the result index of the output

    size_t riTranByAreaScResIdx = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, riTransByAreaCompResultName);
    CVF_ASSERT(riTranByAreaScResIdx != cvf::UNDEFINED_SIZE_T);

    // Get the result count, to handle that one of them might be globally defined

    size_t resultValueCount = m_cellResults->cellScalarResults(tranCompScResIdx)[0].size();

    // Get all the actual result values

    std::vector<double> & transResults = m_cellResults->cellScalarResults(tranCompScResIdx)[0];
    std::vector<double> & riTransByAreaResults = m_cellResults->cellScalarResults(riTranByAreaScResIdx)[0];

    // Set up output container to correct number of results

    riTransByAreaResults.resize(resultValueCount);

    // Prepare how to index the result values:

    bool isUsingResIdx = m_cellResults->isUsingGlobalActiveIndex(tranCompScResIdx);
  
    // Set up result index function pointers

    ResultIndexFunction resValIdxFunc = isUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;

    const RigActiveCellInfo* activeCellInfo = m_cellResults->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes = m_ownerMainGrid->nodes();
    bool isFaceNormalsOutwards = m_ownerMainGrid->isFaceNormalsOutwards();

    for (size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->cells().size(); nativeResvCellIndex++)
    {
        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t nativeCellResValIdx = (*resValIdxFunc)(activeCellInfo, nativeResvCellIndex);

        if (nativeCellResValIdx == cvf::UNDEFINED_SIZE_T) continue;

        const RigCell& nativeCell = m_ownerMainGrid->cells()[nativeResvCellIndex];
        RigGridBase* grid = nativeCell.hostGrid();

        size_t gridLocalNativeCellIndex = nativeCell.gridLocalCellIndex();

        size_t i, j, k, gridLocalNeighborCellIdx;

        grid->ijkFromCellIndex(gridLocalNativeCellIndex, &i, &j, &k);

        if (grid->cellIJKNeighbor(i, j, k, faceId, &gridLocalNeighborCellIdx))
        {
            size_t neighborResvCellIdx = grid->reservoirCellIndex(gridLocalNeighborCellIdx);
            const RigCell& neighborCell = m_ownerMainGrid->cells()[neighborResvCellIdx];

            // Connection geometry

            const RigFault* fault = grid->mainGrid()->findFaultFromCellIndexAndCellFace(nativeResvCellIndex, faceId);
            bool isOnFault = fault;

            cvf::Vec3d faceAreaVec;

            if (isOnFault)
            {
                calculateConnectionGeometry(nativeCell, neighborCell, nodes, faceId, &faceAreaVec);
            }
            else
            {
                faceAreaVec = nativeCell.faceNormalWithAreaLenght(faceId);
            }

            double areaOfOverlap = faceAreaVec.length();
            double transCompValue = transResults[nativeCellResValIdx];

            riTransByAreaResults[nativeCellResValIdx] = transCompValue / areaOfOverlap;
        }

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::computeNncCombRiTRANSbyArea()
{
    if (!m_cellResults) return;

    size_t riCombTransByAreaScResIdx = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedRiAreaNormTranResultName());
    size_t combTransScalarResultIndex = m_cellResults->findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedTransmissibilityResultName());

    if (m_ownerMainGrid->nncData()->connectionScalarResult(riCombTransByAreaScResIdx)) return;

    std::vector<double> & riAreaNormTransResults = m_ownerMainGrid->nncData()->makeConnectionScalarResult(riCombTransByAreaScResIdx);
    const std::vector<double> * transResults = m_ownerMainGrid->nncData()->connectionScalarResult(combTransScalarResultIndex);

    const std::vector<RigConnection>& connections = m_ownerMainGrid->nncData()->connections();

    for (size_t nncConIdx = 0; nncConIdx < riAreaNormTransResults.size(); ++nncConIdx)
    {
        const std::vector<cvf::Vec3d>& realPolygon = connections[nncConIdx].m_polygon;
        cvf::Vec3d faceAreaVec =  cvf::GeometryTools::polygonAreaNormal3D(realPolygon);
        double areaOfOverlap = faceAreaVec.length();

        riAreaNormTransResults[nncConIdx] = (*transResults)[nncConIdx] / areaOfOverlap;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::setCellResults(RigCaseCellResultsData* cellResults)
{
    m_cellResults = cellResults;

    if (m_cellResults == NULL) 
        return;

    // Now that we have got the results container, we can finally 
    // Read data from the internal storage and populate it

    if (m_resultCacheFileName().isEmpty()) 
        return;

    // Get the name of the cache name relative to the current project file position
    QString newValidCacheFileName = getValidCacheFileName();

    QFile storageFile(newValidCacheFileName);

    // Warn if we thought we were to find some data on the storage file

    if (!storageFile.exists() && m_resultCacheMetaData.size())
    {
        qWarning() << "Reading stored results: Missing the storage file : " + newValidCacheFileName; 
        return;
    }

    if (!storageFile.open(QIODevice::ReadOnly)) 
    {
        qWarning() << "Reading stored results: Can't open the file : " + newValidCacheFileName; 
        return;
    }

    QDataStream stream(&storageFile);
    stream.setVersion(QDataStream::Qt_4_6);
    quint32 magicNumber = 0;
    quint32 versionNumber = 0;
    stream >> magicNumber;

    if (magicNumber != 0xCEECAC4E)
    {
        qWarning() << "Reading stored results: The storage file has wrong type "; 
        return;
    }

    stream >> versionNumber;
    if (versionNumber > 1 )
    {
        qWarning() << "Reading stored results: The storage file has been written by a newer version of ResInsight"; 
        return;
    }

    caf::ProgressInfo progress(m_resultCacheMetaData.size(), "Reading internally stored results");
    // Fill the object with data from the storage

    for (size_t rIdx = 0; rIdx < m_resultCacheMetaData.size(); ++rIdx)
    {
        RimReservoirCellResultsStorageEntryInfo* resInfo = m_resultCacheMetaData[rIdx];
        size_t resultIndex = m_cellResults->addEmptyScalarResult(resInfo->m_resultType(), resInfo->m_resultName(), true);

        m_cellResults->setTimeStepDates(resultIndex, resInfo->m_timeStepDates());

        progress.setProgressDescription(resInfo->m_resultName);

        for (size_t tsIdx = 0; tsIdx < resInfo->m_timeStepDates().size(); ++tsIdx)
        {
            std::vector<double>* data = NULL;

            data = &(m_cellResults->cellScalarResults(rIdx, tsIdx));

            quint64 cellCount = 0;
            stream >> cellCount;
            data->resize(cellCount, HUGE_VAL);

            for (size_t cIdx = 0; cIdx < cellCount; ++cIdx)
            {
                stream >> (*data)[cIdx];
            }
        }

        progress.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoirCellResultsStorage::setMainGrid(RigMainGrid* mainGrid)
{
    m_ownerMainGrid = mainGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimReservoirCellResultsStorage::storedResultsCount()
{
    return m_resultCacheMetaData.size();
}

//--------------------------------------------------------------------------------------------------
///  If we have any results on any time step, assume we have loaded results
//--------------------------------------------------------------------------------------------------
bool RimReservoirCellResultsStorage::isDataPresent(size_t scalarResultIndex) const
{
    if (scalarResultIndex >= m_cellResults->resultCount())
    {
        return false;
    }

    const std::vector< std::vector<double> > data = m_cellResults->cellScalarResults(scalarResultIndex);

    for (size_t tsIdx = 0; tsIdx < data.size(); ++tsIdx)
    {
        if (data[tsIdx].size())
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimReservoirCellResultsStorage::darchysValue()
{
    // See "Cartesian transmissibility calculations" in the "Eclipse Technical Description"
    //     CDARCY Darcys constant
    //         = 0.00852702 (E300); 0.008527 (ECLIPSE 100) (METRIC)
    //         = 0.00112712 (E300); 0.001127 (ECLIPSE 100) (FIELD)
    //         = 3.6 (LAB)
    //         = 0.00864 (PVT - M)

    double darchy = 0.008527; // (ECLIPSE 100) (METRIC)

    RimEclipseCase* rimCase = NULL;
    this->firstAncestorOfType(rimCase);

    if (rimCase && rimCase->reservoirData())
    {
        RigCaseData::UnitsType unitsType = rimCase->reservoirData()->unitsType();

        if (unitsType == RigCaseData::UNITS_FIELD)
        {
            darchy = 0.001127;
        }
        else if (unitsType == RigCaseData::UNITS_METRIC)
        {
            darchy = 0.008527;
        }
        else if (unitsType == RigCaseData::UNITS_LAB)
        {
            darchy = 3.6;
        }
        else
        {
            darchy = 0.00864; // Assuming (PVT - M)
            CVF_TIGHT_ASSERT(false); // The enum and doc does not state that the PVT-M actually exists, so to trap this in debug
        }
    }

    return darchy;
}



CAF_PDM_SOURCE_INIT(RimReservoirCellResultsStorageEntryInfo, "ResultStorageEntryInfo");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorageEntryInfo::RimReservoirCellResultsStorageEntryInfo()
{
    CAF_PDM_InitObject("Cache Entry", "", "", "");

    CAF_PDM_InitField(&m_resultType, "ResultType",  caf::AppEnum<RimDefines::ResultCatType>(RimDefines::REMOVED), "ResultType", "", "" ,"");
    CAF_PDM_InitField(&m_resultName, "ResultName",  QString(), "ResultName", "", "" ,"");
    CAF_PDM_InitFieldNoDefault(&m_timeStepDates, "TimeSteps", "TimeSteps", "", "" ,"");
    CAF_PDM_InitField(&m_filePosition, "FilePositionDataStart",  qint64(-1), "FilePositionDataStart", "", "" ,"");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorageEntryInfo::~RimReservoirCellResultsStorageEntryInfo()
{

}
