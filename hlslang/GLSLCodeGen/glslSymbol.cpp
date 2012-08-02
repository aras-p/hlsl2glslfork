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
	out << initializerString;
}

// two requirements here:
// 1) We always need a decimal-point so that the GLSL output is clearly a float
// 2) We want to have reasonably consistent behaviour between MacOS & Windows (so tests are consistent)
// historically there was this pattern instead 
//  active.setf ( std::stringstream::showpoint );
// 	active.unsetf(std::ios::fixed);
// 	active.unsetf(std::ios::scientific);
// 	active.precision (6);
// but the interpretation of precision was different between platforms

void GlslSymbol::writeFloat(std::stringstream &out, float f)
{
   char buffer[64];
   sprintf(buffer, "%g", f);
   out << buffer;
   
   if(!strchr(buffer, '.'))
   {
       out << ".0";
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
