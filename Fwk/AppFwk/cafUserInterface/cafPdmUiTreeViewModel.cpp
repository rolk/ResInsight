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


#include "cafPdmUiTreeViewModel.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiTreeItemEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewEditor.h"

#include <QTreeView>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewModel::PdmUiTreeViewModel(PdmUiTreeViewEditor* treeViewEditor)
{
    m_treeOrderingRoot = NULL;
    m_treeViewEditor = treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
/// Will populate the tree with the contents of the Pdm data structure rooted at rootItem.
/// Will not show the rootItem itself, only the children and downwards 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::setPdmItemRoot(PdmUiItem* rootItem)
{
    // Check if we are already watching this root
    if (m_treeOrderingRoot && m_treeOrderingRoot->activeItem() == rootItem)
    {
        this->updateSubTree(rootItem);
        return;
    }

    PdmUiTreeOrdering* newRoot = NULL;
    PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*> (rootItem);

    if (field)
    {
        newRoot = new PdmUiTreeOrdering(field->fieldHandle());
        PdmUiObjectHandle::expandUiTree(newRoot, m_uiConfigName);
    }
    else
    {
        PdmUiObjectHandle * obj = dynamic_cast<PdmUiObjectHandle*> (rootItem);
        if (obj)
        {
            newRoot = obj->uiTreeOrdering(m_uiConfigName);
        }
    }

    assert( newRoot || rootItem == NULL ); // Only fields, objects or NULL is allowed.

    if (newRoot) newRoot->debugDump(0);

    this->resetTree(newRoot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::resetTree(PdmUiTreeOrdering* newRoot)
{
    beginResetModel();
    
    if (m_treeOrderingRoot)
    {
        delete m_treeOrderingRoot;
    }

    m_treeOrderingRoot = newRoot;

    updateEditorsForSubTree(m_treeOrderingRoot);

    endResetModel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::setColumnHeaders(const QStringList& columnHeaders)
{
    m_columnHeaders = columnHeaders;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::emitDataChanged(const QModelIndex& index)
{
    emit dataChanged(index, index);
}

//--------------------------------------------------------------------------------------------------
/// Refreshes the UI-tree below the supplied root PdmUiItem
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::updateSubTree(PdmUiItem* pdmRoot)
{
    // Build the new "Correct" Tree

    PdmUiTreeOrdering* newTreeRootTmp = NULL;
    PdmUiFieldHandle* field = dynamic_cast<PdmUiFieldHandle*> (pdmRoot);
    if (field)
    {
        newTreeRootTmp = new PdmUiTreeOrdering(field->fieldHandle());
    }
    else
    {
        PdmUiObjectHandle* obj = dynamic_cast<PdmUiObjectHandle*> (pdmRoot);
        if (obj)
        {
            newTreeRootTmp = new PdmUiTreeOrdering(obj->owner());
        }
    }

    PdmUiObjectHandle::expandUiTree(newTreeRootTmp, m_uiConfigName);

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "New Stuff: " << std::endl ;
    newTreeRootTmp->debugDump(0);
#endif

    // Find the corresponding entry for "root" in the existing Ui tree

    QModelIndex existingSubTreeRootModIdx = findModelIndex(pdmRoot);

    PdmUiTreeOrdering* existingSubTreeRoot = NULL;
    if (existingSubTreeRootModIdx.isValid())
    {
        existingSubTreeRoot = treeItemFromIndex(existingSubTreeRootModIdx);
    }
    else
    {
        existingSubTreeRoot = m_treeOrderingRoot;
    }

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "Old :"<< std::endl ;
    existingSubTreeRoot->debugDump(0);
#endif

    updateSubTreeRecursive(existingSubTreeRootModIdx, existingSubTreeRoot, newTreeRootTmp);

    delete newTreeRootTmp;

    updateEditorsForSubTree(existingSubTreeRoot);

	// Notify Qt that the toggle/name/icon etc might have been changed
	emitDataChanged(existingSubTreeRootModIdx);

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "Result :"<< std::endl ;
    existingSubTreeRoot->debugDump(0);
#endif	
}

//--------------------------------------------------------------------------------------------------
/// Makes the destinationSubTreeRoot tree become identical to the tree in sourceSubTreeRoot, 
/// calling begin..() end..() to make the UI update accordingly.
/// This assumes that all the items have a pointer an unique PdmObject 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::updateSubTreeRecursive(const QModelIndex& existingSubTreeRootModIdx, PdmUiTreeOrdering* existingSubTreeRoot, PdmUiTreeOrdering* sourceSubTreeRoot)
{
    // First loop over children in the old ui tree, deleting the ones not present in 
    // the newUiTree

    for (int cIdx = 0; cIdx < existingSubTreeRoot->childCount() ; ++cIdx)
    {
        PdmUiTreeOrdering* oldChild = existingSubTreeRoot->child(cIdx);

        int childIndex = findChildItemIndex(sourceSubTreeRoot, oldChild->activeItem());

        if (childIndex == -1) // Not found
        {
            this->beginRemoveRows(existingSubTreeRootModIdx, cIdx, cIdx);
            existingSubTreeRoot->removeChildren(cIdx, 1);
            this->endRemoveRows();
            cIdx--;
        }
    }

    // Then loop over the children in the new ui tree, finding the corresponding items in the old tree. 
    // If they are found, we move them to the correct position. 
    // If not found, we pull the item out of the old ui tree, inserting it 
    // into the new tree.

    int sourceChildCount = sourceSubTreeRoot->childCount();
    int source_cIdx = 0;

    for (int cIdx = 0; cIdx < sourceChildCount; ++cIdx, ++source_cIdx)
    {
        PdmUiTreeOrdering* newChild = sourceSubTreeRoot->child(source_cIdx);

        int existing_cIdx = findChildItemIndex(existingSubTreeRoot, newChild->activeItem());

        if (existing_cIdx == -1) // Not found, move from source to existing
        {
            this->beginInsertRows(existingSubTreeRootModIdx, cIdx, cIdx);
            existingSubTreeRoot->insertChild(cIdx, newChild);
            this->endInsertRows();

            sourceSubTreeRoot->removeChildrenNoDelete(source_cIdx, 1);
            source_cIdx--;
        }
        else if (existing_cIdx != cIdx) // Found, but must be moved
        {
            assert(existing_cIdx > cIdx);

            PdmUiTreeOrdering* oldChild = existingSubTreeRoot->child(existing_cIdx);

            this->beginMoveRows(existingSubTreeRootModIdx, existing_cIdx, existing_cIdx, existingSubTreeRootModIdx, cIdx);
            existingSubTreeRoot->removeChildrenNoDelete(existing_cIdx, 1);
            existingSubTreeRoot->insertChild(cIdx, oldChild);
            this->endMoveRows();

            updateSubTreeRecursive( index(cIdx, 0, existingSubTreeRootModIdx), oldChild, newChild);
        }
        else // Found the corresponding item in the right place.
        {
            PdmUiTreeOrdering* oldChild = existingSubTreeRoot->child(existing_cIdx);

            updateSubTreeRecursive( index(cIdx, 0, existingSubTreeRootModIdx), oldChild, newChild);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns the index of the first child with activeItem() == pdmItemToFindInChildren. 
//  -1 if not found or pdmItemToFindInChildren == NULL
//--------------------------------------------------------------------------------------------------
int PdmUiTreeViewModel::findChildItemIndex(const PdmUiTreeOrdering * parent, const PdmUiItem* pdmItemToFindInChildren)
{
    if (pdmItemToFindInChildren == NULL) return -1;

    for (int i = 0; i < parent->childCount(); ++i)
    {
        PdmUiTreeOrdering* child = parent->child(i);
        if (child->activeItem() == pdmItemToFindInChildren)
        {
            return i;
        }
        else if (child->isDisplayItemOnly())
        {
            // Todo, possibly. Find a way to check for equality based on content
            // until done, the display items will always be regenerated with all their children
        }

    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewModel::updateEditorsForSubTree(PdmUiTreeOrdering* root)
{
    if (!root) return;

    if (!root->editor())
    {
        PdmUiTreeItemEditor* treeItemEditor = new PdmUiTreeItemEditor(root->activeItem());
        root->setEditor(treeItemEditor);
        assert(root->editor());
    }

    PdmUiTreeItemEditor* treeItemEditor = dynamic_cast<PdmUiTreeItemEditor*>(root->editor());
    if (treeItemEditor)
    {
        treeItemEditor->setTreeViewEditor(m_treeViewEditor);
    }

    for (int i = 0; i < root->childCount(); ++i)
    {
        updateEditorsForSubTree(root->child(i));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* caf::PdmUiTreeViewModel::treeItemFromIndex(const QModelIndex& index) const 
{
    if (!index.isValid())
    {
        return m_treeOrderingRoot;
    }
    
    assert(index.internalPointer());

    PdmUiTreeOrdering* treeItem = static_cast<PdmUiTreeOrdering*>(index.internalPointer());

    return treeItem;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeViewModel::findModelIndex( const PdmUiItem * object) const
{
    QModelIndex foundIndex;
    int numRows = rowCount(QModelIndex());
    int r = 0;
    while (r < numRows && !foundIndex.isValid())
    {
        foundIndex = findModelIndexRecursive(index(r, 0, QModelIndex()), object);
        ++r;
    }
    return foundIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex caf::PdmUiTreeViewModel::findModelIndexRecursive(const QModelIndex& currentIndex, const PdmUiItem * pdmItem) const
{
    if (currentIndex.internalPointer())
    {
        PdmUiTreeOrdering* treeItem = static_cast<PdmUiTreeOrdering*>(currentIndex.internalPointer());
        if (treeItem->activeItem() == pdmItem) return currentIndex;
    }

   int row;
   for (row = 0; row < rowCount(currentIndex); ++row)
   {
       QModelIndex foundIndex = findModelIndexRecursive(index(row, 0, currentIndex), pdmItem);
       if (foundIndex.isValid()) return foundIndex;
   }
   return QModelIndex();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void caf::PdmUiTreeViewModel::selectedUiItems(std::vector<PdmUiItem*>& objects)
{
    if (!(m_treeViewEditor && m_treeViewEditor->treeView())) return;

    QModelIndexList idxList = m_treeViewEditor->treeView()->selectionModel()->selectedIndexes();

    for (int i = 0; i < idxList.size(); i++)
    {
        PdmUiTreeOrdering* treeItem = this->treeItemFromIndex(idxList[i]);
        if (treeItem)
        {
            caf::PdmUiItem* item = treeItem->activeItem();
            objects.push_back(item);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// An invalid parent index is implicitly meaning the root item, and not "above" root, since
/// we are not showing the root item itself
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewModel::index(int row, int column, const QModelIndex &parentIndex ) const
{
    if (!m_treeOrderingRoot)
        return QModelIndex();

    PdmUiTreeOrdering* parentItem = NULL;

    if (!parentIndex.isValid())
        parentItem = m_treeOrderingRoot;
    else
        parentItem = static_cast<PdmUiTreeOrdering*>(parentIndex.internalPointer());

    assert(parentItem);

    if (parentItem->childCount() <= row)
    {
        return QModelIndex();
    }

    PdmUiTreeOrdering* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewModel::parent(const QModelIndex &childIndex) const
{
    if (!childIndex.isValid()) return QModelIndex();

    PdmUiTreeOrdering* childItem = static_cast<PdmUiTreeOrdering*>(childIndex.internalPointer());
    if (!childItem) return QModelIndex();

    PdmUiTreeOrdering* parentItem = childItem->parent();
    if (!parentItem) return QModelIndex();

    if (parentItem == m_treeOrderingRoot) return QModelIndex();

    return createIndex(parentItem->indexInParent(), 0, parentItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTreeViewModel::rowCount(const QModelIndex &parentIndex ) const
{
    if (!m_treeOrderingRoot)
        return 0;

    if (parentIndex.column() > 0)
        return 0;

    PdmUiTreeOrdering* parentItem = this->treeItemFromIndex(parentIndex);

    return parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmUiTreeViewModel::columnCount(const QModelIndex &parentIndex ) const
{
    if (!m_treeOrderingRoot)
        return 0;

    return 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTreeViewModel::data(const QModelIndex &index, int role ) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    PdmUiTreeOrdering* uitreeOrdering = static_cast<PdmUiTreeOrdering*>(index.internalPointer());
    if (!uitreeOrdering)
    {
        return QVariant();
    }

    bool isObjRep = uitreeOrdering->isRepresentingObject();
    bool isFieldRep = uitreeOrdering->isRepresentingField();
    bool isDisplayOnly = uitreeOrdering->isDisplayItemOnly();

	// MODTODO
    //assert (uitreeOrdering->isValid()); // Tree generation has some error.

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (isObjRep)
        {
            PdmUiObjectHandle* pdmUiObject = uiObj(uitreeOrdering->object());
            if (pdmUiObject)
            {
                if (pdmUiObject->userDescriptionField())
                {
                    caf::PdmUiFieldHandle* uiFieldHandle = uiField(pdmUiObject->userDescriptionField());
                    if (uiFieldHandle)
                    {
                        return uiFieldHandle->uiValue();
                    }
                }

                return pdmUiObject->uiName();
            }
            else
            {
                return QVariant();
            }
        }
       
        if (uitreeOrdering->activeItem())
        {
        return uitreeOrdering->activeItem()->uiName();
    }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::DecorationRole)
    {
        if (uitreeOrdering->activeItem())
        {
        return uitreeOrdering->activeItem()->uiIcon();
    }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::ToolTipRole)
    {
        if (uitreeOrdering->activeItem())
        {
         return uitreeOrdering->activeItem()->uiToolTip();
    }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::WhatsThisRole)
    {
        if (uitreeOrdering->activeItem())
        {
        return uitreeOrdering->activeItem()->uiWhatsThis();
    }
        else
        {
            return QVariant();
        }
    }
    else if (role == Qt::CheckStateRole)
    {
        if (isObjRep)
        {
            PdmUiObjectHandle* pdmUiObj = uiObj(uitreeOrdering->object());
            if (pdmUiObj && pdmUiObj->objectToggleField())
            {
                caf::PdmUiFieldHandle* uiFieldHandle = uiField(pdmUiObj->objectToggleField());
                if (uiFieldHandle)
            {
                    bool isToggledOn = uiFieldHandle->uiValue().toBool();
                if (isToggledOn)
                {
                    return Qt::Checked;
                }
                else
                {
                    return Qt::Unchecked;
                }
            }
                else
                {
                    return QVariant();
                }
        }
    }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (!index.isValid())
    {
        return false;
    }

    PdmUiTreeOrdering* treeItem = PdmUiTreeViewModel::treeItemFromIndex(index);
    assert(treeItem);

    if (!treeItem->isRepresentingObject()) return false;

    PdmUiObjectHandle* uiObject = uiObj(treeItem->object());
    if (uiObject)
    {
        if (role == Qt::EditRole && uiObject->userDescriptionField())
        {
            PdmUiFieldHandle* userDescriptionUiField = uiField(uiObject->userDescriptionField());
            if (userDescriptionUiField)
            {
                userDescriptionUiField->setValueFromUi(value);
            }

            return true;
        }
        else if (role == Qt::CheckStateRole && uiObject->objectToggleField())
        {
            bool toggleOn = (value == Qt::Checked);

            PdmUiFieldHandle* toggleUiField = uiField(uiObject->objectToggleField());
            if (toggleUiField)
            {
                toggleUiField->setValueFromUi(value);
            }

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Enable edit of this item if we have a editable user description field for a pdmObject
/// Disable edit for other items
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags PdmUiTreeViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags flagMask = QAbstractItemModel::flags(index);

    PdmUiTreeOrdering* treeItem = treeItemFromIndex(index);
    assert(treeItem);

    if (treeItem->isRepresentingObject())
    {
        PdmUiObjectHandle* pdmUiObject = uiObj(treeItem->object());
        if (pdmUiObject)
        {
            if (pdmUiObject->userDescriptionField() && !uiField(pdmUiObject->userDescriptionField())->isUiReadOnly())
        {
            flagMask = flagMask | Qt::ItemIsEditable;
        }

            if (pdmUiObject->objectToggleField())
        {
            flagMask = flagMask | Qt::ItemIsUserCheckable;
        }
    }
    }

    if (treeItem->isValid())
    {
        if (treeItem->activeItem()->isUiReadOnly())
        {
            flagMask = flagMask & (~Qt::ItemIsEnabled);
        }
    }

    return flagMask;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant PdmUiTreeViewModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (section < m_columnHeaders.size())
    {
        return m_columnHeaders[section];
    }

    return QVariant();
}


} // end namespace caf
