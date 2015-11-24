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

#include "RiaSocketCommand.h"
#include "RiaSocketDataTransfer.h"
#include "RiaSocketServer.h"
#include "RiaSocketTools.h"

#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigResultModifier.h"
#include "RigResultModifierFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimReservoirCellResultsStorage.h"

#include "RiuMainWindow.h"
#include "RiuProcessMonitor.h"

#include <QErrorMessage>


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetActiveCellProperty: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetActiveCellProperty"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);

        QString propertyName = args[2];
        QString porosityModelName = args[3];

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName == "Fracture")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        // Find the requested data

        size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;
        std::vector< std::vector<double> >* scalarResultFrames = NULL;

        if (rimCase && rimCase->results(porosityModelEnum))
        {
            scalarResultIndex = rimCase->results(porosityModelEnum)->findOrLoadScalarResult(propertyName);

            if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
            {
                scalarResultFrames = &(rimCase->results(porosityModelEnum)->cellResults()->cellScalarResults(scalarResultIndex));
            }

        }

        if (scalarResultFrames == NULL)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the %1 model property named: \"%2\"").arg(porosityModelName).arg(propertyName));
        }

        // Write data back : timeStepCount, bytesPrTimestep, dataForTimestep0 ... dataForTimestepN

        if ( scalarResultFrames == NULL)
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
        }
        else
        {
            // Create a list of all the requested timesteps

            std::vector<size_t> requestedTimesteps;

            if (args.size() <= 4)
            {
                // Select all
                for (size_t tsIdx = 0; tsIdx < scalarResultFrames->size(); ++tsIdx)
                {
                    requestedTimesteps.push_back(tsIdx);
                }
            }
            else
            {
                bool timeStepReadError = false;
                for (int argIdx = 4; argIdx < args.size(); ++argIdx)
                {
                    bool conversionOk = false;
                    int tsIdx = args[argIdx].toInt(&conversionOk);

                    if (conversionOk)
                    {
                        requestedTimesteps.push_back(tsIdx);
                    }
                    else
                    {
                        timeStepReadError = true;
                    }
                }

                if (timeStepReadError)
                {
                    server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: riGetActiveCellProperty : \n") + RiaSocketServer::tr("An error occured while interpreting the requested timesteps."));
                }

            }

            // First write timestep count
            quint64 timestepCount = (quint64)requestedTimesteps.size();
            socketStream << timestepCount;

            // then the byte-size of the result values in one timestep

            const RigActiveCellInfo* activeInfo = rimCase->reservoirData()->activeCellInfo(porosityModelEnum);
            size_t  timestepResultCount = activeInfo->reservoirActiveCellCount();

            quint64 timestepByteCount = (quint64)(timestepResultCount*sizeof(double));
            socketStream << timestepByteCount ;

            // Then write the data.
            size_t valueCount = RiaSocketDataTransfer::maximumValueCountInBlock();
            std::vector<double> values(valueCount);
            size_t valueIndex = 0;

            size_t reservoirCellCount = activeInfo->reservoirCellCount();
            for (size_t tIdx = 0; tIdx < requestedTimesteps.size(); ++tIdx)
            {
                std::vector<double>& doubleValues = scalarResultFrames->at(requestedTimesteps[tIdx]);
                for (size_t gcIdx = 0; gcIdx < reservoirCellCount; ++gcIdx)
                {
                    size_t resultIdx = activeInfo->cellResultIndex(gcIdx);
                    if (resultIdx == cvf::UNDEFINED_SIZE_T) continue;

                    if (resultIdx < doubleValues.size())
                    {
                        if (doubleValues.size() == activeInfo->reservoirCellCount())
                        {
                            // When reading data from input text files, result data is read for all grid cells
                            // Read out values from data vector using global cell index instead of active cell result index
                            // When data is written back to ResInsight using RiaSetActiveCellProperty, the resulting 
                            // data vector will have activeCellCount data values, which is potentially smaller
                            // than total number of cells
                            values[valueIndex] = doubleValues[gcIdx];
                        }
                        else
                        {
                            values[valueIndex] = doubleValues[resultIdx];
                        }
                    }
                    else
                    {
                        values[valueIndex] = HUGE_VAL;
                    }

                    valueIndex++;
                    if (valueIndex >= valueCount)
                    {
                        if (!RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)values.data(), valueIndex * sizeof(double)))
                        {
                            return false;
                        }

                        valueIndex = 0;
                    }
                }
            }

            // Write remaining data
            if (!RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)values.data(), valueIndex * sizeof(double)))
            {
                return false;
            }

        }
        return true;
    }
};

