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

#include "cvfString.h"

namespace cvf {


//=================================================================================================
// 
// Trace class
// 
//=================================================================================================
class Trace
{
public:
    static void show(String message);
    static void show(const char* format, ...);
    static void showFileLineNumber(const String& file, int line, const String& message);

private:
    static void showTraceOutput(String text, bool addNewLine);
};

#define CVF_TRACE_FILELINE               Trace::showFileLineNumber(__FILE__, __LINE__, "");
#define CVF_TRACE_FILELINE_MSG(message)  Trace::showFileLineNumber(__FILE__, __LINE__, message);

}
