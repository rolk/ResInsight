#include "RiuSimpleHistogramWidget.h"
#include <QPainter>
#include <cmath>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSimpleHistogramWidget::RiuSimpleHistogramWidget(QWidget * parent /*= 0*/, Qt::WindowFlags f /*= 0*/):
QWidget(parent, f)
{
    m_minPercentile = HUGE_VAL;
    m_maxPercentile = HUGE_VAL;
    m_mean = HUGE_VAL;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSimpleHistogramWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
   this->draw(&painter, 0, 0, this->width()-1, this->height()-1);
}



void RiuSimpleHistogramWidget::draw(QPainter *painter,int x, int y, int width, int height )
{

    // Initialize variables
    m_x = x; m_y = y; m_width = width; m_height = height;

    QRect r1(x,y,width, height);

    // Frame around it all;
    QColor windowColor = palette().color(QPalette::Window);// QColor(144, 173, 208, 180);
    QColor frameColor = palette().color(QPalette::WindowText);//QColor(220, 240, 255, 100);
    QColor foregroundColor = palette().color(QPalette::Dark);// QColor(100, 141, 189);

    //painter->fillRect(r1, windowColor);
    painter->setPen(frameColor);
    painter->drawRect(r1);

    // Columns
    painter->setPen(foregroundColor);

    int yColBottom = yPosFromCount(0);
 
    for ( size_t colIdx = 0; colIdx < m_histogramData.size(); ++colIdx)
    {
        size_t colCount = m_histogramData[colIdx];
        int yColTop = yPosFromCount(colCount);
        int xColStart = xPosFromColIdx(colIdx);
        int xColEnd = xPosFromColIdx(colIdx+1);

        // First line
        painter->drawLine(xColStart, yColBottom, xColStart, yColTop);
        // If we span more than one pixel with :
        ++xColStart;
        for (;xColStart < xColEnd; ++xColStart )
        {
            painter->drawLine(xColStart, yColBottom, xColStart, yColTop);
        }

    }

    // Vertical lines for percentiles
    if (m_minPercentile != HUGE_VAL)
    {
        int xpos = xPosFromDomainValue(m_minPercentile);
        painter->setPen(QColor(255, 0, 0, 200));
        painter->drawLine(xpos, y+1, xpos, y + height -1);
    }

    // Vertical lines for percentiles
    if (m_maxPercentile != HUGE_VAL)
    {
        int xpos = xPosFromDomainValue(m_maxPercentile);
        painter->setPen(QColor(255, 0, 0, 200));
        painter->drawLine(xpos, y+1, xpos, y + height -1);
    }

    // Vertical lines for percentiles
    if (m_mean != HUGE_VAL)
    {
        int xpos = xPosFromDomainValue(m_mean);
        painter->setPen(QColor(0, 0, 255, 200));
        painter->drawLine(xpos, y+1, xpos, y + height -1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSimpleHistogramWidget::setHistogramData(double min, double max, const std::vector<size_t>& histogram)
{
    m_min = min;
    m_max = max;
    m_histogramData = histogram;

    m_maxHistogramCount = 0;
    for (size_t colIdx = 0; colIdx < m_histogramData.size(); ++colIdx)
    {
        if (m_maxHistogramCount < m_histogramData[colIdx]) m_maxHistogramCount = m_histogramData[colIdx] ;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSimpleHistogramWidget::setPercentiles(double pmin, double pmax)
{
    m_minPercentile = pmin; 
    m_maxPercentile = pmax;
}

#define xBorder 1
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiuSimpleHistogramWidget::xPosFromColIdx(size_t colIdx)
{
    return  (int)(m_x + xBorder + (m_width - 2*xBorder) * colIdx/m_histogramData.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiuSimpleHistogramWidget::xPosFromDomainValue(double value)
{
    double range = m_max - m_min; 
    return  (range == 0.0) ? (int)(m_x + xBorder) : (int)(m_x + xBorder + (m_width - 2*xBorder) * (value - m_min)/(m_max - m_min));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiuSimpleHistogramWidget::yPosFromCount(size_t colHeight)
{
    return  (int)(m_y + m_height - 1 - (m_height - 3) * colHeight/m_maxHistogramCount);
}
