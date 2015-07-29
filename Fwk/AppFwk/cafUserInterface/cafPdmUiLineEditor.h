//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cafPdmUiFieldEditorHandle.h"

#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QString>
#include <QWidget>

class QGridLayout;

namespace caf 
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiLineEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiLineEditorAttribute()
    {
        useRangeValidator = false;
        minValue = 0;
        maxValue = 0;
    }

public:
    bool useRangeValidator;
    int minValue;
    int maxValue;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiLineEditorAttributeUniqueValues : public PdmUiEditorAttribute
{
public:
    PdmUiLineEditorAttributeUniqueValues()
    {}

public:
    std::set<int> usedIds;
    QString errorMessage;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiLineEditorAttributeUiDisplayString : public PdmUiEditorAttribute
{
public:
    PdmUiLineEditorAttributeUiDisplayString()
    {}

public:
    QString m_displayString;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiLineEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiLineEditor()          {} 
    virtual ~PdmUiLineEditor() {} 

protected:
    virtual QWidget*    createEditorWidget(QWidget * parent);
    virtual QWidget*    createLabelWidget(QWidget * parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);

protected slots:
    void                slotEditingFinished();

private:
    bool                isMultipleFieldsWithSameKeywordSelected(PdmFieldHandle* editorField) const;

private:
    QPointer<QLineEdit> m_lineEdit;
    QPointer<QLabel>    m_label;

};


} // end namespace caf
