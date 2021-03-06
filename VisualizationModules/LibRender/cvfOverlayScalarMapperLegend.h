//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cvfOverlayItem.h"
#include "cvfArray.h"
#include "cvfCamera.h"
#include "cvfString.h"
#include "cvfRect.h"

namespace cvf {

class Font;
class ShaderProgram;
class MatrixState;
class TextDrawer;
class ScalarMapper;


//==================================================================================================
//
// Overlay color legend
//
//==================================================================================================
class OverlayScalarMapperLegend : public OverlayItem
{
public:
    OverlayScalarMapperLegend(Font* font);
    virtual ~OverlayScalarMapperLegend();

    virtual Vec2ui  sizeHint();
    virtual Vec2ui  maximumSize();
    virtual Vec2ui  minimumSize();
    void            setScalarMapper(const ScalarMapper* scalarMapper);

    virtual void    render(OpenGLContext* oglContext, const Vec2ui& position, const Vec2ui& size);
    virtual void    renderSoftware(OpenGLContext* oglContext, const Vec2ui& position, const Vec2ui& size);
    virtual bool    pick(uint oglXCoord, uint oglYCoord, const Vec2ui& position, const Vec2ui& size);


    void            setSizeHint(const Vec2ui& size);
    
    void            setColor(const Color3f& color);
    const Color3f&  color() const;

    void            setLineColor(const Color3f& lineColor);
    const Color3f&  lineColor() const;
    void            setLineWidth(int lineWidth);
    int             lineWidth() const;
    
    void            setTitle(const String& title);
    String          title() const;

protected:

    //==================================================================================================
    //
    // Helper for storing layout info
    //
    //==================================================================================================
    struct OverlayColorLegendLayoutInfo
    {
        OverlayColorLegendLayoutInfo(const Vec2ui& pos, const Vec2ui& setSize)
        {
            position = pos;
            size = setSize;
        }

        float charHeight;
        float lineSpacing;
        Vec2f margins;
        float tickX;
        float x0, x1;

        Rectf legendRect;

        ref<DoubleArray> tickPixelPos;

        Vec2ui position;
        Vec2ui size;
    };


    void         render(OpenGLContext* oglContext, const Vec2ui& position, const Vec2ui& size, bool software);
    virtual void renderLegend(OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout, const MatrixState& matrixState);
    virtual void renderLegendImmediateMode(OpenGLContext* oglContext, OverlayColorLegendLayoutInfo* layout);
    virtual void setupTextDrawer(TextDrawer* textDrawer, OverlayColorLegendLayoutInfo* layout);

    void layoutInfo(OverlayColorLegendLayoutInfo* layout);

protected:
    DoubleArray         m_tickValues;           // Ticks between each level + top and bottom of legend (n+1 entries)
    std::vector<bool>   m_visibleTickLabels;    // Skip tick labels ending up on top of previous visible label

    Vec2ui              m_sizeHint;     // Pixel size of the color legend area
    
    Color3f             m_color;
    Color3f             m_lineColor;
    int                 m_lineWidth;
    std::vector<String> m_titleStrings;
    ref<Font>           m_font;

    cref<ScalarMapper> m_scalarMapper;
};

}
