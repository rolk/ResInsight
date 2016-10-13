/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"
#include "cafPdmChildArrayField.h"
#include "cafAppEnum.h"
#include "cafPdmPtrArrayField.h"

#include "RifEclipseSummaryAddress.h"
#include "RimDefines.h"
#include "RimSummaryCurveAppearanceCalculator.h"

#include "qwt_plot.h"

class QwtPlot;
class QwtPlotCurve;
class RifReaderEclipseSummary;
class RimSummaryCase;
class RimSummaryCurve;
class RimSummaryFilter;
class RiuLineSegmentQwtPlotCurve;

#include <QPointer>


Q_DECLARE_METATYPE(RifEclipseSummaryAddress);

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCurveFilter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurveFilter();
    virtual ~RimSummaryCurveFilter();

    void                                    createCurves(RimSummaryCase* summaryCase, const QString& stringFilter);
    bool                                    isCurvesVisible();

    void                                    loadDataAndUpdate();
    void                                    setParentQwtPlot(QwtPlot* plot);
    void                                    detachQwtCurves();

    RimSummaryCurve*                        findRimCurveFromQwtCurve(const QwtPlotCurve* qwtCurve) const;
    std::set<std::string>                   unitNames();

    void                                    updateCaseNameHasChanged();

    RimDefines::PlotAxis                    associatedPlotAxis() const;
    void                                    setPlotAxis(RimDefines::PlotAxis plotAxis);

private:
    void                                    syncCurvesFromUiSelection();
    void                                    createCurvesFromCurveDefinitions(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefinitions);
    void                                    syncUiSelectionFromCurves();
    std::set<RifEclipseSummaryAddress>      findPossibleSummaryAddresses();
    
    void                                    createSetOfCasesAndResultAdresses(
                                                   const std::vector<RimSummaryCase*>& cases,
                                                   const RimSummaryFilter& filter,
                                                   std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >* curveDefinitions) const;

    // Overridden PDM methods
    virtual caf::PdmFieldHandle*            objectToggleField() override;
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly); 
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute) override;
    void                                    updatePlotAxisForCurves();

private:
    QPointer<QwtPlot>                       m_parentQwtPlot;

    // Fields
    caf::PdmField<bool>                       m_showCurves;
    caf::PdmPtrArrayField<RimSummaryCase*>    m_selectedSummaryCases;
    caf::PdmChildArrayField<RimSummaryCurve*> m_curves;

    caf::PdmField<QString>                  m_selectedVariableDisplayField;
    
    caf::PdmField< caf::AppEnum< RimDefines::PlotAxis > > m_plotAxis;


    // Filter fields
    caf::PdmChildField<RimSummaryFilter*>   m_summaryFilter;
    caf::PdmField<std::vector<RifEclipseSummaryAddress> > 
                                            m_uiFilterResultMultiSelection;

    caf::PdmField<bool>                     m_applyButtonField;

    caf::PdmField<bool>                     m_useAutoAppearanceAssignment;
    typedef caf::AppEnum<RimSummaryCurveAppearanceCalculator::CurveAppearanceType> AppearanceTypeAppEnum;
    caf::PdmField< AppearanceTypeAppEnum >  m_caseAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_variableAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_wellAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_groupAppearanceType;
    caf::PdmField< AppearanceTypeAppEnum >  m_regionAppearanceType;
};

