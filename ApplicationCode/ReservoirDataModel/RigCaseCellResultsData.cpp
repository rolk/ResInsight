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

#include "RigCaseCellResultsData.h"

#include "RigMainGrid.h"
#include "RigStatisticsDataCache.h"
#include "RigStatisticsMath.h"
#include "RigStatisticsCalculator.h"

#include <QDateTime>
#include <math.h>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData::RigCaseCellResultsData(RigMainGrid* ownerGrid)
{
    CVF_ASSERT(ownerGrid != NULL);
    m_ownerMainGrid = ownerGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setMainGrid(RigMainGrid* ownerGrid)
{
    m_ownerMainGrid = ownerGrid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::minMaxCellScalarValues(size_t scalarResultIndex, double& min, double& max)
{
    m_statisticsDataCache[scalarResultIndex]->minMaxCellScalarValues(min, max);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::minMaxCellScalarValues(size_t scalarResultIndex, size_t timeStepIndex, double& min, double& max)
{
    m_statisticsDataCache[scalarResultIndex]->minMaxCellScalarValues(timeStepIndex, min, max);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::posNegClosestToZero(size_t scalarResultIndex, double& pos, double& neg)
{
    m_statisticsDataCache[scalarResultIndex]->posNegClosestToZero(pos, neg);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::posNegClosestToZero(size_t scalarResultIndex, size_t timeStepIndex, double& pos, double& neg)
{
    m_statisticsDataCache[scalarResultIndex]->posNegClosestToZero(timeStepIndex, pos, neg);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigCaseCellResultsData::cellScalarValuesHistogram(size_t scalarResultIndex)
{
    return m_statisticsDataCache[scalarResultIndex]->cellScalarValuesHistogram();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::p10p90CellScalarValues(size_t scalarResultIndex, double& p10, double& p90)
{
    m_statisticsDataCache[scalarResultIndex]->p10p90CellScalarValues(p10, p90);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::meanCellScalarValues(size_t scalarResultIndex, double& meanValue)
{
    m_statisticsDataCache[scalarResultIndex]->meanCellScalarValues(meanValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::resultCount() const
{
	return m_cellScalarResults.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::timeStepCount(size_t scalarResultIndex) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

    return m_cellScalarResults[scalarResultIndex].size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector< std::vector<double> > & RigCaseCellResultsData::cellScalarResults( size_t scalarResultIndex ) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

    return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::vector<double> > & RigCaseCellResultsData::cellScalarResults( size_t scalarResultIndex )
{
	CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());

	return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double>& RigCaseCellResultsData::cellScalarResults(size_t scalarResultIndex, size_t timeStepIndex)
{
    CVF_TIGHT_ASSERT(scalarResultIndex < resultCount());
    CVF_TIGHT_ASSERT(timeStepIndex < m_cellScalarResults[scalarResultIndex].size());

    return m_cellScalarResults[scalarResultIndex][timeStepIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCaseCellResultsData::cellScalarResult( size_t scalarResultIndex, size_t timeStepIndex, size_t resultValueIndex)
{
    if (scalarResultIndex < resultCount() &&
        timeStepIndex < m_cellScalarResults[scalarResultIndex].size() &&
        resultValueIndex != cvf::UNDEFINED_SIZE_T &&
        resultValueIndex < m_cellScalarResults[scalarResultIndex][timeStepIndex].size())
    {
        return m_cellScalarResults[scalarResultIndex][timeStepIndex][resultValueIndex];
    }
    else
    {
        return HUGE_VAL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findScalarResultIndex(RimDefines::ResultCatType type, const QString& resultName) const
{
    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); it++)
    {
        if (it->m_resultType == type && it->m_resultName == resultName)
        {
            return it->m_gridScalarResultIndex;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findScalarResultIndex(const QString& resultName) const
{
    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    scalarResultIndex = this->findScalarResultIndex(RimDefines::STATIC_NATIVE, resultName);

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::DYNAMIC_NATIVE, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::GENERATED, resultName);
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->findScalarResultIndex(RimDefines::INPUT_PROPERTY, resultName);
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// Adds an empty scalar set, and returns the scalarResultIndex to it.
/// if resultName already exists, it returns the scalarResultIndex to the existing result.
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::addEmptyScalarResult(RimDefines::ResultCatType type, const QString& resultName, bool needsToBeStored)
{
    size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

    scalarResultIndex = this->findScalarResultIndex(type, resultName);
    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        scalarResultIndex = this->resultCount();
        m_cellScalarResults.push_back(std::vector<std::vector<double> >());
        ResultInfo resInfo(type, needsToBeStored, false, resultName, scalarResultIndex);
        m_resultInfos.push_back(resInfo);

        // Create statistics calculator and add cache object
        if (resultName == RimDefines::combinedTransmissibilityResultName())
        {
            size_t tranX = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANX");
            size_t tranY = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANY");
            size_t tranZ = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANZ");

            cvf::ref<RigMultipleDatasetStatCalc> calc = new RigMultipleDatasetStatCalc();
            calc->addStatisticsCalculator(new RigNativeStatCalc(this, tranX));
            calc->addStatisticsCalculator(new RigNativeStatCalc(this, tranY));
            calc->addStatisticsCalculator(new RigNativeStatCalc(this, tranZ));

            cvf::ref<RigStatisticsDataCache> dataCache = new RigStatisticsDataCache(calc.p());
            m_statisticsDataCache.push_back(dataCache.p());
        }
        else if (resultName == RimDefines::combinedMultResultName())
        {
            cvf::ref<RigMultipleDatasetStatCalc> calc = new RigMultipleDatasetStatCalc();

            {
                size_t scalarIdx = findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTX");
                if (scalarIdx != cvf::UNDEFINED_SIZE_T) calc->addStatisticsCalculator(new RigNativeStatCalc(this, scalarIdx));
            }
            {
                size_t scalarIdx = findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTX-");
                if (scalarIdx != cvf::UNDEFINED_SIZE_T) calc->addStatisticsCalculator(new RigNativeStatCalc(this, scalarIdx));
            }
            {
                size_t scalarIdx = findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTY");
                if (scalarIdx != cvf::UNDEFINED_SIZE_T) calc->addStatisticsCalculator(new RigNativeStatCalc(this, scalarIdx));
            }
            {
                size_t scalarIdx = findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTY-");
                if (scalarIdx != cvf::UNDEFINED_SIZE_T) calc->addStatisticsCalculator(new RigNativeStatCalc(this, scalarIdx));
            }
            {
                size_t scalarIdx = findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTZ");
                if (scalarIdx != cvf::UNDEFINED_SIZE_T) calc->addStatisticsCalculator(new RigNativeStatCalc(this, scalarIdx));
            }
            {
                size_t scalarIdx = findScalarResultIndex(RimDefines::STATIC_NATIVE, "MULTZ-");
                if (scalarIdx != cvf::UNDEFINED_SIZE_T) calc->addStatisticsCalculator(new RigNativeStatCalc(this, scalarIdx));
            }

            cvf::ref<RigStatisticsDataCache> dataCache = new RigStatisticsDataCache(calc.p());
            m_statisticsDataCache.push_back(dataCache.p());
        }
        else
        {
            cvf::ref<RigNativeStatCalc> calc = new RigNativeStatCalc(this, scalarResultIndex);

            cvf::ref<RigStatisticsDataCache> dataCache = new RigStatisticsDataCache(calc.p());
            m_statisticsDataCache.push_back(dataCache.p());
        }
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RigCaseCellResultsData::resultNames(RimDefines::ResultCatType resType) const
{
    QStringList varList;
    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); it++)
    {
        if (it->m_resultType == resType )
        {
            varList.push_back(it->m_resultName);
        }
    } 
    return varList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::recalculateStatistics(size_t scalarResultIndex)
{
    m_statisticsDataCache[scalarResultIndex]->clearAllStatistics();
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result data in question is addressed by Active Cell Index
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::isUsingGlobalActiveIndex(size_t scalarResultIndex) const
{
    CVF_TIGHT_ASSERT(scalarResultIndex < m_cellScalarResults.size());

    if (!m_cellScalarResults[scalarResultIndex].size()) return true;
    
    size_t firstTimeStepResultValueCount = m_cellScalarResults[scalarResultIndex][0].size();
    if (firstTimeStepResultValueCount == m_ownerMainGrid->cells().size()) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RigCaseCellResultsData::timeStepDate(size_t scalarResultIndex, size_t timeStepIndex) const
{
    if (scalarResultIndex < m_resultInfos.size() && (size_t)(m_resultInfos[scalarResultIndex].m_timeStepDates.size()) > timeStepIndex)
        return m_resultInfos[scalarResultIndex].m_timeStepDates[static_cast<int>(timeStepIndex)];
    else
        return QDateTime();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigCaseCellResultsData::timeStepDates(size_t scalarResultIndex) const
{
    if (scalarResultIndex < m_resultInfos.size() )
        return  m_resultInfos[scalarResultIndex].m_timeStepDates;
    else
        return std::vector<QDateTime>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setTimeStepDates(size_t scalarResultIndex, const std::vector<QDateTime>& dates)
{
    CVF_ASSERT(scalarResultIndex < m_resultInfos.size() );

    m_resultInfos[scalarResultIndex].m_timeStepDates = dates;

    std::vector< std::vector<double> >& dataValues = this->cellScalarResults(scalarResultIndex);
    dataValues.resize(dates.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::maxTimeStepCount(size_t* scalarResultIndexWithMostTimeSteps) const
{
    size_t maxTsCount = 0;
    size_t scalarResultIndexWithMaxTsCount = cvf::UNDEFINED_SIZE_T;

    for (size_t i = 0; i < m_resultInfos.size(); i++)
    {
        if (m_resultInfos[i].m_timeStepDates.size() > maxTsCount)
        {
            maxTsCount = m_resultInfos[i].m_timeStepDates.size();
            scalarResultIndexWithMaxTsCount = i;
        }
    }

    if (scalarResultIndexWithMostTimeSteps)
    {
        *scalarResultIndexWithMostTimeSteps = scalarResultIndexWithMaxTsCount;
    }

    return maxTsCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigCaseCellResultsData::makeResultNameUnique(const QString& resultNameProposal) const
{
    QString newResultName = resultNameProposal;
    size_t resultIndex = cvf::UNDEFINED_SIZE_T;
    int nameNum = 1;
    int stringLength = newResultName.size();
    while (true) 
    {
        resultIndex = this->findScalarResultIndex(newResultName);
        if (resultIndex == cvf::UNDEFINED_SIZE_T) break;

        newResultName.truncate(stringLength);
        newResultName += "_" + QString::number(nameNum);
        ++nameNum;
    }

    return newResultName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::removeResult(const QString& resultName)
{
    size_t resultIdx = findScalarResultIndex(resultName);
    if (resultIdx == cvf::UNDEFINED_SIZE_T) return;

    m_cellScalarResults[resultIdx].clear();

    m_resultInfos[resultIdx].m_resultType = RimDefines::REMOVED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::clearAllResults()
{
    m_cellScalarResults.clear();
    m_resultInfos.clear();
}

//--------------------------------------------------------------------------------------------------
/// Removes all the actual numbers put into this object, and frees up the memory. 
/// Does not touch the metadata in any way
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::freeAllocatedResultsData()
{
    for (size_t resultIdx = 0; resultIdx < m_cellScalarResults.size(); ++resultIdx)
    {
        for (size_t tsIdx = 0; tsIdx < m_cellScalarResults[resultIdx].size(); ++tsIdx)
        {
            // Using swap with an empty vector as that is the safest way to really get rid of the allocated data in a vector
            std::vector<double> empty;
            m_cellScalarResults[resultIdx][tsIdx].swap(empty);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Add a result with given type and name, and allocate one result vector for the static result values
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::addStaticScalarResult(RimDefines::ResultCatType type, const QString& resultName, bool needsToBeStored, size_t resultValueCount)
{
    size_t resultIdx = addEmptyScalarResult(type, resultName, needsToBeStored);
    
    m_cellScalarResults[resultIdx].push_back(std::vector<double>());
    m_cellScalarResults[resultIdx][0].resize(resultValueCount, HUGE_VAL);

    return resultIdx;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderInterface::PorosityModelResultType RigCaseCellResultsData::convertFromProjectModelPorosityModel(RimDefines::PorosityModelType porosityModel)
{
    if (porosityModel == RimDefines::MATRIX_MODEL) return RifReaderInterface::MATRIX_RESULTS;
    
    return RifReaderInterface::FRACTURE_RESULTS;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::mustBeCalculated(size_t scalarResultIndex) const
{
    std::vector<ResultInfo>::const_iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); it++)
    {
        if (it->m_gridScalarResultIndex == scalarResultIndex)
        {
            return it->m_mustBeCalculated;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setMustBeCalculated(size_t scalarResultIndex)
{
    std::vector<ResultInfo>::iterator it;
    for (it = m_resultInfos.begin(); it != m_resultInfos.end(); it++)
    {
        if (it->m_gridScalarResultIndex == scalarResultIndex)
        {
            it->m_mustBeCalculated = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::createPerFaceCombinedResults()
{
    {
        size_t combinedTransmissibilityIndex = findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedTransmissibilityResultName());
        if (combinedTransmissibilityIndex == cvf::UNDEFINED_SIZE_T)
        {
            size_t tranX, tranY, tranZ;
            if (findTransmissibilityResults(tranX, tranY, tranZ))
            {
                addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::combinedTransmissibilityResultName(), false, 0);
            }
        }
    }

    {
        size_t combinedMultIndex = findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedMultResultName());
        if (combinedMultIndex == cvf::UNDEFINED_SIZE_T)
        {
            addStaticScalarResult(RimDefines::STATIC_NATIVE, RimDefines::combinedMultResultName(), false, 0);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::findTransmissibilityResults(size_t& tranX, size_t& tranY, size_t& tranZ) const
{
    tranX = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANX");
    tranY = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANY");
    tranZ = findScalarResultIndex(RimDefines::STATIC_NATIVE, "TRANZ");

    if (tranX == cvf::UNDEFINED_SIZE_T ||
        tranY == cvf::UNDEFINED_SIZE_T ||
        tranZ == cvf::UNDEFINED_SIZE_T)
    {
        return false;
    }

    return true;
}

