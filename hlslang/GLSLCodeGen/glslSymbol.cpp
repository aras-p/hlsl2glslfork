// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "glslSymbol.h"
#include <float.h>
#include <cstring>

// Check against names that are keywords in GLSL, but not HLSL
static bool IsReservedGlslKeyword (const std::string& name)
{
	if ((name == "input") ||
        (name == "output") ||
		(name == "varying") ||
		(name == "attribute"))
	{
		return true;
	}
	
	return false;
}


GlslSymbol::GlslSymbol( const std::string &n, const std::string &s, int id, EGlslSymbolType t, TPrecision prec, EGlslQualifier q, int as ) :
   semantic(s),
   identifier(id),
   type(t),
   precision(prec),
   qual(q),
   arraySize(as),
   mangleCounter(0),
   structPtr(0),
   isParameter(false),
   refCount(0)
{
	if (IsReservedGlslKeyword(n))
	{
		name = "xlat_var_" + n;
	}
	else
	{
		name = n;
	}
	mangledName = name;

	if (qual == EqtMutableUniform)
		mutableMangledName = "xlat_mutable_" + mangledName;
	else
		mutableMangledName = mangledName;   
}



void GlslSymbol::writeDecl (std::stringstream& out, unsigned flags)
{
	const bool writeMutableUniforms = (flags & WRITE_DECL_MUTABLE_UNIFORMS);
	
	switch (qual)
	{
	case EqtNone:            break;
	case EqtUniform:
		if (writeMutableUniforms)
			out << "uniform ";
		break;
	case EqtMutableUniform:
		if (!writeMutableUniforms)
			out << "uniform ";
		break;
	case EqtConst:           out << "const ";   break;
	case EqtIn:              out << "in ";      break;
	case EqtOut:             out << "out ";     break;
	case EqtInOut:           out << "inout ";   break;
	}

	writeType (out, type, structPtr, precision);

	out << " " << (writeMutableUniforms ? mutableMangledName : mangledName);
	if (arraySize)
		out << "[" << arraySize << "]";
	
	if (writeMutableUniforms && qual == EqtMutableUniform)
		out << " = " << mangledName;
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
	static char buffer[64];
	
	if (fractionalPart(f) == 0.f)
		sprintf(buffer, "%.1f", f);
	else
		sprintf(buffer, "%.*g", FLT_DIG, f);

	out << buffer;
}

void GlslSymbol::mangleName()
{
	std::stringstream s;
	mangleCounter++;
	s << "_" << mangleCounter;
	mangledName = name + s.str();
	if ( qual == EqtMutableUniform) 
		mutableMangledName = "xlat_mutable_" + mangledName;
	else
		mutableMangledName = mangledName;
}
