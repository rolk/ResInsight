//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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


#include "cafPdmUiToolBarEditor.h"

#include "cafPdmField.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"

#include <QToolBar>
#include <QMainWindow>
#include <QAction>

#include <assert.h>
#include "cafPdmUiOrdering.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiFieldHandle.h"


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiToolBarEditor::PdmUiToolBarEditor(const QString& title, QMainWindow* mainWindow)
{
    m_toolbar = new QToolBar(mainWindow);
    m_toolbar->setObjectName(title);
    m_toolbar->setWindowTitle(title);

    mainWindow->addToolBar(m_toolbar);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiToolBarEditor::~PdmUiToolBarEditor()
{
    clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    for (size_t fIdx = 0; fIdx < m_fields.size(); fIdx++)
    {
        PdmFieldHandle* field = m_fields[fIdx];
        PdmUiFieldEditorHandle* fieldEditor = NULL;

        PdmUiOrdering config;
        
        caf::PdmUiObjectHandle* ownerUiObject = uiObj(field->ownerObject());
        if (ownerUiObject)
        {
            ownerUiObject->uiOrdering(uiConfigName, config);
        }

        // Find or create FieldEditor
        std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
        it = m_fieldViews.find(field->keyword());

        if (it == m_fieldViews.end())
        {
            /*

            //Code used to support other editor types than bool
            //Not tested

            // If editor type is specified, find in factory
            if (!field->uiEditorTypeName(uiConfigName).isEmpty())
            {
                fieldEditor = caf::Factory<PdmUiFieldEditorHandle, QString>::instance()->create(field->uiEditorTypeName(uiConfigName));
            }
            else
            {
                // Find the default field editor

                QString editorTypeName = qStringTypeName(*field);

                // Handle a single value field with valueOptions: Make a combobox

                QVariant::Type qtType = field->uiValue().type();

                if (field->uiValue().type() != QVariant::List)
                {
                    bool useOptionsOnly = true;
                    QList<PdmOptionItemInfo> options = field->valueOptions(&useOptionsOnly);

                    if (!options.empty())
                    {
                        editorTypeName = caf::PdmUiComboBoxEditor::uiEditorTypeName();
                    }
                }
                
                if (field->uiValue().type() == QVariant::Bool)
                {
                    // Special handling of bool values into tool button editors
                    
                    editorTypeName = caf::PdmUiToolButtonEditor::uiEditorTypeName();
                }

                fieldEditor = caf::Factory<PdmUiFieldEditorHandle, QString>::instance()->create(editorTypeName);
            }
            */

			caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();

            QString editorTypeName;
            if (uiFieldHandle && uiFieldHandle->uiValue().type() == QVariant::Bool)
            {
                // Special handling of bool values into tool button editors

                editorTypeName = caf::PdmUiToolButtonEditor::uiEditorTypeName();
            }

            fieldEditor = caf::Factory<PdmUiFieldEditorHandle, QString>::instance()->create(editorTypeName);

            if (fieldEditor)
            {
                m_fieldViews[field->keyword()] = fieldEditor;
                fieldEditor->createWidgets(NULL);
                m_actions.push_back(m_toolbar->addWidget(fieldEditor->editorWidget()));

                fieldEditor->setField(uiFieldHandle);
                fieldEditor->updateUi(uiConfigName);
            }

        }
    }

    assert(m_fields.size() == m_fieldViews.size());
    assert(m_fields.size() == m_actions.size());

    for (int i = 0; i < m_fields.size(); i++)
    {
        caf::PdmFieldHandle* field = m_fields[i];

        // Enabled state of a tool button is controlled by the QAction associated with a tool button
        // Changing the state of a widget directly has no effect
        // See Qt doc for QToolBar::insertWidget 
        QAction* action = m_actions[i];

		caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
        if (uiFieldHandle)
        {
            action->setEnabled(!uiFieldHandle->isUiReadOnly(uiConfigName));
        }

        // TODO: Show/hide of tool bar items can be done by
        // action->setVisible(!field->isUiHidden(uiConfigName));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::setFields(std::vector<caf::PdmFieldHandle*>& fields)
{
    clear();

    for (size_t i = 0; i < fields.size(); i++)
    {
		caf::PdmUiFieldHandle* uiFieldHandle = fields[i]->uiCapability();

        // Supports only bool fields
        assert(uiFieldHandle->uiValue().type() == QVariant::Bool);
    }

    m_fields = fields;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiToolBarEditor::clear()
{
    std::map<QString, PdmUiFieldEditorHandle*>::iterator it;
    for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
    {
        delete it->second;
    }

    m_fieldViews.clear();

    if (m_toolbar)
    {
        m_toolbar->clear();
    }

    m_actions.clear();
}


} // end namespace caf
