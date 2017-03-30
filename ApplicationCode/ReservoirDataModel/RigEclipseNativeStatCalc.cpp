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

#include "RigEclipseNativeStatCalc.h"

#include "RigStatisticsMath.h"
#include "RigCaseCellResultsData.h"
#include "RigStatisticsMath.h"

#include <cmath> // Needed for HUGE_VAL on Linux


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseNativeStatCalc::RigEclipseNativeStatCalc(RigCaseCellResultsData* cellResultsData, size_t scalarResultIndex)
    : m_resultsData(cellResultsData),
    m_scalarResultIndex(scalarResultIndex)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max)
{
    MinMaxAccumulator acc(min, max);
    traverseCells(acc, timeStepIndex);
    min = acc.min;
    max = acc.max;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg)
{
    PosNegAccumulator acc(pos, neg);
    traverseCells(acc, timeStepIndex);
    pos = acc.pos;
    neg = acc.neg;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator)
{
    traverseCells(histogramCalculator, timeStepIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::uniqueValues(size_t timeStepIndex, std::set<int>& values)
{
    UniqueValueAccumulator acc;
    traverseCells(acc, timeStepIndex);
    values = acc.uniqueValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseNativeStatCalc::valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount)
{
    SumCountAccumulator acc(valueSum, sampleCount);
    traverseCells(acc, timeStepIndex);
    valueSum = acc.valueSum;
    sampleCount = acc.sampleCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigEclipseNativeStatCalc::timeStepCount()
{
    return m_resultsData->timeStepCount(m_scalarResultIndex);
}
