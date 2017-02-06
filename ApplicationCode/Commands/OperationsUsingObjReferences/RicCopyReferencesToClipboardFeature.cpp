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

#include "RicCopyReferencesToClipboardFeature.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RimMimeData.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "cafPdmObject.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>


namespace caf
{

CAF_CMD_SOURCE_INIT(RicCopyReferencesToClipboardFeature, "RicCopyReferencesToClipboardFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCopyReferencesToClipboardFeature::isCommandEnabled()
{
    return isAnyCopyableObjectSelected();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCopyReferencesToClipboardFeature::onActionTriggered(bool isChecked)
{
    if (!isAnyCopyableObjectSelected()) return;

    std::vector<QString> referenceList;

    std::vector<PdmObject*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesCollObjs);

    for (PdmObject* pdmObject : selectedFormationNamesCollObjs)
    {
        if (RicCopyReferencesToClipboardFeature::isCopyOfObjectSupported(pdmObject))
        {
            QString itemRef = PdmReferenceHelper::referenceFromRootToObject(SelectionManager::instance()->pdmRootObject(), pdmObject);
            
            referenceList.push_back(itemRef);
        }
    }

    MimeDataWithReferences* myObject = new MimeDataWithReferences;
    myObject->setReferences(referenceList);

    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard)
    {
        clipboard->setMimeData(myObject);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCopyReferencesToClipboardFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Copy");
    actionToSetup->setIcon(QIcon(":/Copy.png"));
    actionToSetup->setShortcuts(QKeySequence::Copy);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCopyReferencesToClipboardFeature::isAnyCopyableObjectSelected()
{
    std::vector<PdmObject*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType(&selectedFormationNamesCollObjs);

    for (PdmObject* pdmObject : selectedFormationNamesCollObjs)
    {
        if (RicCopyReferencesToClipboardFeature::isCopyOfObjectSupported(pdmObject))
        {
            return true;
        }
    }

    return false;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCopyReferencesToClipboardFeature::isCopyOfObjectSupported(PdmObject* pdmObject)
{
    RimWellAllocationPlot* wellAllocPlot = nullptr;
    pdmObject->firstAncestorOrThisOfType(wellAllocPlot);

    if (dynamic_cast<RimGeoMechView*>(pdmObject))
    {
        return true;
    }
    else if (dynamic_cast<RimEclipseView*>(pdmObject))
    {
        return true;
    }
    else if (dynamic_cast<RimEclipseCase*>(pdmObject))
    {
        return true;
    }
    else if (dynamic_cast<RimSummaryPlot*>(pdmObject))
    {
        return true;
    }
    else if (dynamic_cast<RimSummaryCurve*>(pdmObject))
    {
        return true;
    }
    else if (dynamic_cast<RimSummaryCurveFilter*>(pdmObject))
    {
        return true;
    }
    else if (dynamic_cast<RimWellLogCurve*>(pdmObject))
    {
        if (!wellAllocPlot) return true;
    }
    else if (dynamic_cast<RimWellLogTrack*>(pdmObject))
    {
        if (!wellAllocPlot) return true;
    }
    else if (dynamic_cast<RimWellLogPlot*>(pdmObject))
    {
        if (!wellAllocPlot) return true;
    }

    return false;
}

} // end namespace caf
