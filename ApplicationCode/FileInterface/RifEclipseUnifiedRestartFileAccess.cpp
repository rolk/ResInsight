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

#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifEclipseOutputFileTools.h"

#include <well_state.h>
#include <well_info.h>
#include <well_conn.h>
#include <well_ts.h>

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseUnifiedRestartFileAccess::RifEclipseUnifiedRestartFileAccess()
    : RifEclipseRestartDataAccess()
{
    m_gridCount = 0;
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseUnifiedRestartFileAccess::~RifEclipseUnifiedRestartFileAccess()
{
    close();
}

//--------------------------------------------------------------------------------------------------
/// Open file
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::open(const QStringList& fileSet, const std::vector<size_t>& matrixModelActiveCellCounts, const std::vector<size_t>& fractureModelActiveCellCounts)
{
    QString fileName = fileSet[0];

    cvf::ref<RifEclipseOutputFileTools> fileAccess = new RifEclipseOutputFileTools;
    if (!fileAccess->open(fileName, matrixModelActiveCellCounts, fractureModelActiveCellCounts))
    {
        return false;
    }

    m_file = fileAccess;

    m_gridCount = matrixModelActiveCellCounts.size();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Close file
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::close()
{
    if (m_file.notNull())
    {
        m_file->close();
        m_file = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// Get the number of time steps
//--------------------------------------------------------------------------------------------------
size_t RifEclipseUnifiedRestartFileAccess::numTimeSteps()
{
    QStringList timeSteps = timeStepsText();
    return timeSteps.size();
}

//--------------------------------------------------------------------------------------------------
/// Get the time step texts
//--------------------------------------------------------------------------------------------------
QStringList RifEclipseUnifiedRestartFileAccess::timeStepsText()
{
    RifEclipseOutputFileTools* file = m_file.p();
    CVF_ASSERT(file != NULL);

    QStringList timeSteps;
    file->timeStepsText(&timeSteps);

    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// Get the time steps
//--------------------------------------------------------------------------------------------------
QList<QDateTime> RifEclipseUnifiedRestartFileAccess::timeSteps()
{
    RifEclipseOutputFileTools* file = m_file.p();
    CVF_ASSERT(file != NULL);

    QList<QDateTime> timeSteps;
    file->timeSteps(&timeSteps);

    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// Get list of result names
//--------------------------------------------------------------------------------------------------
QStringList RifEclipseUnifiedRestartFileAccess::resultNames(RifReaderInterface::PorosityModelResultType matrixOrFracture)
{
    // Get the results found on the UNRST file
    QStringList resultsList;
    m_file->validKeywords(&resultsList, matrixOrFracture);

    return resultsList;
}

//--------------------------------------------------------------------------------------------------
/// Get result values for given time step
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::results(const QString& resultName, RifReaderInterface::PorosityModelResultType matrixOrFracture, size_t timeStep, std::vector<double>* values)
{
    size_t numOccurrences   = m_file->numOccurrences(resultName);
    size_t startIndex       = timeStep * m_gridCount;
    CVF_ASSERT(startIndex + m_gridCount <= numOccurrences);

    size_t occurrenceIdx;
    for (occurrenceIdx = startIndex; occurrenceIdx < startIndex + m_gridCount; occurrenceIdx++)
    {
        std::vector<double> partValues;
        if (!m_file->keywordData(resultName, occurrenceIdx, matrixOrFracture, &partValues))  // !! don't need to append afterwards
        {
            return false;
        }

        values->insert(values->end(), partValues.begin(), partValues.end());
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::readWellData(well_info_type* well_info)
{
    if (!well_info) return;

    well_info_add_UNRST_wells(well_info, m_file->filePointer());
}

