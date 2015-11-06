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

#include "RigCurveDataTools.h"

#include <cmath>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCurveDataTools::calculateIntervalsOfValidValues(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >* intervals)
{
    CVF_ASSERT(intervals);

    int startIdx = -1;
    size_t vIdx = 0;

    size_t valueCount = values.size();
    while (vIdx < valueCount)
    {
        double value = values[vIdx];
        if (value == HUGE_VAL || value == -HUGE_VAL || value != value)
        {
            if (startIdx >= 0)
            {
                intervals->push_back(std::make_pair(startIdx, vIdx - 1));
                startIdx = -1;
            }
        }
        else if (startIdx < 0)
        {
            startIdx = (int)vIdx;
        }

        vIdx++;
    }

    if (startIdx >= 0 && startIdx < ((int)valueCount))
    {
        intervals->push_back(std::make_pair(startIdx, valueCount - 1));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCurveDataTools::computePolyLineStartStopIndices(const std::vector< std::pair<size_t, size_t> >& intervals, 
                                                          std::vector< std::pair<size_t, size_t> >* fltrIntervals)
{
    CVF_ASSERT(fltrIntervals);

    const size_t intervalCount = intervals.size();
    if (intervalCount < 1) return;

    size_t index = 0;
    for (size_t intIdx = 0; intIdx < intervalCount; intIdx++)
    {
        size_t intervalSize = intervals[intIdx].second - intervals[intIdx].first + 1;
        fltrIntervals->push_back(std::make_pair(index, index + intervalSize - 1));

        index += intervalSize;
    }
}