static bool RiaGetActiveCellProperty_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetActiveCellProperty>(RiaGetActiveCellProperty::commandName());



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetGridProperty: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetGridProperty"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int caseId                = args[1].toInt();
        int gridIdx               = args[2].toInt();
        QString propertyName      = args[3];
        QString porosityModelName = args[4];
        
        RimEclipseCase*rimCase = server->findReservoir(caseId);
        if (rimCase == NULL)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the case with ID: \"%1\"").arg(caseId));

            // No data available
            socketStream << (quint64)0 << (quint64)0 <<  (quint64)0  << (quint64)0 ;
            return true;
        }

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName == "Fracture")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;

        if (gridIdx < 0  || rimCase->reservoirData()->gridCount() <= (size_t)gridIdx)
        {
            server->errorMessageDialog()->showMessage("ResInsight SocketServer: riGetGridProperty : \n"
                                                      "The gridIndex \"" + QString::number(gridIdx) + "\" does not point to an existing grid." );
        }
        else
        {
            // Find the requested data
            if (rimCase && rimCase->results(porosityModelEnum))
            {
                scalarResultIndex = rimCase->results(porosityModelEnum)->findOrLoadScalarResult(propertyName);
            }
        }

        if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the %1 model property named: \"%2\"").arg(porosityModelName).arg(propertyName));

            // No data available
            socketStream << (quint64)0 << (quint64)0 <<  (quint64)0  << (quint64)0 ;
            return true;
        }


        // Create a list of all the requested time steps

        std::vector<size_t> requestedTimesteps;

        if (args.size() <= 5)
        {
            // Select all
            for (size_t tsIdx = 0; tsIdx < rimCase->results(porosityModelEnum)->cellResults()->timeStepCount(scalarResultIndex); ++tsIdx)
            {
                requestedTimesteps.push_back(tsIdx);
            }
        }
        else
        {
            bool timeStepReadError = false;
            for (int argIdx = 5; argIdx < args.size(); ++argIdx)
            {
                bool conversionOk = false;
                int tsIdx = args[argIdx].toInt(&conversionOk);

                if (conversionOk)
                {
                    requestedTimesteps.push_back(tsIdx);
                }
                else
                {
                    timeStepReadError = true;
                }
            }

            if (timeStepReadError)
            {
                server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: riGetGridProperty : \n")
                                                          + RiaSocketServer::tr("An error occured while interpreting the requested timesteps."));
            }

        }


        RigGridBase* rigGrid = rimCase->reservoirData()->grid(gridIdx);

        quint64 cellCountI = (quint64)rigGrid->cellCountI();
        quint64 cellCountJ = (quint64)rigGrid->cellCountJ();
        quint64 cellCountK = (quint64)rigGrid->cellCountK();

        socketStream << cellCountI;
        socketStream << cellCountJ;
        socketStream << cellCountK;

        // Write time step count

        quint64 timestepCount = (quint64)requestedTimesteps.size();
        socketStream << timestepCount;

        for (size_t tsIdx = 0; tsIdx < timestepCount; tsIdx++)
        {
            cvf::ref<RigResultAccessor> resultAccessor = RigResultAccessorFactory::createResultAccessor(rimCase->reservoirData(), gridIdx, porosityModelEnum, requestedTimesteps[tsIdx], propertyName);

            if (resultAccessor.isNull())
            {
                continue;
            }

            size_t valueCount = RiaSocketDataTransfer::maximumValueCountInBlock();
            std::vector<double> values(valueCount);
            size_t valueIndex = 0;
            for (size_t cellIdx = 0; cellIdx < rigGrid->cellCount(); cellIdx++)
            {
                double cellValue = resultAccessor->cellScalar(cellIdx);
                if (cellValue == HUGE_VAL)
                {
                    cellValue = 0.0;
                }
                values[valueIndex++] = cellValue;

                if (valueIndex >= valueCount)
                {
                    if (!RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)values.data(), valueIndex * sizeof(double)))
                    {
                        return false;
                    }

                    valueIndex = 0;
                }
            }

            // Write remaining data
            if (!RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)values.data(), valueIndex * sizeof(double)))
            {
                return false;
            }
        }

        return true;
    }
};

static bool RiaGetGridProperty_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetGridProperty>(RiaGetGridProperty::commandName());






