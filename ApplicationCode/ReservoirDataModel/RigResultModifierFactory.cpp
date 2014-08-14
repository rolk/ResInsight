/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "RigResultModifierFactory.h"

#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigResultModifier.h"

#include <math.h>


//--------------------------------------------------------------------------------------------------
/// This function must be harmonized with RigResultAccessorFactory::createNativeResultAccessor()
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultModifier> RigResultModifierFactory::createResultModifier(RigCaseData* eclipseCase,
    size_t gridIndex,
    RifReaderInterface::PorosityModelResultType porosityModel,
    size_t timeStepIndex,
    QString& uiResultName)
{
    CVF_ASSERT(gridIndex < eclipseCase->gridCount());
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(eclipseCase->results(porosityModel));
    CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

    if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return NULL;
    }

    size_t scalarSetIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(uiResultName);

    return createResultModifier(eclipseCase, gridIndex, porosityModel, timeStepIndex, scalarSetIndex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultModifier> RigResultModifierFactory::createResultModifier(RigCaseData* eclipseCase,
    size_t gridIndex,
    RifReaderInterface::PorosityModelResultType porosityModel,
    size_t timeStepIndex, size_t scalarResultIndex)
{
    CVF_ASSERT(gridIndex < eclipseCase->gridCount());
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(eclipseCase->results(porosityModel));
    CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

    RigGridBase* grid = eclipseCase->grid(gridIndex);

    if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return NULL;
    }

    if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
    {
        return NULL;
    }

    std::vector< std::vector<double> >& scalarSetResults = eclipseCase->results(porosityModel)->cellScalarResults(scalarResultIndex);

    if (timeStepIndex >= scalarSetResults.size())
    {
        return NULL;
    }

    std::vector<double>* resultValues = NULL;
    if (timeStepIndex < scalarSetResults.size())
    {
        resultValues = &(scalarSetResults[timeStepIndex]);
    }

    bool useGlobalActiveIndex = eclipseCase->results(porosityModel)->isUsingGlobalActiveIndex(scalarResultIndex);
    if (useGlobalActiveIndex)
    {
        cvf::ref<RigResultModifier> object = new RigActiveCellsResultModifier(grid, resultValues, eclipseCase->activeCellInfo(porosityModel));
        return object;
    }
    else
    {
        cvf::ref<RigResultModifier> object = new RigAllGridCellsResultModifier(grid, resultValues);
        return object;
    }
}
