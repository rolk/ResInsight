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

#include "qwt_plot.h"

class QwtPlotCurve;
class QwtPlotGrid;

namespace cvf
{
    class Color3f;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RiuResultQwtPlot : public QwtPlot
{
public:
    explicit RiuResultQwtPlot(QWidget* parent = NULL);
    virtual ~RiuResultQwtPlot();

    void addCurve(const QString& curveName, const cvf::Color3f& curveColor, const std::vector<QDateTime>& dateTimes, const std::vector<double>& timeHistoryValues);
    void addCurve(const QString& curveName, const cvf::Color3f& curveColor, const std::vector<double>& frameTimes, const std::vector<double>& timeHistoryValues);
    
    void deleteAllCurves();

protected:
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    virtual void contextMenuEvent(QContextMenuEvent *) override;

private:
    void setDefaults();

private:
    std::vector<QwtPlotCurve*>  m_plotCurves;
};

