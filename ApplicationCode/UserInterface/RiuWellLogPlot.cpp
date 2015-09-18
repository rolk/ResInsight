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

#include "RiuWellLogPlot.h"

#include "RiuWellLogTracePlot.h"
#include "RiuMainWindow.h"

#include "RimWellLogPlot.h"
#include "RimWellLogPlotTrace.h"

#include "cafPdmUiTreeView.h"
#include "cvfAssert.h"

#include <QHBoxLayout>
#include <QScrollBar>
#include <QFocusEvent>

#include <math.h>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::RiuWellLogPlot(RimWellLogPlot* plotDefinition, QWidget* parent)
    : QWidget(parent)
{
    Q_ASSERT(plotDefinition);
    m_plotDefinition = plotDefinition;

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);
    setLayout(m_layout);

    m_scrollBar = new QScrollBar(this);
    m_scrollBar->setOrientation(Qt::Vertical);
    m_scrollBar->setVisible(false);
    m_layout->addWidget(m_scrollBar);
    
    connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(slotSetMinDepth(int)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::~RiuWellLogPlot()
{
    if (m_plotDefinition)
    {
        m_plotDefinition->handleViewerDeletion();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::insertTracePlot(RiuWellLogTracePlot* tracePlot)
{
    // Insert the plot to the left of the scroll bar
    m_layout->insertWidget(m_layout->count() - 1, tracePlot);
    m_tracePlots.append(tracePlot);  
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::setDepthRangeAndReplot(double minDepth, double maxDepth)
{
    for (int tpIdx = 0; tpIdx < m_tracePlots.count(); tpIdx++)
    {
        m_tracePlots[tpIdx]->setDepthRange(minDepth, maxDepth);
        m_tracePlots[tpIdx]->replot();
    }

    updateScrollBar(minDepth, maxDepth);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateScrollBar(double minDepth, double maxDepth)
{
    double availableMinDepth;
    double availableMaxDepth;
    m_plotDefinition->availableDepthRange(&availableMinDepth, &availableMaxDepth);

    double availableDepth = availableMaxDepth - availableMinDepth;
    double visibleDepth = maxDepth - minDepth;

    m_scrollBar->setRange((int) availableMinDepth, (int) (ceil(availableMaxDepth - visibleDepth)));
    m_scrollBar->setPageStep((int) visibleDepth);
    m_scrollBar->setValue((int) minDepth);

    m_scrollBar->setVisible(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::slotSetMinDepth(int value)
{
    double minimumDepth;
    double maximumDepth;
    m_plotDefinition->visibleDepthRange(&minimumDepth, &maximumDepth);

    double delta = value - minimumDepth;
    m_plotDefinition->setVisibleDepthRange(minimumDepth + delta, maximumDepth + delta);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RiuWellLogPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}
