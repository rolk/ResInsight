
#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfScalarMapper.h"

namespace caf {


//==================================================================================================
//
//
//==================================================================================================
class CategoryMapper : public cvf::ScalarMapper
{
public:
    CategoryMapper();

    void                setCategories(const cvf::IntArray& categories);
    void                setColors(const cvf::Color3ubArray& colorArray);

    cvf::IntArray       categories() const;

    virtual cvf::Vec2f  mapToTextureCoord(double scalarValue) const;
    virtual bool        updateTexture(cvf::TextureImage* image) const;

    virtual cvf::Color3ub   mapToColor(double normalizedValue) const;

    virtual void        majorTickValues(std::vector<double>* domainValues) const;
    virtual double      normalizedValue(double domainValue) const;
    virtual double      domainValue(double normalizedValue) const;

private:
    int                 categoryIndexForCategory(double domainValue) const;

private:
    cvf::Color3ubArray   m_colors;
    cvf::uint            m_textureSize;      // The size of texture that updateTexture() is will produce. 

    cvf::IntArray        m_categories;
};

}