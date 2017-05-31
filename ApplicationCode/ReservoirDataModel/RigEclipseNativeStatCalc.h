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

#include "RigStatisticsCalculator.h"

#include "RigCaseCellResultsData.h"
#include "RigActiveCellInfo.h"

class RigHistogramCalculator;

//==================================================================================================
/// 
//==================================================================================================

class RigEclipseNativeStatCalc : public RigStatisticsCalculator
{
public:
    RigEclipseNativeStatCalc(RigCaseCellResultsData* cellResultsData, size_t scalarResultIndex);

    virtual void    minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max);
    virtual void    posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg);
    virtual void    valueSumAndSampleCount(size_t timeStepIndex, double& valueSum, size_t& sampleCount);
    virtual void    addDataToHistogramCalculator(size_t timeStepIndex, RigHistogramCalculator& histogramCalculator);
    virtual void    uniqueValues(size_t timeStepIndex, std::set<int>& values);
    virtual size_t  timeStepCount();

private:
    RigCaseCellResultsData* m_resultsData;
    size_t                  m_scalarResultIndex;

    template <typename StatisticsAccumulator>
    void traverseCells(StatisticsAccumulator& accumulator, size_t timeStepIndex)
    {
        std::vector<double>& values = m_resultsData->cellScalarResults(m_scalarResultIndex, timeStepIndex);

        if (values.empty())
        {
            // Can happen if values do not exist for the current time step index.
            return;
        }

        const RigActiveCellInfo* actCellInfo = m_resultsData->activeCellInfo();
        size_t cellCount = actCellInfo->reservoirCellCount();

        for (size_t cIdx = 0; cIdx < cellCount; ++cIdx)
        {
            // Filter out inactive cells
            if (!actCellInfo->isActive(cIdx)) continue;

			size_t cellResultIndex = cIdx;
			if (m_resultsData->isUsingGlobalActiveIndex(m_scalarResultIndex))
			{
				cellResultIndex = actCellInfo->cellResultIndex(cIdx);
			}

            if (cellResultIndex != cvf::UNDEFINED_SIZE_T && cellResultIndex < values.size())
            {
                accumulator.addValue(values[cellResultIndex]);
            }
        }
    }
};
