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

#include "RimEclipseResultDefinition.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResultAddress.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimView.h"
#include "RimViewLinker.h"
#include "RimWellLogCurve.h"

#include "cafPdmUiListEditor.h"

namespace caf
{
    template<>
    void RimEclipseResultDefinition::FlowTracerSelectionEnum::setUp()
    {
        addItem(RimEclipseResultDefinition::FLOW_TR_INJ_AND_PROD,   "FLOW_TR_INJ_AND_PROD", "All Injectors and Producers");
        addItem(RimEclipseResultDefinition::FLOW_TR_PRODUCERS,      "FLOW_TR_PRODUCERS",    "All Producers");
        addItem(RimEclipseResultDefinition::FLOW_TR_INJECTORS,      "FLOW_TR_INJECTORS",    "All Injectors");
        addItem(RimEclipseResultDefinition::FLOW_TR_BY_SELECTION,   "FLOW_TR_BY_SELECTION", "By Selection");

        setDefault(RimEclipseResultDefinition::FLOW_TR_INJ_AND_PROD);
    }
}


CAF_PDM_SOURCE_INIT(RimEclipseResultDefinition, "ResultDefinition");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition::RimEclipseResultDefinition() 
{
    CAF_PDM_InitObject("Result Definition", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_resultType,     "ResultType",           "Type", "", "", "");
    m_resultType.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_porosityModel,  "PorosityModelType",    "Porosity", "", "", "");
    m_porosityModel.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_resultVariable, "ResultVariable", RimDefines::undefinedResultName(), "Variable", "", "", "" );
    m_resultVariable.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_flowSolution, "FlowDiagSolution", "Solution", "", "", "");
    m_flowSolution.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_selectedTracers, "SelectedTracers", "Tracers", "", "", "");
    m_selectedTracers.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_flowTracerSelectionMode, "FlowTracerSelectionMode", "Tracers", "", "", "");

    // Ui only fields

    CAF_PDM_InitFieldNoDefault(&m_resultTypeUiField,     "MResultType",           "Type", "", "", "");
    m_resultTypeUiField.xmlCapability()->setIOReadable(false);
    m_resultTypeUiField.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_porosityModelUiField,  "MPorosityModelType",    "Porosity", "", "", "");
    m_porosityModelUiField.xmlCapability()->setIOReadable(false);
    m_porosityModelUiField.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_resultVariableUiField, "MResultVariable", RimDefines::undefinedResultName(), "Result property", "", "", "" );
    m_resultVariableUiField.xmlCapability()->setIOReadable(false);
    m_resultVariableUiField.xmlCapability()->setIOWritable(false);
    m_resultVariableUiField.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());


    CAF_PDM_InitFieldNoDefault(&m_flowSolutionUiField, "MFlowDiagSolution", "Solution", "", "", "");
    m_flowSolutionUiField.xmlCapability()->setIOReadable(false);
    m_flowSolutionUiField.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_selectedTracersUiField, "MSelectedTracers", " ", "", "", "");
    m_selectedTracersUiField.xmlCapability()->setIOReadable(false);
    m_selectedTracersUiField.xmlCapability()->setIOWritable(false);
    m_selectedTracersUiField.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition::~RimEclipseResultDefinition()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::simpleCopy(const RimEclipseResultDefinition* other)
{
    this->setResultVariable(other->resultVariable());
    this->setPorosityModel(other->porosityModel());
    this->setResultType(other->resultType());
    this->setFlowSolution(other->m_flowSolution());
    this->setSelectedTracers(other->m_selectedTracers());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setEclipseCase(RimEclipseCase* eclipseCase)
{
     m_eclipseCase = eclipseCase;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage* RimEclipseResultDefinition::currentGridCellResults() const
{
    if (!m_eclipseCase ) return nullptr;

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_porosityModel());

    return m_eclipseCase->results(porosityModel);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (   &m_flowSolutionUiField  == changedField 
        || &m_resultTypeUiField    == changedField
        || &m_porosityModelUiField == changedField )
    {
        // If the user are seeing the list with the actually selected result, 
        // select that result in the list. Otherwise select nothing.

        QStringList varList = getResultNamesForCurrentUiResultType();

        bool isFlowDiagFieldsRelevant = (m_resultType() == RimDefines::FLOW_DIAGNOSTICS);
        

        if (   ( m_flowSolutionUiField() == m_flowSolution() || !isFlowDiagFieldsRelevant)
            &&  m_resultTypeUiField()    == m_resultType()
            &&  m_porosityModelUiField() == m_porosityModel() )
        {
            if (varList.contains(resultVariable()))
            {
                m_resultVariableUiField = resultVariable();
            }

            if (isFlowDiagFieldsRelevant) m_selectedTracersUiField = m_selectedTracers();
            else                          m_selectedTracersUiField = std::vector<QString>();
        }
        else
        {
            m_resultVariableUiField = "";
            m_selectedTracersUiField = std::vector<QString>();
        }
    }

    if (&m_resultVariableUiField == changedField)
    {
        m_porosityModel  = m_porosityModelUiField;
        m_resultType     = m_resultTypeUiField;
        m_resultVariable = m_resultVariableUiField;
        
        if (m_resultTypeUiField() == RimDefines::FLOW_DIAGNOSTICS)
        {
            m_flowSolution = m_flowSolutionUiField();
            m_selectedTracers = m_selectedTracersUiField();
        }
        updateResultNameHasChanged();
    }

    if ( &m_selectedTracersUiField == changedField )
    {
        m_flowSolution = m_flowSolutionUiField();
        m_selectedTracers = m_selectedTracersUiField();
        updateResultNameHasChanged();
    }

    updateAnyFieldHasChanged();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::updateAnyFieldHasChanged()
{
    RimEclipsePropertyFilter* propFilter = nullptr;
    this->firstAncestorOrThisOfType(propFilter);
    if (propFilter)
    {
        propFilter->updateConnectedEditors();
    }
    
    RimEclipseFaultColors* faultColors = nullptr;
    this->firstAncestorOrThisOfType(faultColors);
    if (faultColors)
    {
        faultColors->updateConnectedEditors();
    }

    RimCellEdgeColors* cellEdgeColors = nullptr;
    this->firstAncestorOrThisOfType(cellEdgeColors);
    if (cellEdgeColors)
    {
        cellEdgeColors->updateConnectedEditors();
    }

    RimEclipseCellColors* cellColors = nullptr;
    this->firstAncestorOrThisOfType(cellColors);
    if (cellColors)
    {
        cellColors->updateConnectedEditors();
    }

    RimWellLogCurve* curve = nullptr;
    this->firstAncestorOrThisOfType(curve);
    if (curve)
    {
        curve->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::updateResultNameHasChanged()
{
    RimView* view = nullptr;
    this->firstAncestorOrThisOfType(view);

    loadResult();

    RimEclipsePropertyFilter* propFilter = nullptr;
    this->firstAncestorOrThisOfType(propFilter);
    if (propFilter)
    {
        propFilter->setToDefaultValues();
        propFilter->updateFilterName();

        if (view)
        {
            view->scheduleGeometryRegen(PROPERTY_FILTERED);
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }

    RimEclipseCellColors* cellColors = nullptr;
    this->firstAncestorOrThisOfType(cellColors);
    if (cellColors)
    {
        this->updateLegendCategorySettings();

        if (view)
        {
            RimViewLinker* viewLinker = view->assosiatedViewLinker();
            if (viewLinker)
            {
                viewLinker->updateCellResult();
            }
        }
    }

    RimCellEdgeColors* cellEdgeColors = nullptr;
    this->firstAncestorOrThisOfType(cellEdgeColors);
    if (cellEdgeColors)
    {
        cellEdgeColors->singleVarEdgeResultColors()->updateLegendCategorySettings();
        cellEdgeColors->loadResult();

        if (view)
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }

    RimWellLogCurve* curve = nullptr;
    this->firstAncestorOrThisOfType(curve);
    if (curve)
    {
        curve->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipseResultDefinition::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_resultTypeUiField )
    {

        bool hasFlowDiagFluxes = false;
        RimEclipseResultCase* eclResCase = dynamic_cast<RimEclipseResultCase*>(m_eclipseCase.p());
        if ( eclResCase && eclResCase->reservoirData() )
        {
            hasFlowDiagFluxes = eclResCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->hasFlowDiagUsableFluxes();
        }

        // Do not include flow diag results if not available

        if ( !hasFlowDiagFluxes )
        {
            using ResCatEnum = caf::AppEnum< RimDefines::ResultCatType >;
            for ( int i = 0; i < ResCatEnum::size(); ++i )
            {
                RimDefines::ResultCatType resType = ResCatEnum::fromIndex(i);
                if ( resType != RimDefines::FLOW_DIAGNOSTICS )
                {
                    QString uiString = ResCatEnum::uiTextFromIndex(i);
                    options.push_back(caf::PdmOptionItemInfo(uiString, resType));
                }
            }
        }
        else
        {
            // Do nothing, and thereby use the defaults of the AppEnum field
        }
    }

    if ( m_resultTypeUiField() != RimDefines::FLOW_DIAGNOSTICS )
    {
        if ( fieldNeedingOptions == &m_resultVariableUiField )
        {
                options = calcOptionsForVariableUiFieldStandard();
        }
    }
    else
    {
        if ( fieldNeedingOptions == &m_resultVariableUiField )
        {
            options.push_back(caf::PdmOptionItemInfo("Time Of Flight (Average)",   RIG_FLD_TOF_RESNAME));
            options.push_back(caf::PdmOptionItemInfo("Tracer Cell Fraction (Sum)",      RIG_FLD_CELL_FRACTION_RESNAME));
            options.push_back(caf::PdmOptionItemInfo("Max Fraction Tracer",             RIG_FLD_MAX_FRACTION_TRACER_RESNAME));
            options.push_back(caf::PdmOptionItemInfo("Injector Producer Communication", RIG_FLD_COMMUNICATION_RESNAME));
        }
        else if (fieldNeedingOptions == &m_flowSolutionUiField)
        {
            RimEclipseResultCase* eclCase;
            this->firstAncestorOrThisOfType(eclCase);
            if (eclCase)
            {
                std::vector<RimFlowDiagSolution*> flowSols = eclCase->flowDiagSolutions();
                for (RimFlowDiagSolution* flowSol : flowSols)
                {
                    options.push_back(caf::PdmOptionItemInfo(flowSol->userDescription(), flowSol));
                }
            }           
        }
        else if (fieldNeedingOptions == &m_selectedTracersUiField)
        {
            RimFlowDiagSolution* flowSol = m_flowSolutionUiField();
            if (flowSol)
            {
                std::vector<QString> tracerNames = flowSol->tracerNames();
                std::map<QString, QString> prefixedTracerNamesMap;
                for ( const QString& tracerName : tracerNames )
                {
                    RimFlowDiagSolution::TracerStatusType status = flowSol->tracerStatusOverall(tracerName);
                    QString prefix; 
                    switch (status)
                    {
                    case RimFlowDiagSolution::INJECTOR: prefix = "I   : "; break;
                    case RimFlowDiagSolution::PRODUCER: prefix = "P  : "; break;
                    case RimFlowDiagSolution::VARYING:  prefix = "I/P: "; break;
                    }

                    if (status != RimFlowDiagSolution::CLOSED) prefixedTracerNamesMap[prefix + tracerName] = tracerName;
                }

                for (auto nameIt: prefixedTracerNamesMap)
                {
                    options.push_back(caf::PdmOptionItemInfo(nameIt.first, QVariant(nameIt.second)));
                }
            }
        }
    }

    (*useOptionsOnly) = true;
    
    return options;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipseResultDefinition::calcOptionsForVariableUiFieldStandard()
{
    CVF_ASSERT(m_resultTypeUiField() != RimDefines::FLOW_DIAGNOSTICS);

    if (this->currentGridCellResults())
    {
        QList<caf::PdmOptionItemInfo> optionList;

        QStringList cellCenterResultNames;
        QStringList cellFaceResultNames;

        foreach(QString s, getResultNamesForCurrentUiResultType())
        {
            if (RimDefines::isPerCellFaceResult(s))
            {
                cellFaceResultNames.push_back(s);
            }
            else
            {
                cellCenterResultNames.push_back(s);
            }
        }

        cellCenterResultNames.sort();
        cellFaceResultNames.sort();

        // Cell Center result names
        foreach(QString s, cellCenterResultNames)
        {
            optionList.push_back(caf::PdmOptionItemInfo(s, s));
        }

        // Ternary Result
        bool hasAtLeastOneTernaryComponent = false;
        if (cellCenterResultNames.contains("SOIL")) hasAtLeastOneTernaryComponent = true;
        else if (cellCenterResultNames.contains("SGAS")) hasAtLeastOneTernaryComponent = true;
        else if (cellCenterResultNames.contains("SWAT")) hasAtLeastOneTernaryComponent = true;

        if (m_resultTypeUiField == RimDefines::DYNAMIC_NATIVE && hasAtLeastOneTernaryComponent)
        {
            optionList.push_front(caf::PdmOptionItemInfo(RimDefines::ternarySaturationResultName(), RimDefines::ternarySaturationResultName()));
        }

        // Cell Face result names
        bool showDerivedResultsFirstInList = false;
        {
            RimEclipseFaultColors* rimEclipseFaultColors = nullptr;
            this->firstAncestorOrThisOfType(rimEclipseFaultColors);

            if ( rimEclipseFaultColors ) showDerivedResultsFirstInList = true;
        }

        foreach(QString s, cellFaceResultNames)
        {
            if (showDerivedResultsFirstInList)
            {
                optionList.push_front(caf::PdmOptionItemInfo(s, s));
            }
            else
            {
                optionList.push_back(caf::PdmOptionItemInfo(s, s));
            }
        }

        optionList.push_front(caf::PdmOptionItemInfo(RimDefines::undefinedResultName(), RimDefines::undefinedResultName()));

        // Remove Per Cell Face options
        {
            RimWellLogCurve* curve = nullptr;
            this->firstAncestorOrThisOfType(curve);

            RimEclipsePropertyFilter* propFilter = nullptr;
            this->firstAncestorOrThisOfType(propFilter);

            RimCellEdgeColors* cellEdge = nullptr;
            this->firstAncestorOrThisOfType(cellEdge);

            if ( propFilter || curve || cellEdge )
            {
                removePerCellFaceOptionItems(optionList);
            }
        }

        return optionList;
    }

    return QList<caf::PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseResultDefinition::getResultNamesForCurrentUiResultType()
{
    if ( m_resultTypeUiField() != RimDefines::FLOW_DIAGNOSTICS )
    {
        RimReservoirCellResultsStorage* cellResultsStorage = currentGridCellResults();

        if ( !cellResultsStorage ) return QStringList();

        if ( !cellResultsStorage->cellResults() ) return QStringList();

        return cellResultsStorage->cellResults()->resultNames(m_resultTypeUiField());
    }
    else
    {
        QStringList flowVars;
        flowVars.push_back(RIG_FLD_TOF_RESNAME);
        flowVars.push_back(RIG_FLD_CELL_FRACTION_RESNAME);
        flowVars.push_back(RIG_FLD_MAX_FRACTION_TRACER_RESNAME);
        flowVars.push_back(RIG_FLD_COMMUNICATION_RESNAME);
        return flowVars;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimEclipseResultDefinition::scalarResultIndex() const
{
    size_t gridScalarResultIndex = cvf::UNDEFINED_SIZE_T;

    if (m_resultType() == RimDefines::FLOW_DIAGNOSTICS) return cvf::UNDEFINED_SIZE_T;

    const RimReservoirCellResultsStorage* gridCellResults = this->currentGridCellResults();
    if (gridCellResults && gridCellResults->cellResults())
    {
        gridScalarResultIndex = gridCellResults->cellResults()->findScalarResultIndex(m_resultType(), m_resultVariable());
    }

    return gridScalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagResultAddress RimEclipseResultDefinition::flowDiagResAddress() const
{
    CVF_ASSERT(m_resultType() == RimDefines::FLOW_DIAGNOSTICS);

    RimView* rimView = nullptr;
    this->firstAncestorOrThisOfType(rimView);

    size_t timeStep = rimView->currentTimeStep();

    std::set<std::string> selTracerNames;
    if (m_flowTracerSelectionMode == FLOW_TR_BY_SELECTION)
    {
        for (const QString& tName : m_selectedTracers())
        {
            selTracerNames.insert(tName.toStdString());
        }
    }
    else
    {
        RimFlowDiagSolution* flowSol = m_flowSolutionUiField();
        if (flowSol)
        {
            std::vector<QString> tracerNames = flowSol->tracerNames();

            if (m_flowTracerSelectionMode == FLOW_TR_INJECTORS || m_flowTracerSelectionMode == FLOW_TR_INJ_AND_PROD)
            {
                for (const QString& tracerName : tracerNames)
                {
                    RimFlowDiagSolution::TracerStatusType status = flowSol->tracerStatusInTimeStep(tracerName, timeStep);
                    if (status == RimFlowDiagSolution::INJECTOR)
                    {
                        selTracerNames.insert(tracerName.toStdString());
                    }
                }
            }
            
            if (m_flowTracerSelectionMode == FLOW_TR_PRODUCERS || m_flowTracerSelectionMode == FLOW_TR_INJ_AND_PROD)
            {
                for (const QString& tracerName : tracerNames)
                {
                    RimFlowDiagSolution::TracerStatusType status = flowSol->tracerStatusInTimeStep(tracerName, timeStep);
                    if (status == RimFlowDiagSolution::PRODUCER)
                    {
                        selTracerNames.insert(tracerName.toStdString());
                    }
                }
            }
        }
    }

    return RigFlowDiagResultAddress(m_resultVariable().toStdString(), selTracerNames);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinition::resultVariableUiName() const
{
    if (resultType() == RimDefines::FLOW_DIAGNOSTICS)
    {
        QString fullName;

        if (m_flowTracerSelectionMode() == FLOW_TR_BY_SELECTION)
        {
            fullName = QString::fromStdString(flowDiagResAddress().uiText());
        }
        else
        {
            fullName = QString::fromStdString(flowDiagResAddress().uiShortText());
            fullName += QString(" (%1)").arg(m_flowTracerSelectionMode().uiText());
        }

        return fullName;
    }

    return m_resultVariable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinition::resultVariableUiShortName() const
{
    if (resultType() == RimDefines::FLOW_DIAGNOSTICS)
    {
        QString shortName;

        if (m_flowTracerSelectionMode() == FLOW_TR_BY_SELECTION)
        {
            QString candidate = QString::fromStdString(flowDiagResAddress().uiText());

            int stringSizeLimit = 32;
            if (candidate.size() > stringSizeLimit)
            {
                candidate = candidate.left(stringSizeLimit);
                candidate += "...";
            }

            shortName = candidate;
        }
        else
        {
            shortName = QString::fromStdString(flowDiagResAddress().uiShortText());
            shortName += QString(" (%1)").arg(m_flowTracerSelectionMode().uiText());
        }

        return shortName;
    }

    return m_resultVariable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::loadResult()
{
    if (m_resultType() == RimDefines::FLOW_DIAGNOSTICS) return; // Will load automatically on access

    RimReservoirCellResultsStorage* gridCellResults = this->currentGridCellResults();
    if (gridCellResults)
    {
        gridCellResults->findOrLoadScalarResult(m_resultType(), m_resultVariable);
    }

}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is a single frame result 
/// The result needs to be loaded before asking
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasStaticResult() const
{
    if (this->resultType() == RimDefines::FLOW_DIAGNOSTICS) return false;

    const RimReservoirCellResultsStorage* gridCellResults = this->currentGridCellResults();
    size_t gridScalarResultIndex = this->scalarResultIndex();

    if (hasResult() && gridCellResults->cellResults()->timeStepCount(gridScalarResultIndex) == 1 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is loaded or possible to load from the result file
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasResult() const
{
    if (this->resultType() == RimDefines::FLOW_DIAGNOSTICS)
    {
        if (m_flowSolution() && !m_resultVariable().isEmpty()) return true;
    }
    else if (this->currentGridCellResults() && this->currentGridCellResults()->cellResults())
    {
        const RigCaseCellResultsData* gridCellResults = this->currentGridCellResults()->cellResults();
        size_t gridScalarResultIndex = gridCellResults->findScalarResultIndex(m_resultType(), m_resultVariable());
        return gridScalarResultIndex != cvf::UNDEFINED_SIZE_T;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is a multi frame result 
/// The result needs to be loaded before asking
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasDynamicResult() const
{
    if (hasResult())
    {
        if (m_resultType() == RimDefines::DYNAMIC_NATIVE)
        {
            return true;
        }
        else if (m_resultType() == RimDefines::FLOW_DIAGNOSTICS)
        {
            return true;
        }

        if (this->currentGridCellResults() && this->currentGridCellResults()->cellResults())
        {
            const RigCaseCellResultsData* gridCellResults = this->currentGridCellResults()->cellResults();
            size_t gridScalarResultIndex = this->scalarResultIndex();
            if (gridCellResults->timeStepCount(gridScalarResultIndex) > 1 )
            {
                return true;
            }
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::initAfterRead()
{
    m_porosityModelUiField = m_porosityModel;
    m_resultTypeUiField = m_resultType;
    m_resultVariableUiField = m_resultVariable;

    m_flowSolutionUiField = m_flowSolution();
    m_selectedTracersUiField = m_selectedTracers;

    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setResultType(RimDefines::ResultCatType val)
{
    m_resultType = val;
    m_resultTypeUiField = val;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setPorosityModel(RimDefines::PorosityModelType val)
{
    m_porosityModel = val;
    m_porosityModelUiField = val;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setResultVariable(const QString& val)
{
    m_resultVariable = val;
    m_resultVariableUiField = val;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution* RimEclipseResultDefinition::flowDiagSolution()
{
    return m_flowSolution();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setFlowSolution(RimFlowDiagSolution* flowSol)
{
    this->m_flowSolution = flowSol;
    this->m_flowSolutionUiField = flowSol;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setSelectedTracers(const std::vector<QString>& selectedTracers)
{
    this->m_selectedTracers = selectedTracers;
    this->m_selectedTracersUiField = selectedTracers;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isTernarySaturationSelected() const
{
    bool isTernary =    (m_resultType() == RimDefines::DYNAMIC_NATIVE) && 
                        (m_resultVariable().compare(RimDefines::ternarySaturationResultName(), Qt::CaseInsensitive) == 0);

    return isTernary;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasCategoryResult() const
{
    if (this->m_resultType() == RimDefines::FORMATION_NAMES
        && m_eclipseCase 
        && m_eclipseCase->reservoirData() 
        && m_eclipseCase->reservoirData()->activeFormationNames() ) return true;

    if (this->m_resultType() == RimDefines::FLOW_DIAGNOSTICS
        && m_resultVariable() == RIG_FLD_MAX_FRACTION_TRACER_RESNAME) return true;

    if (!this->hasStaticResult()) return false;

    return this->resultVariable().contains("NUM", Qt::CaseInsensitive);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasDualPorFractureResult()
{
    if ( m_eclipseCase
        && m_eclipseCase->reservoirData()
        && m_eclipseCase->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS) 
        && m_eclipseCase->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS)->reservoirActiveCellCount() > 0 )
        {
            return true;
        } 

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_resultTypeUiField);

    if (hasDualPorFractureResult()) 
    {
        uiOrdering.add(&m_porosityModelUiField);
    }

    if ( m_resultTypeUiField() == RimDefines::FLOW_DIAGNOSTICS )
    { 
        uiOrdering.add(&m_flowSolutionUiField);

        uiOrdering.add(&m_flowTracerSelectionMode);
        
        if (m_flowTracerSelectionMode == FLOW_TR_BY_SELECTION)
        {
            uiOrdering.add(&m_selectedTracersUiField);
        }

        if ( m_flowSolution() == nullptr )
        {
            RimEclipseResultCase* eclCase;
            this->firstAncestorOrThisOfType(eclCase);
            if ( eclCase )
            {
                std::vector<RimFlowDiagSolution*> flowSols = eclCase->flowDiagSolutions();
                if (flowSols.size()){ this->setFlowSolution(flowSols[0]); } 
            }
        }
    }
    uiOrdering.add(&m_resultVariableUiField);

    uiOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::removePerCellFaceOptionItems(QList<caf::PdmOptionItemInfo>& optionItems)
{
    std::vector<int> indicesToRemove;
    for (int i = 0; i < optionItems.size(); i++)
    {
        QString text = optionItems[i].optionUiText;

        if (RimDefines::isPerCellFaceResult(text))
        {
            indicesToRemove.push_back(i);
        }
    }

    std::sort(indicesToRemove.begin(), indicesToRemove.end());

    std::vector<int>::reverse_iterator rit;
    for (rit = indicesToRemove.rbegin(); rit != indicesToRemove.rend(); ++rit)
    {
        optionItems.takeAt(*rit);
    }
}

