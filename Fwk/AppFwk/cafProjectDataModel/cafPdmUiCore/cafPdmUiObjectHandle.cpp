#include "cafPdmUiObjectHandle.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include <assert.h>


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiObjectHandle::PdmUiObjectHandle(PdmObjectHandle* owner, bool giveOwnership)
{
    m_owner = owner;
    m_owner->addCapability(this, giveOwnership);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiObjectHandle* uiObj(PdmObjectHandle* obj)
{
    if (!obj) return NULL;
    PdmUiObjectHandle* uiObject = obj->capability<PdmUiObjectHandle>();
    assert(uiObject);
    return uiObject;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::uiOrdering(QString uiConfigName, PdmUiOrdering& uiOrdering)
{
#if 1
    this->defineUiOrdering(uiConfigName, uiOrdering);
    if (!uiOrdering.forgetRemainingFields())
    {
        // Add the remaining Fields To UiConfig
        std::vector<PdmFieldHandle*> fields;
        m_owner->fields(fields);
        for (size_t i = 0; i < fields.size(); ++i)
        {
            PdmUiFieldHandle * field = uiField(fields[i]);
            if (!uiOrdering.contains(field))
            {
                uiOrdering.add(field->fieldHandle());
            }
        }
    }
#endif
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::editorAttribute(const PdmFieldHandle* field, QString uiConfigName, PdmUiEditorAttribute * attribute)
{
    this->defineEditorAttribute(field, uiConfigName, attribute);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::objectEditorAttribute(QString uiConfigName, PdmUiEditorAttribute* attribute)
{
    this->defineObjectEditorAttribute(uiConfigName, attribute);
}

//--------------------------------------------------------------------------------------------------
/// This method creates a tree-representation of the object hierarchy starting at this
/// object to be used for a tree view. 
/// This method calls the optional virtual user defined method "defineUiTreeOrdering" to customize the 
/// order and content of the children directly below each object. If this method does nothing, 
/// the default behavior applies: Add all fields that contains objects, and their objects.
///
/// The caller is responsible to delete the returned PdmUiTreeOrdering
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* PdmUiObjectHandle::uiTreeOrdering(QString uiConfigName /*= ""*/)
{
    assert(this); // This method actually is possible to call on a NULL ptr without getting a crash, so we assert instead.

    PdmUiTreeOrdering* uiTreeOrdering = new PdmUiTreeOrdering(NULL, m_owner);

    expandUiTree(uiTreeOrdering, uiConfigName);

    return uiTreeOrdering;
}

//--------------------------------------------------------------------------------------------------
/// Adds the direct children of this PdmObject to the PdmUiTree according to
/// the default rules. Add all fields that contains objects, and their objects.
/// Takes into account the control variables regarding this:
/// PdmField::isUiHidden
/// PdmField::isUiChildrenHidden
/// And whether the fields and objects are already added by the user. 
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::addDefaultUiTreeChildren(PdmUiTreeOrdering* uiTreeOrdering)
{
#if 1
    if (!uiTreeOrdering->forgetRemainingFields())
    {
        // Add the remaining Fields To UiConfig
        std::vector<PdmFieldHandle*> fields;
        m_owner->fields(fields);

        for (size_t fIdx = 0; fIdx < fields.size(); ++fIdx)
        {
            if (fields[fIdx]->hasChildObjects() && !uiTreeOrdering->containsField(fields[fIdx]))
            {
                if (uiField(fields[fIdx])->isUiHidden() && !uiField(fields[fIdx])->isUiChildrenHidden())
                {
                    std::vector<PdmObjectHandle*> children;
                    fields[fIdx]->childObjects(&children);

                    for (size_t cIdx = 0; cIdx < children.size(); cIdx++)
                    {
                        if (!uiTreeOrdering->containsObject(children[cIdx]))
                        {
                            uiTreeOrdering->add(children[cIdx]);
                        }
                    }
                }
                else if (!uiField(fields[fIdx])->isUiHidden())
                {
                    uiTreeOrdering->add(fields[fIdx]);
                }
            }
        }
    }
#endif
}


//--------------------------------------------------------------------------------------------------
/// Builds the sPdmUiTree for all the children of @param root recursively, and stores the result
/// in root
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::expandUiTree(PdmUiTreeOrdering* root, QString uiConfigName /*= "" */)
{
#if 1
    if (!root || !root->isValid()) return;

    if (root->childCount() > 0)
    {
        for (int cIdx = 0; cIdx < root->childCount(); ++cIdx)
        {
            PdmUiTreeOrdering* child = root->child(cIdx);
            if (child->isValid() && !child->ignoreSubTree())
            {
                expandUiTree(child);
            }
        }
    }
    else //( root->childCount() == 0) // This means that no one has tried to expand it.
    {
        if (!root->ignoreSubTree())
        {
            if (root->isRepresentingField() && !uiField(root->field())->isUiChildrenHidden(uiConfigName))
            {
                std::vector<PdmObjectHandle*> fieldsChildObjects;
                root->field()->childObjects(&fieldsChildObjects);
                for (size_t cIdx = 0; cIdx < fieldsChildObjects.size(); ++cIdx)
                {
                    PdmObjectHandle* childObject = fieldsChildObjects[cIdx];
                    if (childObject)
                    {
                        root->appendChild(uiObj(childObject)->uiTreeOrdering(uiConfigName));
                    }
                }
            }
            else if (root->isRepresentingObject())
            {
                uiObj(root->object())->defineUiTreeOrdering(*root, uiConfigName);
                uiObj(root->object())->addDefaultUiTreeChildren(root);
                if (root->childCount())
                {
                    expandUiTree(root);
                }
            }
        }
    }
#endif
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiObjectHandle::updateUiIconFromToggleField()
{
    if (objectToggleField())
    {
        PdmUiFieldHandle* uiFieldHandle = uiField(objectToggleField());
        if (uiFieldHandle)
        {
            bool active = uiFieldHandle->uiValue().toBool();
            updateUiIconFromState(active);
        }
    }
}

} // End namespace caf
