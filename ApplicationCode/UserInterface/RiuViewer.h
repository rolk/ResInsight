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

#pragma once

#include "cafViewer.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cafMouseState.h"
#include "cvfStructGrid.h"

class RimView;
class QLabel;
class QProgressBar;
class RiuSimpleHistogramWidget;
class QCDEStyle;
class RiuViewerCommands;

namespace cvf
{
    class Part;
    class OverlayItem;
}

//==================================================================================================
//
// RiuViewer
//
//==================================================================================================
class RiuViewer : public caf::Viewer
{
    Q_OBJECT

public:
    RiuViewer(const QGLFormat& format, QWidget* parent);
    ~RiuViewer();

    void            setDefaultView();
    cvf::Vec3d      pointOfInterest();
    void            setPointOfInterest(cvf::Vec3d poi);
    void            setOwnerReservoirView(RimView * owner);
    void            setEnableMask(unsigned int mask);

    void            showInfoText(bool enable);
    void            setInfoText(QString text);
    void            showHistogram(bool enable);
    void            setHistogram(double min, double max, const std::vector<size_t>& histogram);
    void            setHistogramPercentiles(double pmin, double pmax, double mean);

    void            showAnimationProgress(bool enable);
    
    void            removeAllColorLegends();
    void            addColorLegendToBottomLeftCorner(cvf::OverlayItem* legend);

    void            updateNavigationPolicy();

    
    virtual void    update();               // Override of caf::Viewer::update()
    void            issueBaseClassUpdate();

public slots:
    virtual void    slotSetCurrentFrame(int frameIndex);
    virtual void    slotEndAnimation();

private:
    void            paintOverlayItems(QPainter* painter);

    void            mouseReleaseEvent(QMouseEvent* event);
    void            mousePressEvent(QMouseEvent* event);


    QLabel*         m_InfoLabel;
    QLabel*         m_versionInfoLabel;
    bool            m_showInfoText;; 

    QProgressBar*   m_animationProgress;
    bool            m_showAnimProgress; 
    RiuSimpleHistogramWidget* m_histogramWidget;
    bool            m_showHistogram;

    QCDEStyle*      m_progressBarStyle;


    cvf::Collection<cvf::OverlayItem> m_visibleLegends;

    caf::PdmPointer<RimView> m_reservoirView;
    QPoint          m_lastMousePressPosition;

    RiuViewerCommands * m_viewerCommands;
};

