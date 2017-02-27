/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiaLogging.h"

#include <QPointer>
#include <QWidget>

class QDockWidget;
class QPlainTextEdit;


//==================================================================================================
//
// 
//
//==================================================================================================
class RiuMessagePanel : public QWidget
{
    Q_OBJECT

public:
    RiuMessagePanel(QDockWidget* parent);

    void            addMessage(RILogLevel messageLevel, const QString& msg);
    virtual QSize	sizeHint () const;

private slots:
    void            slotShowContextMenu(const QPoint& pos);
    void            slotClearMessages();

private:
    QPointer<QPlainTextEdit> m_textEdit;
};



//==================================================================================================
//
// 
//
//==================================================================================================
class RiuMessagePanelLogger : public RiaLogger
{
public:
    RiuMessagePanelLogger(RiuMessagePanel* messagePanel);

    virtual int     level() const override;
    virtual void    setLevel(int logLevel) override;

    virtual void    error(  const char* message) override;
    virtual void    warning(const char* message) override;
    virtual void    info(   const char* message) override;
    virtual void    debug(  const char* message) override;

private:
    void            writeToMessagePanel(RILogLevel messageLevel, const char* message);

private:
    QPointer<RiuMessagePanel>   m_messagePanel;
    int                         m_logLevel; 
};