//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaSetActiveCellProperty: public RiaSocketCommand
{
public:
    RiaSetActiveCellProperty() :
        m_currentReservoir(NULL),
        m_scalarResultsToAdd(NULL),
        m_currentScalarIndex(cvf::UNDEFINED_SIZE_T),
        m_timeStepCountToRead(0),
        m_bytesPerTimeStepToRead(0),
        m_currentTimeStepNumberToRead(0),
        m_invalidActiveCellCountDetected(false),
        m_porosityModelEnum(RifReaderInterface::MATRIX_RESULTS)
    {}

    static QString commandName () { return QString("SetActiveCellProperty"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);

        QString propertyName = args[2];
        QString porosityModelName = args[3];

        if (porosityModelName == "Fracture")
        {
            m_porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        // Find the requested data, Or create a set if we are setting data and it is not found

        size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;
        std::vector< std::vector<double> >* scalarResultFrames = NULL;

        if (rimCase && rimCase->results(m_porosityModelEnum))
        {
            scalarResultIndex = rimCase->results(m_porosityModelEnum)->findOrLoadScalarResult(RimDefines::GENERATED, propertyName);

            if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
            {
                scalarResultIndex = rimCase->results(m_porosityModelEnum)->cellResults()->addEmptyScalarResult(RimDefines::GENERATED, propertyName, true);
            }

            if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
            {
                scalarResultFrames = &(rimCase->results(m_porosityModelEnum)->cellResults()->cellScalarResults(scalarResultIndex));
                size_t timeStepCount = rimCase->results(m_porosityModelEnum)->cellResults()->maxTimeStepCount();
                scalarResultFrames->resize(timeStepCount);

                m_currentScalarIndex = scalarResultIndex;
                m_currentPropertyName = propertyName;
            }
        }

        if (scalarResultFrames == NULL)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the %1 model property named: \"%2\"").arg(porosityModelName).arg(propertyName));
            return true;
        }

        // If we have not read the header and there are data enough: Read it.
        // Do nothing if we have not enough data

        if (m_timeStepCountToRead == 0 || m_bytesPerTimeStepToRead == 0)
        {
            if (server->currentClient()->bytesAvailable() < (int)sizeof(quint64)*2) return true;

            socketStream >> m_timeStepCountToRead;
            socketStream >> m_bytesPerTimeStepToRead;
        }

