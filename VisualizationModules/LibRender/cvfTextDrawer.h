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

#include "cvfObject.h"
#include "cvfString.h"
#include "cvfVector3.h"
#include "cvfColor3.h"


namespace cvf {

class Font;
class ShaderProgram;
class MatrixState;
class Viewport;
class OpenGLContext;


//==================================================================================================
//
// Draw text in currently set projection
//
//==================================================================================================
class TextDrawer : public Object
{
public:
    enum Alignment
    {
        TOP,
        CENTER,
        BASELINE,
        BOTTOM
    };

public:
    TextDrawer(Font* font);
    virtual ~TextDrawer();

    void    addText(const String& text, const Vec2f& pos);
    void    removeAllTexts();

    void    setVerticalAlignment(Alignment alignment);
    void    setTextColor(const Color3f& color);
    void    setBackgroundColor(const Color3f& color);
    void    setBorderColor(const Color3f& color);
    void    setDrawBackground(bool drawBackground);
    void    setDrawBorder(bool drawBorder);

    Color3f textColor() const;
    Color3f backgroundColor() const;
    Color3f borderColor() const;
    bool    drawBackground() const;
    bool    drawBorder() const;

    void    render(OpenGLContext* oglContext, const MatrixState& matrixState);
    void    renderSoftware(OpenGLContext* oglContext, const MatrixState& matrixState);

private:
    void doRender2d(OpenGLContext* oglContext, const MatrixState& matrixState, bool softwareRendering);

private:
    ref<Font>           m_font;             // Font used to draw text
    std::vector<Vec3f>  m_positions;        // Coordinate of the lower left corner of where to place each individual text strings
    std::vector<String> m_texts;            // Text strings to be drawn
    
    bool                m_drawBackground;
    bool                m_drawBorder;

    Color3f             m_textColor;        
    Color3f             m_backgroundColor;  
    Color3f             m_borderColor;      

    short               m_verticalAlignment;// Vertical alignment for horizontal text
};

}  // namespace cvf
