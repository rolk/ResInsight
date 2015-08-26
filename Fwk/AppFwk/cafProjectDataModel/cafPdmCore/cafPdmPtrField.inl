namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmPtrField<DataType*>::PdmPtrField(const DataTypePtr& fieldValue)
{
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addReferencingPtrField(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmPtrField<DataType*>::~PdmPtrField()
{
    if (!m_fieldValue.isNull()) m_fieldValue.rawPtr()->removeReferencingPtrField(this);
    m_fieldValue.setRawPtr(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void PdmPtrField<DataType*>::setValue(const DataTypePtr& fieldValue)
{
    if (m_fieldValue) m_fieldValue->removeReferencingPtrField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addReferencingPtrField(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void PdmPtrField<DataType*>::setRawPtr(PdmObjectHandle* obj)
{
    if (m_fieldValue.notNull()) m_fieldValue.rawPtr()->removeReferencingPtrField(this);
    m_fieldValue.setRawPtr(obj);
    if (m_fieldValue.notNull()) m_fieldValue.rawPtr()->addReferencingPtrField(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmPtrField<DataType*>& PdmPtrField<DataType*>::operator=(const DataTypePtr & fieldValue)
{
    if (m_fieldValue) m_fieldValue->removeReferencingPtrField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addReferencingPtrField(this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmPtrField<DataType*>& PdmPtrField<DataType*>::operator=(const FieldDataType & fieldValue)
{
    if (m_fieldValue) m_fieldValue->removeReferencingPtrField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addReferencingPtrField(this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmPtrField<DataType*>::childObjects(std::vector<PdmObjectHandle*>* objects)
{
	assert(objects);
	PdmObjectHandle* obj = m_fieldValue.rawPtr();
	if (obj)
	{
		objects->push_back(obj);
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void PdmPtrField<DataType*>::ptrReferencedObjects(std::vector<PdmObjectHandle*>* objectsToFill)
{
    if (m_fieldValue.rawPtr())
    {
        objectsToFill->push_back(m_fieldValue.rawPtr());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void PdmPtrField<DataType*>::resolveReferences()
{
    if (m_isResolved) return;
    if (m_referenceString.isEmpty()) return;

    PdmObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference(this, m_referenceString);
    this->setRawPtr(objHandle);
    m_isResolved = true;
}

} // End of namespace caf

