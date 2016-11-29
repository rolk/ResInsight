// Based on the example scalepicker from the Qwt/examples/event_filter

#pragma once

#include <QObject>
#include <QRect>

class QwtPlot;
class QwtScaleWidget;

class RiuQwtScalePicker : public QObject
{
    Q_OBJECT
public:
    RiuQwtScalePicker(QwtPlot *plot);
    
    virtual bool eventFilter( QObject *, QEvent * );

Q_SIGNALS:
    void clicked( int axis, double value );

private:
    void mouseClicked( const QwtScaleWidget *, const QPoint & );
};
