//
//Copyright (C) 2005-2006  ATI Research, Inc.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of ATI Research, Inc. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//

//=================================================================================================================================
// ATI Research, Inc.
//
// Definition of GLSL common utility functions
//=================================================================================================================================

#ifndef GLSL_COMMON_H
#define GLSL_COMMON_H

#include <sstream>

#include "localintermediate.h"

// GLSL symbol types
enum EGlslSymbolType
{
   EgstVoid,
   EgstBool,
   EgstBool2,
   EgstBool3,
   EgstBool4,
   EgstInt,
   EgstInt2,
   EgstInt3,
   EgstInt4,
   EgstFloat,
   EgstFloat2,
   EgstFloat3,
   EgstFloat4,
   EgstFloat2x2,
   EgstFloat3x3,
   EgstFloat4x4,
   EgstSamplerGeneric,
   EgstSampler1D,
   EgstSampler2D,
   EgstSampler3D,
   EgstSamplerCube,
   EgstStruct,
   EgstTypeCount
};

// GLSL qualifier enums
enum EGlslQualifier
{
   EqtNone,
   EqtUniform,
   EqtMutableUniform,
   EqtConst,
   EqtIn,
   EqtOut,
   EqtInOut
};


// Forward Declarations
class GlslStruct;

//=========================================================================================================
/// Outputs the type of the symbol to the output buffer
//=========================================================================================================
void writeType( std::stringstream &out, EGlslSymbolType type, GlslStruct *s );

//=========================================================================================================
/// Gets the type as a string
//=========================================================================================================
const char *getTypeString( const EGlslSymbolType t );

//=========================================================================================================
/// Translates the type to a GLSL symbol type
//=========================================================================================================
EGlslSymbolType translateType( const TType *type );

//=========================================================================================================
/// Returns whether the qualifier is associated with a local or global
//=========================================================================================================
bool isLocal( EGlslQualifier qual );

//=========================================================================================================
/// Translates the qualifier to a GLSL qualifier enumerant
//=========================================================================================================
EGlslQualifier translateQualifier( TQualifier qual);

#endif //GLSL_COMMON_H
