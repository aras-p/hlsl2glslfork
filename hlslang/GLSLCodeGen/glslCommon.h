// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


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
   EgstSamplerRect,
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

/// Outputs the type of the symbol to the output buffer
void writeType(std::stringstream &out, EGlslSymbolType type, GlslStruct *s, TPrecision precision);

const char *getTypeString( const EGlslSymbolType t );
const char *getGLSLPrecisiontring (TPrecision prec);

/// Translates the type to a GLSL symbol type
EGlslSymbolType translateType( const TType *type );

/// Translates the qualifier to a GLSL qualifier enumerant
EGlslQualifier translateQualifier( TQualifier qual);

#endif //GLSL_COMMON_H
