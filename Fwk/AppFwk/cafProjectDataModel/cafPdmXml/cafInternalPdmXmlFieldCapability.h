#pragma once

#include "cafInternalPdmXmlFieldReaderWriter.h"
#include "cafPdmXmlFieldHandle.h"

namespace caf
{

template < typename FieldType>
class PdmFieldXmlCap : public PdmXmlFieldHandle
{
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership) { m_field = field; }

    // Xml Serializing
public:
    virtual void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory);
    virtual void        writeFieldData(QXmlStreamWriter& xmlStream);
private:
    FieldType* m_field;
};


template <typename DataType> class PdmPtrField;

template < typename DataType>
class PdmFieldXmlCap< PdmPtrField<DataType*> > : public PdmXmlFieldHandle
{
    typedef PdmPtrField<DataType*> FieldType;
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership) { m_field = field; m_childClassKeyword = DataType::classKeywordStatic(); }

    // Xml Serializing
public:
    virtual void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory);
    virtual void        writeFieldData(QXmlStreamWriter& xmlStream);
private:
    FieldType* m_field;
};



template <typename DataType> class PdmChildField;

template < typename DataType>
class PdmFieldXmlCap< PdmChildField<DataType*> > : public PdmXmlFieldHandle
{
    typedef PdmChildField<DataType*> FieldType;
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership) { m_field = field; m_childClassKeyword = DataType::classKeywordStatic(); }

    // Xml Serializing
public:
    virtual void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory); 
    virtual void        writeFieldData(QXmlStreamWriter& xmlStream);
private:
    FieldType* m_field;
};


template <typename DataType> class PdmChildArrayField;

template < typename DataType>
class PdmFieldXmlCap< PdmChildArrayField<DataType*> > : public PdmXmlFieldHandle
{
    typedef PdmChildArrayField<DataType*> FieldType;
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership) { m_field = field; m_childClassKeyword = DataType::classKeywordStatic();}

    // Xml Serializing
public:
    virtual void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory);
    virtual void        writeFieldData(QXmlStreamWriter& xmlStream);
private:
    FieldType* m_field;
};




template<typename FieldType>
void AddXmlCapabilityToField(FieldType* field)
{
    if(field->template capability< PdmFieldXmlCap<FieldType> >() == NULL)
    {
        new PdmFieldXmlCap<FieldType>(field, true);
    }
}


} // End of namespace caf

#include "cafInternalPdmXmlFieldCapability.inl"