//        std::cout << "RiaSetActiveCellProperty: " << propertyName.data() << " timeStepCount " << m_timeStepCountToRead << " bytesPerTimeStep " << m_bytesPerTimeStepToRead;

        // Create a list of all the requested timesteps

        m_requestedTimesteps.clear();

        if (args.size() <= 4)
        {
            // Select all
            for (size_t tsIdx = 0; tsIdx < m_timeStepCountToRead; ++tsIdx)
            {
                m_requestedTimesteps.push_back(tsIdx);
            }
        }
        else
        {
            bool timeStepReadError = false;
            for (int argIdx = 4; argIdx < args.size(); ++argIdx)
            {
                bool conversionOk = false;
                int tsIdx = args[argIdx].toInt(&conversionOk);

                if (conversionOk)
                {
                    m_requestedTimesteps.push_back(tsIdx);
                }
                else
                {
                    timeStepReadError = true;
                }
            }

            if (timeStepReadError)
            {
                server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: riGetActiveCellProperty : \n") + RiaSocketServer::tr("An error occured while interpreting the requested timesteps."));
            }

        }

        if (! m_requestedTimesteps.size())
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("No time steps specified").arg(porosityModelName).arg(propertyName));

            return true;
        }

        m_currentReservoir = rimCase;
        m_scalarResultsToAdd = scalarResultFrames;

        if (server->currentClient()->bytesAvailable())
        {
            return this->interpretMore(server, server->currentClient());
        }

        return false;
    }

    virtual bool interpretMore(RiaSocketServer* server, QTcpSocket* currentClient)
    {
//        std::cout << "RiaSetActiveCellProperty, interpretMore: scalarIndex : " << m_currentScalarIndex;

        if (m_invalidActiveCellCountDetected) return true;

        // If nothing should be read, or we already have read everything, do nothing

        if ((m_timeStepCountToRead == 0) || (m_currentTimeStepNumberToRead >= m_timeStepCountToRead) )  return true;

        if (!currentClient->bytesAvailable()) return false;

        if (m_timeStepCountToRead != m_requestedTimesteps.size())
        {
            CVF_ASSERT(false);
        }

        // Check if a complete timestep is available, return and whait for readyRead() if not
        if (currentClient->bytesAvailable() < (int)m_bytesPerTimeStepToRead) return false;

        size_t  cellCountFromOctave = m_bytesPerTimeStepToRead / sizeof(double);

        RigActiveCellInfo* activeCellInfo = m_currentReservoir->reservoirData()->activeCellInfo(m_porosityModelEnum);

        size_t activeCellCountReservoir    = activeCellInfo->reservoirActiveCellCount();
        size_t totalCellCount           = activeCellInfo->reservoirCellCount();
        size_t reservoirCellResultCount    = activeCellInfo->reservoirCellResultCount();

        bool isCoarseningActive = reservoirCellResultCount != activeCellCountReservoir;

        if (cellCountFromOctave != activeCellCountReservoir )
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") +
                                              RiaSocketServer::tr("The number of cells in the data coming from octave does not match the case") + ":\""  + m_currentReservoir->caseUserDescription() + "\"\n"
                                              "   Octave: " + QString::number(cellCountFromOctave) + "\n"
                                              "  " + m_currentReservoir->caseUserDescription() + ": Active cell count: " + QString::number(activeCellCountReservoir) + " Total cell count: " +  QString::number(totalCellCount)) ;

            cellCountFromOctave = 0;
            m_invalidActiveCellCountDetected = true;
            currentClient->abort();

            return true;
        }

        // Make sure the size of the retreiving container is correct.
        // If it is, this is noops
        {
            size_t maxRequestedTimeStepIdx = cvf::UNDEFINED_SIZE_T;
            for (size_t tIdx = 0; tIdx < m_timeStepCountToRead; ++tIdx)
            {
                size_t tsId = m_requestedTimesteps[tIdx];
                if (maxRequestedTimeStepIdx == cvf::UNDEFINED_SIZE_T || tsId > maxRequestedTimeStepIdx)
                {
                    maxRequestedTimeStepIdx = tsId;
                }
            }

            if (maxRequestedTimeStepIdx != cvf::UNDEFINED_SIZE_T
                && m_scalarResultsToAdd->size() <= maxRequestedTimeStepIdx)
            {
                m_scalarResultsToAdd->resize(maxRequestedTimeStepIdx + 1);
            }
        }

        for (size_t tIdx = 0; tIdx < m_timeStepCountToRead; ++tIdx)
        {
            size_t tsId = m_requestedTimesteps[tIdx];
            m_scalarResultsToAdd->at(tsId).resize(reservoirCellResultCount, HUGE_VAL);
        }

        std::vector<double> readBuffer;
        double * internalMatrixData = NULL;

        if (isCoarseningActive)
        {
            readBuffer.resize(cellCountFromOctave, HUGE_VAL);
            internalMatrixData = readBuffer.data();
        }

        QDataStream socketStream(currentClient);
        socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

        // Read available complete timestepdata

        while ((currentClient->bytesAvailable() >= (int)m_bytesPerTimeStepToRead) && (m_currentTimeStepNumberToRead < m_timeStepCountToRead))
        {
            qint64 bytesRead = 0;
            if ( !isCoarseningActive)
            {
                internalMatrixData = m_scalarResultsToAdd->at(m_requestedTimesteps[m_currentTimeStepNumberToRead]).data();
            }

            QStringList errorMessages;
            if (!RiaSocketDataTransfer::readBlockDataFromSocket(currentClient, (char*)(internalMatrixData), m_bytesPerTimeStepToRead, errorMessages))
            {
                for (int i = 0; i < errorMessages.size(); i++)
                {
                    server->errorMessageDialog()->showMessage(errorMessages[i]);
                }

                currentClient->abort();
                return true;
            }

            // Map data from active  to result index based container ( Coarsening is active)
            if (isCoarseningActive)
            {
                size_t acIdx = 0;
                for (size_t gcIdx = 0; gcIdx < totalCellCount; ++gcIdx)
                {
                    if (activeCellInfo->isActive(gcIdx))
                    {
                        m_scalarResultsToAdd->at(m_requestedTimesteps[m_currentTimeStepNumberToRead])[activeCellInfo->cellResultIndex(gcIdx)] = readBuffer[acIdx];
                        ++acIdx;
                    }
                }
            }

            ++m_currentTimeStepNumberToRead;
        }


        // If we have read all the data, refresh the views

        if (m_currentTimeStepNumberToRead == m_timeStepCountToRead)
        {
            if (m_currentReservoir != NULL)
            {
                // Create a new input property if we have an input reservoir
                RimEclipseInputCase* inputRes = dynamic_cast<RimEclipseInputCase*>(m_currentReservoir);
                if (inputRes)
                {
                    RimEclipseInputProperty* inputProperty = NULL;
                    inputProperty = inputRes->m_inputPropertyCollection->findInputProperty(m_currentPropertyName);
                    if (!inputProperty)
                    {
                        inputProperty = new RimEclipseInputProperty;
                        inputProperty->resultName = m_currentPropertyName;
                        inputProperty->eclipseKeyword = "";
                        inputProperty->fileName = "";
                        inputRes->m_inputPropertyCollection->inputProperties.push_back(inputProperty);
                        inputRes->m_inputPropertyCollection()->updateConnectedEditors();
                    }
                    inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED_NOT_SAVED;
                }

                if( m_currentScalarIndex != cvf::UNDEFINED_SIZE_T &&
                        m_currentReservoir->reservoirData() &&
                        m_currentReservoir->reservoirData()->results(m_porosityModelEnum) )
                {
                    // Adjust the result data if only one time step is requested so the result behaves like a static result
                    if (m_requestedTimesteps.size() == 1 && m_currentScalarIndex != cvf::UNDEFINED_SIZE_T)
                    {
                        std::vector< std::vector<double> >* scalarResultFrames = NULL;
                        scalarResultFrames = &(m_currentReservoir->results(m_porosityModelEnum)->cellResults()->cellScalarResults(m_currentScalarIndex));
                        size_t lastIndexWithDataPresent = cvf::UNDEFINED_SIZE_T;
                        for (size_t i = 0; i < scalarResultFrames->size(); i++)
                        {
                            if ((*scalarResultFrames)[i].size() > 0)
                            {
                                lastIndexWithDataPresent = i;
                            }
                        }

                        if (lastIndexWithDataPresent == 0)
                        {
                            scalarResultFrames->resize(1);
                        }
                    }

                    m_currentReservoir->reservoirData()->results(m_porosityModelEnum)->recalculateStatistics(m_currentScalarIndex);
                }

                for (size_t i = 0; i < m_currentReservoir->reservoirViews.size(); ++i)
                {
                    if (m_currentReservoir->reservoirViews[i])
                    {
                        // As new result might have been introduced, update all editors connected
                        m_currentReservoir->reservoirViews[i]->cellResult->updateConnectedEditors();

                        // It is usually not needed to create new display model, but if any derived geometry based on generated data (from Octave) 
                        // a full display model rebuild is required
                        m_currentReservoir->reservoirViews[i]->scheduleCreateDisplayModelAndRedraw();
                    }
                }
            }

            return true;
        }

        return false;

    }

