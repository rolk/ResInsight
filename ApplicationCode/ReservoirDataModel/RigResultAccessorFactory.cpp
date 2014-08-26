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

#include "RigResultAccessorFactory.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigAllGridCellsResultAccessor.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigCombMultResultAccessor.h"
#include "RigCombTransResultAccessor.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"

#include "cvfAssert.h"
#include "cvfBase.h"
#include "cvfLibCore.h"
#include "cvfObject.h"

#include <math.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createResultAccessor(RigCaseData* eclipseCase,
	size_t gridIndex,
	RifReaderInterface::PorosityModelResultType porosityModel,
	size_t timeStepIndex,
	const QString& uiResultName)
{
	CVF_ASSERT(gridIndex < eclipseCase->gridCount());
	CVF_ASSERT(eclipseCase);
	CVF_ASSERT(eclipseCase->results(porosityModel));
	CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

	RigGridBase* grid = eclipseCase->grid(gridIndex);

	if (uiResultName == RimDefines::combinedTransmissibilityResultName())
	{
		cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

		cvf::ref<RigResultAccessor> xTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANX");
		cvf::ref<RigResultAccessor> yTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANY");
		cvf::ref<RigResultAccessor> zTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANZ");

		cellFaceAccessObject->setTransResultAccessors(xTransAccessor.p(), yTransAccessor.p(), zTransAccessor.p());

		return cellFaceAccessObject;
	}
    else if (uiResultName == RimDefines::combinedMultResultName())
    {
        cvf::ref<RigCombMultResultAccessor> cellFaceAccessObject = new RigCombMultResultAccessor(grid);

        cvf::ref<RigResultAccessor> multXPos = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTX");
        cvf::ref<RigResultAccessor> multXNeg = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTX-");
        cvf::ref<RigResultAccessor> multYPos = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTY");
        cvf::ref<RigResultAccessor> multYNeg = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTY-");
        cvf::ref<RigResultAccessor> multZPos = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTZ");
        cvf::ref<RigResultAccessor> multZNeg = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "MULTZ-");

        cellFaceAccessObject->setMultResultAccessors(multXPos.p(), multXNeg.p(), multYPos.p(), multYNeg.p(), multZPos.p(), multZNeg.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RimDefines::combinedRiTransResultName())
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, RimDefines::riTransXResultName());
        cvf::ref<RigResultAccessor> yTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, RimDefines::riTransYResultName());
        cvf::ref<RigResultAccessor> zTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, RimDefines::riTransZResultName());

        cellFaceAccessObject->setTransResultAccessors(xTransAccessor.p(), yTransAccessor.p(), zTransAccessor.p());

        return cellFaceAccessObject;
    }
    else if (uiResultName == RimDefines::combinedRiMultResultName())
    {
        cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

        cvf::ref<RigResultAccessor> xRiMultAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, RimDefines::riMultXResultName());
        cvf::ref<RigResultAccessor> yRiMultAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, RimDefines::riMultYResultName());
        cvf::ref<RigResultAccessor> zRiMultAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, RimDefines::riMultZResultName());

        cellFaceAccessObject->setTransResultAccessors(xRiMultAccessor.p(), yRiMultAccessor.p(), zRiMultAccessor.p());

        return cellFaceAccessObject;
    }

	return RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, uiResultName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createResultAccessor(RigCaseData* eclipseCase,
	size_t gridIndex,
	RifReaderInterface::PorosityModelResultType porosityModel,
	size_t timeStepIndex,
	const QString& uiResultName,
	RimDefines::ResultCatType resultType)
{
	CVF_ASSERT(gridIndex < eclipseCase->gridCount());
	CVF_ASSERT(eclipseCase);
	CVF_ASSERT(eclipseCase->results(porosityModel));
	CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

	RigGridBase *grid = eclipseCase->grid(gridIndex);

	if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
	{
		return NULL;
	}

	size_t scalarSetIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(resultType, uiResultName);
	if (scalarSetIndex == cvf::UNDEFINED_SIZE_T)
	{
		return NULL;
	}

	return createResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, scalarSetIndex);
}

//--------------------------------------------------------------------------------------------------
/// This function must be harmonized with RigResultModifierFactory::createResultModifier()
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createNativeResultAccessor(RigCaseData* eclipseCase,
    size_t gridIndex,
    RifReaderInterface::PorosityModelResultType porosityModel,
    size_t timeStepIndex,
    const QString& uiResultName)
{
    CVF_ASSERT(gridIndex < eclipseCase->gridCount());
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(eclipseCase->results(porosityModel));
    CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

    RigGridBase *grid = eclipseCase->grid(gridIndex);

    if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return NULL;
    }

    size_t scalarSetIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(uiResultName);
    if (scalarSetIndex == cvf::UNDEFINED_SIZE_T)
    {
        return NULL;
    }

	return createResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, scalarSetIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createResultAccessor(RigCaseData* eclipseCase,
	size_t gridIndex, 
	RifReaderInterface::PorosityModelResultType porosityModel, 
	size_t timeStepIndex, 
	size_t resultIndex)
{
	if (resultIndex == cvf::UNDEFINED_SIZE_T)
	{
		return new RigHugeValResultAccessor;
	}

	if (!eclipseCase) return NULL;

	RigGridBase* grid = eclipseCase->grid(gridIndex);
	if (!grid) return NULL;

	std::vector< std::vector<double> >& scalarSetResults = eclipseCase->results(porosityModel)->cellScalarResults(resultIndex);

	if (timeStepIndex >= scalarSetResults.size())
	{
		return new RigHugeValResultAccessor;
	}

	std::vector<double>* resultValues = NULL;
	if (timeStepIndex < scalarSetResults.size())
	{
		resultValues = &(scalarSetResults[timeStepIndex]);
	}

	if (!resultValues || resultValues->size() == 0)
	{
		return new RigHugeValResultAccessor;
	}

	bool useGlobalActiveIndex = eclipseCase->results(porosityModel)->isUsingGlobalActiveIndex(resultIndex);
	if (useGlobalActiveIndex)
	{
		cvf::ref<RigResultAccessor> object = new RigActiveCellsResultAccessor(grid, resultValues, eclipseCase->activeCellInfo(porosityModel));
		return object;
	}
	else
	{
		cvf::ref<RigResultAccessor> object = new RigAllGridCellsResultAccessor(grid, resultValues);
		return object;
	}
}

