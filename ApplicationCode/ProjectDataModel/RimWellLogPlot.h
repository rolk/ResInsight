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

#pragma once

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"
#include "cafAppEnum.h"

#include "RimDefines.h"
#include "RimViewWindow.h"

#include <QPointer>

class RiuWellLogPlot;
class RimWellLogTrack;


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    enum DepthTypeEnum
    {
        MEASURED_DEPTH,
        TRUE_VERTICAL_DEPTH
    };


public:
    RimWellLogPlot();
    virtual ~RimWellLogPlot();

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    DepthTypeEnum                                   depthType() const;

    RimDefines::DepthUnitType                       depthUnit() const;
    void                                            setDepthUnit(RimDefines::DepthUnitType depthUnit);

    QString                                         depthPlotTitle() const;

    void                                            addTrack(RimWellLogTrack* track);
    void                                            insertTrack(RimWellLogTrack* track, size_t index);
    size_t                                          trackCount() { return m_tracks.size();}
    void                                            removeTrackByIndex(size_t index);

    void                                            removeTrack(RimWellLogTrack* track);
    size_t                                          trackIndex(RimWellLogTrack* track);
    void                                            moveTracks(RimWellLogTrack* insertAfterTrack, const std::vector<RimWellLogTrack*>& tracksToMove);

    RimWellLogTrack*                                trackByIndex(size_t index);

    virtual void                                    loadDataAndUpdate() override;
    void                                            updateTracks();
    void                                            updateTrackNames();

    void                                            updateDepthZoom();
    void                                            setDepthZoomByFactorAndCenter(double zoomFactor, double zoomCenter);
    void                                            panDepth(double panFactor);
    void                                            setDepthZoomMinMax(double minimumDepth, double maximumDepth);
    void                                            depthZoomMinMax(double* minimumDepth, double* maximumDepth) const;

    void                                            calculateAvailableDepthRange();
    void                                            availableDepthRange(double* minimumDepth, double* maximumDepth) const;
    bool                                            hasAvailableDepthRange() const;

    virtual void                                    zoomAll() override;
    virtual QWidget*                                viewWidget() override;

protected:

    // Overridden PDM methods
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual caf::PdmFieldHandle*                    userDescriptionField()  { return &m_userName; }

    virtual QImage                                  snapshotWindowContent() override;


private:
    void                                            applyZoomAllDepths();
    void                                            applyDepthZoomFromVisibleDepth();
    void                                            recreateTrackPlots();
    void                                            detachAllCurves();

public: // Needed by RiuWellAllocation Plot
    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

private:
    caf::PdmField<QString>                          m_userName;
    
    caf::PdmField< caf::AppEnum< DepthTypeEnum > >              m_depthType;
    caf::PdmField< caf::AppEnum< RimDefines::DepthUnitType > >  m_depthUnit;

    caf::PdmChildArrayField<RimWellLogTrack*>       m_tracks;

    caf::PdmField<double>                           m_minVisibleDepth;
    caf::PdmField<double>                           m_maxVisibleDepth;
    caf::PdmField<bool>                             m_isAutoScaleDepthEnabled;

    double                                          m_minAvailableDepth;
    double                                          m_maxAvailableDepth;

    friend class RiuWellLogPlot;
    QPointer<RiuWellLogPlot>                        m_viewer;

};
