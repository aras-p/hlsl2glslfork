// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "glslSymbol.h"


static void writeBool( std::stringstream &out, bool b )
{
   out << ((b) ? "true" : "false");
}


GlslSymbol::GlslSymbol( const std::string &n, const std::string &s, int id, EGlslSymbolType t, TPrecision prec, EGlslQualifier q, int as ) :
   semantic(s),
   identifier(id),
   type(t),
   precision(prec),
   qual(q),
   arraySize(as),
   mangle(0),
   structPtr(0),
   isParameter(false),
   refCount(0)
{
   if ( isReservedGlslKeyword ( n ) )
   {
      name = "xlat_var_" + n;
   }
   else
   {
      name = n;
   }
   mangledName = name;

   if ( qual == EqtMutableUniform)
      mutableMangledName = "xlat_mutable_" + mangledName;
   else
      mutableMangledName = mangledName;   
}

GlslSymbol::~GlslSymbol() 
{
}
   

bool GlslSymbol::isReservedGlslKeyword ( const std::string &name ) const
{
   // Check against names that are keywords in GLSL, but not HLSL
   if ( (name == "input") ||
        (name == "output") )
   {
      return true;
   }

   return false;
}


void GlslSymbol::writeDecl (std::stringstream &out, bool local, bool skipUniform)
{
   switch (qual)
   {
   case EqtNone:            break;
   case EqtUniform:
      if (!skipUniform)
		  out << "uniform ";
      break;
   case EqtMutableUniform:
      if (!local)
         out << "uniform ";
      break;
   case EqtConst:           out << "const ";   break;
   case EqtIn:              out << "in ";      break;
   case EqtOut:             out << "out ";     break;
   case EqtInOut:           out << "inout ";   break;
   }

   writeType (out, type, structPtr, precision);

   out << " ";

   if (local)
      out << mutableMangledName;
   else
      out << mangledName;

   if (arraySize)
   {
      out << "[" << arraySize << "]";
   }
}


void GlslSymbol::writeInitializer( std::stringstream &out, int element )
{
   int elementCount = 0;
   EGlslSymbolType baseType = EgstVoid;

   switch (type)
   {
   case EgstBool:
   case EgstBool2:
   case EgstBool3:
   case EgstBool4:
      baseType = EgstBool;
      elementCount = type - EgstBool + 1;
      break;

   case EgstInt:
   case EgstInt2:
   case EgstInt3:
   case EgstInt4:
      baseType = EgstInt;
      elementCount = type - EgstInt + 1;
      break;

   case EgstFloat:
   case EgstFloat2:
   case EgstFloat3:
   case EgstFloat4:
      baseType = EgstFloat;
      elementCount = type - EgstFloat + 1;
      break;

   case EgstFloat2x2:
   case EgstFloat3x3:
   case EgstFloat4x4:
      baseType = EgstFloat;
      elementCount = type - EgstFloat2x2 + 2;
      elementCount *= elementCount;
      break;

   default:
      assert(0);
   }

   int offset = elementCount * element;

   if (baseType == EgstBool)
   {
      if (elementCount > 1)
         out << getTypeString(type) << "( ";

      for (int ii = 0; ii < elementCount; ii++)
      {
         if (ii != 0) out << ", ";
         writeBool( out, initializer[ ii + offset] != 0.0f);
      }

      if (elementCount > 1)
         out << ")";
   }
   else if (baseType == EgstInt)
   {
      if (elementCount > 1)
         out << getTypeString(type) << "( ";

      for (int ii = 0; ii < elementCount; ii++)
      {
         if (ii != 0) out << ", ";
         out << (int)initializer[ ii + offset];
      }

      if (elementCount > 1)
         out << ")";
   }
   else if (baseType == EgstFloat)
   {
      if (elementCount > 1)
         out << getTypeString(type) << "( ";

      for (int ii = 0; ii < elementCount; ii++)
      {
         if (ii != 0) out << ", ";
         out << initializer[ ii + offset];
      }

      if (elementCount > 1)
         out << ")";
   }
   else
   {
      assert(0);
   }
}


void GlslSymbol::setInitializer( const constUnion *ptr )
{
   int elementCount = 0;
   EGlslSymbolType baseType = EgstVoid;
   int as = arraySize;

   as = (as) ? as : 1;

   // No support for initialization of samplers or structures presently
   assert( type < EgstSamplerGeneric);
   
   initializer.clear();

   switch (type)
   {
   case EgstBool:
   case EgstBool2:
   case EgstBool3:
   case EgstBool4:
      baseType = EgstBool;
      elementCount = type - EgstBool + 1;
      break;

   case EgstInt:
   case EgstInt2:
   case EgstInt3:
   case EgstInt4:
      baseType = EgstInt;
      elementCount = type - EgstInt + 1;
      break;

   case EgstFloat:
   case EgstFloat2:
   case EgstFloat3:
   case EgstFloat4:
      baseType = EgstFloat;
      elementCount = type - EgstFloat + 1;
      break;

   case EgstFloat2x2:
   case EgstFloat3x3:
   case EgstFloat4x4:
      baseType = EgstFloat;
      elementCount = type - EgstFloat2x2 + 2;
      elementCount *= elementCount; //square it
      break;

   default:
      assert(0);
   }

   // Handle the all the array elements
   elementCount *= as;

   if (baseType == EgstBool)
   {
      for (int ii = 0; ii < elementCount; ii++)
      {
         initializer.push_back( ptr[ii].getBConst() ? 1.0f : 0.0f);
      }
   }
   else if (baseType == EgstInt)
   {
      for (int ii = 0; ii < elementCount; ii++)
      {
         initializer.push_back( (float)ptr[ii].getIConst());
      }
   }
   else if (baseType == EgstFloat)
   {
      for (int ii = 0; ii < elementCount; ii++)
      {
         initializer.push_back( ptr[ii].getFConst());
      }
   }
   else
   {
      assert(0);
   }
}


void GlslSymbol::mangleName() 
{
   std::stringstream s;
   mangle++;
   s << "_" << mangle;
   mangledName = name + s.str();
   if ( qual == EqtMutableUniform) 
      mutableMangledName = "xlat_mutable_" + mangledName;
   else
      mutableMangledName = mangledName;
}
