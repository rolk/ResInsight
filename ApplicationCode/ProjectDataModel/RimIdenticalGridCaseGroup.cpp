/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimIdenticalGridCaseGroup.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigGridManager.h"
#include "RimEclipseCase.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimEclipseView.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseStatisticsCase.h"
#include "RigCaseCellResultsData.h"

#include "cafProgressInfo.h"
#include "RiuMainWindow.h"
#include <QMessageBox>
#include <QDir>

CAF_PDM_SOURCE_INIT(RimIdenticalGridCaseGroup, "RimIdenticalGridCaseGroup");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup::RimIdenticalGridCaseGroup()
{
    CAF_PDM_InitObject("Grid Case Group", ":/GridCaseGroup16x16.png", "", "");

    CAF_PDM_InitField(&name,    "UserDescription",  QString("Grid Case Group"), "Name", "", "", "");

    CAF_PDM_InitField(&groupId, "GroupId", -1, "Case Group ID", "", "" ,"");
    groupId.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&statisticsCaseCollection, "StatisticsCaseCollection", "statisticsCaseCollection ChildArrayField", "", "", "");
    statisticsCaseCollection.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&caseCollection, "CaseCollection", "Source Cases ChildArrayField", "", "", "");
    caseCollection.uiCapability()->setUiHidden(true);
 
    caseCollection = new RimCaseCollection;
    caseCollection->uiCapability()->setUiName("Source Cases");
    caseCollection->uiCapability()->setUiIcon(QIcon(":/Cases16x16.png"));

    statisticsCaseCollection = new RimCaseCollection;
    statisticsCaseCollection->uiCapability()->setUiName("Derived Statistics");
    statisticsCaseCollection->uiCapability()->setUiIcon(QIcon(":/Histograms16x16.png"));


    m_mainGrid = NULL;

    m_unionOfMatrixActiveCells = new RigActiveCellInfo;
    m_unionOfFractureActiveCells = new RigActiveCellInfo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup::~RimIdenticalGridCaseGroup()
{
    m_mainGrid = NULL;

    delete caseCollection;
    caseCollection = NULL;

    delete statisticsCaseCollection;
    statisticsCaseCollection = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::addCase(RimEclipseCase* reservoir)
{
    CVF_ASSERT(reservoir);

    if (!reservoir) return;

    if (!m_mainGrid)
    {
        m_mainGrid = reservoir->reservoirData()->mainGrid();
    }
    else
    {
        reservoir->reservoirData()->setMainGrid(m_mainGrid);
    }

    caseCollection()->reservoirs().push_back(reservoir);

    clearActiveCellUnions();
    clearStatisticsResults();
    updateMainGridAndActiveCellsForStatisticsCases();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::removeCase(RimEclipseCase* reservoir)
{
    if (caseCollection()->reservoirs().count(reservoir) == 0)
    {
        return;
    }

    caseCollection()->reservoirs().removeChildObject(reservoir);

    if (caseCollection()->reservoirs().size() == 0)
    {
        m_mainGrid = NULL;
    }
    
    clearActiveCellUnions();
    clearStatisticsResults();
    updateMainGridAndActiveCellsForStatisticsCases();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimIdenticalGridCaseGroup::mainGrid()
{
    if (m_mainGrid) return m_mainGrid;

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIdenticalGridCaseGroup::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
///  Make sure changes in this functions is validated to RiaApplication::addEclipseCases()
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::loadMainCaseAndActiveCellInfo()
{
    if (caseCollection()->reservoirs().size() == 0)
    {
        return;
    }

    // Read the main case completely including grid.
    // The mainGrid from the first case is reused directly in for the other cases. 
    // When reading active cell info, only the total cell count is tested for consistency

    RimEclipseCase* mainCase = caseCollection()->reservoirs[0];
    if (!mainCase->openReserviorCase())
    {
        QMessageBox::warning(RiuMainWindow::instance(),
                             "Error when opening project file",
                             "Could not open the Eclipse Grid file: \n"+ mainCase->gridFileName() + "\n"+ 
                             "Current working directory is: \n" +
                             QDir::currentPath());
        return;
    }

    RigCaseData* rigCaseData = mainCase->reservoirData();
    CVF_ASSERT(rigCaseData);

    // Action A : Read active cell info
    // Read active cell info from all source cases. The file access is optimized for this purpose, and result meta data
    // is copied from main case to all other cases (see "Action B")
    
    caf::ProgressInfo info(caseCollection()->reservoirs.size(), "Case group - Reading Active Cell data");
    for (size_t i = 1; i < caseCollection()->reservoirs.size(); i++)
    {
        RimEclipseResultCase* rimReservoir = dynamic_cast<RimEclipseResultCase*>(caseCollection()->reservoirs[i]);
        if(!rimReservoir) continue; // Input reservoir

        if (!rimReservoir->openAndReadActiveCellData(rigCaseData))
        {
            // Error message
            continue;
        }

        info.incrementProgress();
    }

    m_mainGrid = rigCaseData->mainGrid();

    // Check if we need to calculate the union of the active cells

    bool foundResultsInCache = false;
    for (size_t i = 0; i < statisticsCaseCollection()->reservoirs.size(); i++)
    {
        RimEclipseCase* rimReservoir = statisticsCaseCollection()->reservoirs[i];

        // Check if any results are stored in cache
        if (rimReservoir->results(RifReaderInterface::MATRIX_RESULTS)->storedResultsCount() > 0 ||
            rimReservoir->results(RifReaderInterface::FRACTURE_RESULTS)->storedResultsCount() > 0)
        {
            foundResultsInCache = true;
            break;
        }
    }

    if (foundResultsInCache)
    {
        computeUnionOfActiveCells();
    }

    // Action B : Copy result meta data from main case to all other cases in grid case group

    // This code was originally part of RimStatisticsCaseEvaluator, but moved here to be a general solution
    // for all cases

    {
        RifReaderInterface::PorosityModelResultType poroModel = RifReaderInterface::MATRIX_RESULTS;

        std::vector<QDateTime> timeStepDates = rigCaseData->results(poroModel)->timeStepDates(0);
        const std::vector<RigCaseCellResultsData::ResultInfo> resultInfos = rigCaseData->results(poroModel)->infoForEachResultIndex();

        for (size_t i = 1; i < caseCollection()->reservoirs.size(); i++)
        {
            RimEclipseResultCase* rimReservoir = dynamic_cast<RimEclipseResultCase*>(caseCollection()->reservoirs[i]);
            if (!rimReservoir) continue; // Input reservoir

            RimReservoirCellResultsStorage* cellResultsStorage = rimReservoir->results(poroModel);

            for (size_t resIdx = 0; resIdx < resultInfos.size(); resIdx++)
            {
                RimDefines::ResultCatType resultType = resultInfos[resIdx].m_resultType;
                QString resultName = resultInfos[resIdx].m_resultName;
                bool needsToBeStored = resultInfos[resIdx].m_needsToBeStored;

                size_t scalarResultIndex = cellResultsStorage->cellResults()->findScalarResultIndex(resultType, resultName);
                if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
                {
                    size_t scalarResultIndex = cellResultsStorage->cellResults()->addEmptyScalarResult(resultType, resultName, needsToBeStored);
                    cellResultsStorage->cellResults()->setTimeStepDates(scalarResultIndex, timeStepDates);

                    std::vector< std::vector<double> >& dataValues = cellResultsStorage->cellResults()->cellScalarResults(scalarResultIndex);
                    dataValues.resize(timeStepDates.size());
                }
            }

            cellResultsStorage->cellResults()->createPlaceholderResultEntries();
        }
    }

    // "Load" the statistical cases

    for (size_t i = 0; i < statisticsCaseCollection()->reservoirs.size(); i++)
    {
        RimEclipseCase* rimReservoir = statisticsCaseCollection()->reservoirs[i];

        rimReservoir->openEclipseGridFile();

        if (i == 0)
        {
            rimReservoir->reservoirData()->computeActiveCellBoundingBoxes();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::computeUnionOfActiveCells()
{
    if (m_unionOfMatrixActiveCells->reservoirActiveCellCount() > 0)
    {
        return;
    }

    if (caseCollection->reservoirs.size() == 0 || !m_mainGrid)
    {
        this->clearActiveCellUnions();

        return;
    }

    m_unionOfMatrixActiveCells->setReservoirCellCount(m_mainGrid->cells().size());
    m_unionOfFractureActiveCells->setReservoirCellCount(m_mainGrid->cells().size());
    m_unionOfMatrixActiveCells->setGridCount(m_mainGrid->gridCount());
    m_unionOfFractureActiveCells->setGridCount(m_mainGrid->gridCount());

    size_t globalActiveMatrixIndex = 0;
    size_t globalActiveFractureIndex = 0;

    for (size_t gridIdx = 0; gridIdx < m_mainGrid->gridCount(); gridIdx++)
    {
        RigGridBase* grid = m_mainGrid->gridByIndex(gridIdx);

        std::vector<char> activeM(grid->cellCount(), 0);
        std::vector<char> activeF(grid->cellCount(), 0);

        for (size_t gridLocalCellIndex = 0; gridLocalCellIndex < grid->cellCount(); gridLocalCellIndex++)
        {
            for (size_t caseIdx = 0; caseIdx < caseCollection->reservoirs.size(); caseIdx++)
            {
                size_t reservoirCellIndex = grid->reservoirCellIndex(gridLocalCellIndex);

                if (activeM[gridLocalCellIndex] == 0)
                {
                    if (caseCollection->reservoirs[caseIdx]->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->isActive(reservoirCellIndex))
                    {
                        activeM[gridLocalCellIndex] = 1;
                    }
                }

                if (activeF[gridLocalCellIndex] == 0)
                {
                    if (caseCollection->reservoirs[caseIdx]->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS)->isActive(reservoirCellIndex))
                    {
                        activeF[gridLocalCellIndex] = 1;
                    }
                }
            }
        }

        size_t activeMatrixIndex = 0;
        size_t activeFractureIndex = 0;

        for (size_t gridLocalCellIndex = 0; gridLocalCellIndex < grid->cellCount(); gridLocalCellIndex++)
        {
            size_t reservoirCellIndex = grid->reservoirCellIndex(gridLocalCellIndex);

            if (activeM[gridLocalCellIndex] != 0)
            {
                m_unionOfMatrixActiveCells->setCellResultIndex(reservoirCellIndex, globalActiveMatrixIndex++);
                activeMatrixIndex++;
            }

            if (activeF[gridLocalCellIndex] != 0)
            {
                m_unionOfFractureActiveCells->setCellResultIndex(reservoirCellIndex, globalActiveFractureIndex++);
                activeFractureIndex++;
            }
        }

        m_unionOfMatrixActiveCells->setGridActiveCellCounts(gridIdx, activeMatrixIndex);
        m_unionOfFractureActiveCells->setGridActiveCellCounts(gridIdx, activeFractureIndex);
    }

    m_unionOfMatrixActiveCells->computeDerivedData();
    m_unionOfFractureActiveCells->computeDerivedData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RimIdenticalGridCaseGroup::createAndAppendStatisticsCase()
{
    RimEclipseStatisticsCase* newStatisticsCase = new RimEclipseStatisticsCase;

    newStatisticsCase->caseUserDescription = QString("Statistics ") + QString::number(statisticsCaseCollection()->reservoirs.size()+1);
    statisticsCaseCollection()->reservoirs.push_back(newStatisticsCase);

    newStatisticsCase->openEclipseGridFile();

    return newStatisticsCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::updateMainGridAndActiveCellsForStatisticsCases()
{
    for (size_t i = 0; i < statisticsCaseCollection->reservoirs().size(); i++)
    {
        RimEclipseCase* rimStaticsCase = statisticsCaseCollection->reservoirs[i];

        if (rimStaticsCase->reservoirData())
        {
            rimStaticsCase->reservoirData()->setMainGrid(this->mainGrid());

            if (i == 0)
            {
                rimStaticsCase->reservoirData()->computeActiveCellBoundingBoxes();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::clearStatisticsResults()
{
    for (size_t i = 0; i < statisticsCaseCollection->reservoirs().size(); i++)
    {
        RimEclipseCase* rimStaticsCase = statisticsCaseCollection->reservoirs[i];
        if (!rimStaticsCase) continue;

        if (rimStaticsCase->results(RifReaderInterface::MATRIX_RESULTS)->cellResults())
        {
            rimStaticsCase->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->clearAllResults();
        }
        if (rimStaticsCase->results(RifReaderInterface::FRACTURE_RESULTS)->cellResults())
        {
            rimStaticsCase->results(RifReaderInterface::FRACTURE_RESULTS)->cellResults()->clearAllResults();
        }

        for (size_t j = 0; j < rimStaticsCase->reservoirViews.size(); j++)
        {
            RimEclipseView* rimReservoirView = rimStaticsCase->reservoirViews[j];
            rimReservoirView->cellResult()->setResultVariable(RimDefines::undefinedResultName());
            rimReservoirView->cellEdgeResult()->resultVariable = RimDefines::undefinedResultName();
            rimReservoirView->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::clearActiveCellUnions()
{
    m_unionOfMatrixActiveCells->clear();
    m_unionOfFractureActiveCells->clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimIdenticalGridCaseGroup::contains(RimEclipseCase* reservoir) const
{
    CVF_ASSERT(reservoir);

    for (size_t i = 0; i < caseCollection()->reservoirs().size(); i++)
    {
        RimEclipseCase* rimReservoir = caseCollection()->reservoirs()[i];
        if (reservoir->gridFileName() == rimReservoir->gridFileName())
        {
            return true;
        }
    }
    
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RimIdenticalGridCaseGroup::unionOfActiveCells(RifReaderInterface::PorosityModelResultType porosityType)
{
    if (porosityType == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_unionOfMatrixActiveCells.p();
    }
    else
    {
        return m_unionOfFractureActiveCells.p();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimIdenticalGridCaseGroup::isStatisticsCaseCollection(RimCaseCollection* rimCaseCollection)
{
    caf::PdmFieldHandle* parentField = rimCaseCollection->parentField();
    if (parentField)
    {
        if (parentField->keyword() == "StatisticsCaseCollection")
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimIdenticalGridCaseGroup::mainCase()
{
    if(caseCollection()->reservoirs().size())
    {
        return caseCollection()->reservoirs()[0];
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimIdenticalGridCaseGroup::canCaseBeAdded(RimEclipseCase* reservoir) const
{
    CVF_ASSERT(reservoir && reservoir->reservoirData() && reservoir->reservoirData()->mainGrid());

    if (!m_mainGrid)
    {
        // Empty case group, reservoir can be added
        return true;
    }

    RigMainGrid* incomingMainGrid = reservoir->reservoirData()->mainGrid();
    
    if (RigGridManager::isEqual(m_mainGrid, incomingMainGrid))
    {
        return true;
    }

    return false;
}