private:
    RimEclipseCase*                            m_currentReservoir;
    std::vector< std::vector<double> >* m_scalarResultsToAdd;
    size_t                              m_currentScalarIndex;
    QString                             m_currentPropertyName;
    std::vector<size_t>                 m_requestedTimesteps;
    RifReaderInterface::PorosityModelResultType m_porosityModelEnum;

    quint64                             m_timeStepCountToRead;
    quint64                             m_bytesPerTimeStepToRead;
    size_t                              m_currentTimeStepNumberToRead;

    bool                                m_invalidActiveCellCountDetected;
};

static bool RiaSetActiveCellProperty_init = RiaSocketCommandFactory::instance()->registerCreator<RiaSetActiveCellProperty>(RiaSetActiveCellProperty::commandName());


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaSetGridProperty : public RiaSocketCommand
{
public:
    RiaSetGridProperty() :
      m_currentReservoir(NULL),
          m_scalarResultsToAdd(NULL),
          m_currentGridIndex(cvf::UNDEFINED_SIZE_T),
          m_currentScalarIndex(cvf::UNDEFINED_SIZE_T),
          m_timeStepCountToRead(0),
          m_bytesPerTimeStepToRead(0),
          m_currentTimeStepNumberToRead(0),
          m_invalidDataDetected(false),
          m_porosityModelEnum(RifReaderInterface::MATRIX_RESULTS)
    {}

    static QString commandName () { return QString("SetGridProperty"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream)
    {
        int caseId = args[1].toInt();
        RimEclipseCase* rimCase = server->findReservoir(caseId);
        if (!rimCase)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the case with ID : \"%1\"").arg(caseId));

            return true;
        }

        m_currentGridIndex = args[2].toInt();
        QString propertyName = args[3];
        QString porosityModelName = args[4];

        if (porosityModelName == "Fracture")
        {
            m_porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        RigGridBase* grid = rimCase->reservoirData()->grid(m_currentGridIndex);
        if (!grid)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the grid index : %1").arg(m_currentGridIndex));
            return true;
        }


        // Read header
        if (server->currentClient()->bytesAvailable() < (int)sizeof(quint64)*5)
        {
            return true;
        }

        quint64 cellCountI = 0;
        quint64 cellCountJ = 0;
        quint64 cellCountK = 0;
        socketStream >> cellCountI;
        socketStream >> cellCountJ;
        socketStream >> cellCountK;

        if (grid->cellCountI() != cellCountI ||
            grid->cellCountJ() != cellCountJ ||
            grid->cellCountK() != cellCountK)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Destination grid size do not match incoming grid size for grid index : %1").arg(m_currentGridIndex));
            return true;
        }

        socketStream >> m_timeStepCountToRead;
        socketStream >> m_bytesPerTimeStepToRead;

        if (m_timeStepCountToRead == 0 || m_bytesPerTimeStepToRead == 0)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") +
                RiaSocketServer::tr("Zero data to read for ") + ":\"" + m_currentReservoir->caseUserDescription() + "\"\n");

            return true;
        }


        size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;
        std::vector< std::vector<double> >* scalarResultFrames = NULL;

        if (rimCase && rimCase->results(m_porosityModelEnum))
        {
            scalarResultIndex = rimCase->results(m_porosityModelEnum)->findOrLoadScalarResult(RimDefines::GENERATED, propertyName);

            if (scalarResultIndex == cvf::UNDEFINED_SIZE_T)
            {
                scalarResultIndex = rimCase->results(m_porosityModelEnum)->cellResults()->addEmptyScalarResult(RimDefines::GENERATED, propertyName, true);
            }

            if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
            {
                scalarResultFrames = &(rimCase->results(m_porosityModelEnum)->cellResults()->cellScalarResults(scalarResultIndex));
                size_t timeStepCount = rimCase->results(m_porosityModelEnum)->cellResults()->maxTimeStepCount();
                scalarResultFrames->resize(timeStepCount);

                m_currentScalarIndex = scalarResultIndex;
                m_currentPropertyName = propertyName;
            }
        }

        if (scalarResultFrames == NULL)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the %1 model property named: \"%2\"").arg(porosityModelName).arg(propertyName));
            return true;
        }

        // Create a list of all the requested timesteps

        m_requestedTimesteps.clear();

        if (args.size() <= 5)
        {
            // Select all
            for (size_t tsIdx = 0; tsIdx < m_timeStepCountToRead; ++tsIdx)
            {
                m_requestedTimesteps.push_back(tsIdx);
            }
        }
        else
        {
            bool timeStepReadError = false;
            for (int argIdx = 5; argIdx < args.size(); ++argIdx)
            {
                bool conversionOk = false;
                int tsIdx = args[argIdx].toInt(&conversionOk);

                if (conversionOk)
                {
                    m_requestedTimesteps.push_back(tsIdx);
                }
                else
                {
                    timeStepReadError = true;
                }
            }

            if (timeStepReadError)
            {
                server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: riGetActiveCellProperty : \n") + RiaSocketServer::tr("An error occured while interpreting the requested timesteps."));
                return true;
            }

        }

        if (! m_requestedTimesteps.size())
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("No time steps specified").arg(porosityModelName).arg(propertyName));

            return true;
        }

        m_currentReservoir = rimCase;
        m_scalarResultsToAdd = scalarResultFrames;

        if (server->currentClient()->bytesAvailable())
        {
            return this->interpretMore(server, server->currentClient());
        }

        return false;
    }

    virtual bool interpretMore(RiaSocketServer* server, QTcpSocket* currentClient)
    {

        if (m_invalidDataDetected){
            RiuMainWindow::instance()->processMonitor()->addStringToLog("[ResInsight SocketServer] > True \n");
            return true;
        }

        // If nothing should be read, or we already have read everything, do nothing

        if ((m_timeStepCountToRead == 0) || (m_currentTimeStepNumberToRead >= m_timeStepCountToRead) )  return true;

        if (!currentClient->bytesAvailable()) return false;

        RigGridBase* grid = m_currentReservoir->reservoirData()->grid(m_currentGridIndex);
        if (!grid)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") +
                RiaSocketServer::tr("No grid found") + ":\"" + m_currentReservoir->caseUserDescription() + "\"\n");

            m_invalidDataDetected = true;
            currentClient->abort(); // Hmmm... should we not let the server handle this ?

            return true;
        }

        if (m_timeStepCountToRead != m_requestedTimesteps.size())
        {
            CVF_ASSERT(false);
        }

        // Check if a complete timestep is available, return and wait for readyRead() if not
        if (currentClient->bytesAvailable() < (int)m_bytesPerTimeStepToRead) return false;

        size_t cellCountFromOctave = m_bytesPerTimeStepToRead / sizeof(double);

        if (cellCountFromOctave != grid->cellCount())
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") +
                RiaSocketServer::tr("Mismatch between expected and received data. Expected : %1, Received : %2").arg(grid->cellCount()).arg(cellCountFromOctave));

            m_invalidDataDetected = true;
            currentClient->abort();
            return true;
        }

        // Resize the timestep container
        {
            size_t maxRequestedTimeStepIdx = cvf::UNDEFINED_SIZE_T;
            for (size_t tIdx = 0; tIdx < m_timeStepCountToRead; ++tIdx)
            {
                size_t tsId = m_requestedTimesteps[tIdx];
                if (maxRequestedTimeStepIdx == cvf::UNDEFINED_SIZE_T || tsId > maxRequestedTimeStepIdx)
                {
                    maxRequestedTimeStepIdx = tsId;
                }
            }

            if (maxRequestedTimeStepIdx != cvf::UNDEFINED_SIZE_T
                && m_scalarResultsToAdd->size() <= maxRequestedTimeStepIdx)
            {
                m_scalarResultsToAdd->resize(maxRequestedTimeStepIdx + 1);
            }
        }

        for (size_t tIdx = 0; tIdx < m_timeStepCountToRead; ++tIdx)
        {
            size_t tsId = m_requestedTimesteps[tIdx];

            // Result data is stored in an array containing all cells for all grids
            // The size of this array must match the test in RigCaseCellResultsData::isUsingGlobalActiveIndex(),
            // as it is used to determine if we have data for active cells or all cells
            // See RigCaseCellResultsData::isUsingGlobalActiveIndex()
            size_t totalNumberOfCellsIncludingLgrCells = grid->mainGrid()->globalCellArray().size();

            m_scalarResultsToAdd->at(tsId).resize(totalNumberOfCellsIncludingLgrCells, HUGE_VAL);
        }

        while ((currentClient->bytesAvailable() >= (int)m_bytesPerTimeStepToRead) && (m_currentTimeStepNumberToRead < m_timeStepCountToRead))
        {
            // Read a single time step with data

            std::vector<double> doubleValues(cellCountFromOctave);

            QStringList errorMessages;
            if (!RiaSocketDataTransfer::readBlockDataFromSocket(currentClient, (char*)(doubleValues.data()), m_bytesPerTimeStepToRead, errorMessages))
            {
                for (int i = 0; i < errorMessages.size(); i++)
                {
                    server->errorMessageDialog()->showMessage(errorMessages[i]);
                }

                currentClient->abort();
                return true;
            }

            cvf::ref<RigResultModifier> resultModifier = RigResultModifierFactory::createResultModifier(m_currentReservoir->reservoirData(), grid->gridIndex(), m_porosityModelEnum, m_requestedTimesteps[m_currentTimeStepNumberToRead], m_currentScalarIndex);
            if (!resultModifier.isNull())
            {
                for (size_t cellIdx = 0; static_cast<size_t>(cellIdx) < cellCountFromOctave; cellIdx++)
                {
                    resultModifier->setCellScalar(cellIdx, doubleValues[cellIdx]);
                }
            }

            ++m_currentTimeStepNumberToRead;
        }

        // If we have read all the data, refresh the views

        if (m_currentTimeStepNumberToRead == m_timeStepCountToRead)
        {
            if (m_currentReservoir != NULL)
            {
                // Create a new input property if we have an input reservoir
                RimEclipseInputCase* inputRes = dynamic_cast<RimEclipseInputCase*>(m_currentReservoir);
                if (inputRes)
                {
                    RimEclipseInputProperty* inputProperty = NULL;
                    inputProperty = inputRes->m_inputPropertyCollection->findInputProperty(m_currentPropertyName);
                    if (!inputProperty)
                    {
                        inputProperty = new RimEclipseInputProperty;
                        inputProperty->resultName = m_currentPropertyName;
                        inputProperty->eclipseKeyword = "";
                        inputProperty->fileName = "";
                        inputRes->m_inputPropertyCollection->inputProperties.push_back(inputProperty);
                        inputRes->m_inputPropertyCollection()->updateConnectedEditors();
                    }
                    inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED_NOT_SAVED;
                }

                if( m_currentScalarIndex != cvf::UNDEFINED_SIZE_T &&
                    m_currentReservoir->reservoirData() &&
                    m_currentReservoir->reservoirData()->results(m_porosityModelEnum) )
                {
                    // Adjust the result data if only one time step is requested so the result behaves like a static result
                    if (m_requestedTimesteps.size() == 1 && m_currentScalarIndex != cvf::UNDEFINED_SIZE_T)
                    {
                        std::vector< std::vector<double> >* scalarResultFrames = NULL;
                        scalarResultFrames = &(m_currentReservoir->results(m_porosityModelEnum)->cellResults()->cellScalarResults(m_currentScalarIndex));
                        size_t lastIndexWithDataPresent = cvf::UNDEFINED_SIZE_T;
                        for (size_t i = 0; i < scalarResultFrames->size(); i++)
                        {
                            if ((*scalarResultFrames)[i].size() > 0)
                            {
                                lastIndexWithDataPresent = i;
                            }
                        }

                        if (lastIndexWithDataPresent == 0)
                        {
                            scalarResultFrames->resize(1);
                        }
                    }

                    m_currentReservoir->reservoirData()->results(m_porosityModelEnum)->recalculateStatistics(m_currentScalarIndex);
                }

                for (size_t i = 0; i < m_currentReservoir->reservoirViews.size(); ++i)
                {
                    if (m_currentReservoir->reservoirViews[i])
                    {
                        // As new result might have been introduced, update all editors connected
                        m_currentReservoir->reservoirViews[i]->cellResult->updateConnectedEditors();

                        // It is usually not needed to create new display model, but if any derived geometry based on generated data (from Octave) 
                        // a full display model rebuild is required
                        m_currentReservoir->reservoirViews[i]->scheduleCreateDisplayModelAndRedraw();
                    }
                }
            }

            return true;
        }

        
        return false;
    }

