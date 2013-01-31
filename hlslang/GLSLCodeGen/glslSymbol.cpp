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
