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

#include "RicEclipsePropertyFilterNewExec.h"

#include "RicEclipsePropertyFilter.h"

#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterNewExec::RicEclipsePropertyFilterNewExec(RimEclipsePropertyFilterCollection* propertyFilterCollection)
    : CmdExecuteCommand(NULL)
{
    assert(propertyFilterCollection);
    m_propertyFilterCollection = propertyFilterCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipsePropertyFilterNewExec::~RicEclipsePropertyFilterNewExec()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicEclipsePropertyFilterNewExec::name()
{
    return "New Property Filter";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewExec::redo()
{ 
    RicEclipsePropertyFilter::addPropertyFilter(m_propertyFilterCollection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewExec::undo()
{
    m_propertyFilterCollection->propertyFilters.erase(m_propertyFilterCollection->propertyFilters.size() - 1);
    caf::PdmUiFieldHandle::updateConnectedUiEditors(m_propertyFilterCollection->parentField());
}
