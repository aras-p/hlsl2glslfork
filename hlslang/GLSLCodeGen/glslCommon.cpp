// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "glslStruct.h"

/// Table to convert GLSL variable types to strings
const char typeString[EgstTypeCount][32] = 
{
   "void",
   "bool",
   "bvec2",
   "bvec3",
   "bvec4",
   "int",
   "ivec2",
   "ivec3",
   "ivec4",
   "float",
   "vec2",
   "vec3",
   "vec4",
   "mat2",
   "mat3",
   "mat4",
   "sampler",
   "sampler1D",
   "sampler1DShadow",
   "sampler2D",
   "sampler2DShadow",
   "sampler3D",
   "samplerCube",
   "sampler2DRect",
   "sampler2DRectShadow",
   "struct"
};

const char* getGLSLPrecisiontring (TPrecision p)
{
	switch (p) {
	case EbpLow: return "lowp ";
	case EbpMedium: return "mediump ";
	case EbpHigh: return "highp ";
	case EbpUndefined: return "";
	default: assert(false); return "";
	}
}

/// Outputs the type of the symbol to the output buffer
///    \param out
///       The output buffer to write the type to
///    \param type
///       The type of the GLSL symbol to output
///    \param s
///       If it is a structure, a pointer to the structure to write out
void writeType (std::stringstream &out, EGlslSymbolType type, GlslStruct *s, TPrecision precision)
{
	if (type >= EgstInt) // precision does not apply to void/bool
		out << getGLSLPrecisiontring (precision);

   switch (type)
   {
   case EgstVoid:  
   case EgstBool:  
   case EgstBool2: 
   case EgstBool3: 
   case EgstBool4: 
   case EgstInt:   
   case EgstInt2:  
   case EgstInt3:  
   case EgstInt4:  
   case EgstFloat: 
   case EgstFloat2:
   case EgstFloat3:
   case EgstFloat4:
   case EgstFloat2x2:
   case EgstFloat3x3:
   case EgstFloat4x4:
   case EgstSamplerGeneric: 
   case EgstSampler1D:
   case EgstSampler1DShadow:
   case EgstSampler2D:
   case EgstSampler2DShadow:
   case EgstSampler3D:
   case EgstSamplerCube:
   case EgstSamplerRect:
   case EgstSamplerRectShadow:
      out << typeString[type];
      break;
   case EgstStruct:
      if (s)
         out << s->getName();
      else
         out << "struct";
      break;
   }
}


const char* getTypeString( const EGlslSymbolType t )
{
   assert (t >= EgstVoid && t < EgstTypeCount);
   return typeString[t];
}


EGlslSymbolType translateType( const TType *type )
{
   if ( type->isMatrix() )
   {
      switch (type->getNominalSize())
      {
      case 2:  return EgstFloat2x2;
      case 3:  return EgstFloat3x3;
      case 4:  return EgstFloat4x4;
      }
   }
   else
   {
      switch (type->getBasicType())
      {
      case EbtVoid:
         return EgstVoid;
      case EbtBool:
         return EGlslSymbolType(EgstBool + (type->getNominalSize() - 1));
      case EbtInt:
         return EGlslSymbolType(EgstInt + (type->getNominalSize() - 1));
      case EbtFloat:
         return EGlslSymbolType(EgstFloat + (type->getNominalSize() - 1));
      case EbtSamplerGeneric:
         return EgstSamplerGeneric;
      case EbtSampler1D:
         return EgstSampler1D;
	  case EbtSampler1DShadow:
		  return EgstSampler1DShadow;
      case EbtSampler2D:
         return EgstSampler2D;
	  case EbtSampler2DShadow:
		  return EgstSampler2DShadow;
      case EbtSampler3D:
         return EgstSampler3D;
      case EbtSamplerCube:
         return EgstSamplerCube;
	  case EbtSamplerRect:
		  return EgstSamplerRect;
	  case EbtSamplerRectShadow:
		  return EgstSamplerRectShadow;
      case EbtStruct:
         return EgstStruct;
      }
   }

   return EgstVoid;
}


EGlslQualifier translateQualifier( TQualifier qual )
{
   switch (qual)
   {
   case EvqTemporary:     return EqtNone;
   case EvqGlobal:        return EqtNone;
   case EvqConst:         return EqtConst;
   case EvqAttribute:     return EqtNone;
   case EvqVaryingIn:     return EqtNone;
   case EvqVaryingOut:    return EqtNone;
   case EvqUniform:       return EqtUniform;
   case EvqMutableUniform: return EqtMutableUniform;
   case EvqIn:            return EqtIn;
   case EvqOut:           return EqtOut;
   case EvqInOut:         return EqtInOut;

   }

   return EqtNone;
}