private:
    RimEclipseCase*                            m_currentReservoir;
    std::vector< std::vector<double> >* m_scalarResultsToAdd;
    size_t                              m_currentGridIndex;
    size_t                              m_currentScalarIndex;
    QString                             m_currentPropertyName;
    std::vector<size_t>                 m_requestedTimesteps;
    RifReaderInterface::PorosityModelResultType m_porosityModelEnum;

    quint64                             m_timeStepCountToRead;
    quint64                             m_bytesPerTimeStepToRead;
    size_t                              m_currentTimeStepNumberToRead;

    bool                                m_invalidDataDetected;
};

static bool RiaSetGridProperty_init = RiaSocketCommandFactory::instance()->registerCreator<RiaSetGridProperty>(RiaSetGridProperty::commandName());

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetPropertyNames : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetPropertyNames"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int caseId = args[1].toInt();
        RimEclipseCase* rimCase = server->findReservoir(caseId);
        if (!rimCase)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the case with ID : \"%1\"").arg(caseId));

            return true;
        }

        QString porosityModelName = args[2];
        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;

        if (porosityModelName == "Fracture")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        std::vector<QString> propNames;
        std::vector<QString> propTypes;

        RigCaseCellResultsData* results = rimCase->reservoirData()->results(porosityModelEnum);
       
        std::vector<RimDefines::ResultCatType> resTypes;
        std::vector<QString> resTypeNames;
        resTypes.push_back(RimDefines::DYNAMIC_NATIVE);
        resTypeNames.push_back("DynamicNative");
        resTypes.push_back(RimDefines::STATIC_NATIVE );
        resTypeNames.push_back("StaticNative");
        resTypes.push_back(RimDefines::GENERATED     );
        resTypeNames.push_back("Generated");
        resTypes.push_back(RimDefines::INPUT_PROPERTY);
        resTypeNames.push_back("Input");

        for (size_t rtIdx = 0; rtIdx < resTypes.size(); ++rtIdx)
        {
            RimDefines::ResultCatType resType = resTypes[rtIdx];

            QStringList names = results->resultNames(resType);
            for (int pnIdx = 0; pnIdx < names.size(); ++pnIdx){
                propNames.push_back(names[pnIdx]);
                propTypes.push_back(resTypeNames[rtIdx]);
            }
        }

        quint64 byteCount = sizeof(quint64);
        quint64 propCount = propNames.size();

        for (size_t rtIdx = 0; rtIdx < propCount; rtIdx++)
        {
            byteCount += propNames[rtIdx].size() * sizeof(QChar);
            byteCount += propTypes[rtIdx].size() * sizeof(QChar);
        }

        socketStream << byteCount;
        socketStream << propCount;

        for (size_t rtIdx = 0; rtIdx < propCount; rtIdx++)
        {
            socketStream << propNames[rtIdx];
            socketStream << propTypes[rtIdx];
        }


        return true;
    }
};

static bool RiaGetPropertyNames_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetPropertyNames>(RiaGetPropertyNames::commandName());
