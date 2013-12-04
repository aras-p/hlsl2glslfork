// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef GLSL_SYMBOL_H
#define GLSL_SYMBOL_H

#include "glslCommon.h"
#include "glslStruct.h"

class GlslSymbol : public GlslSymbolOrStructMemberBase
{
public:
	GlslSymbol( const std::string &n, const std::string &s, int id, EGlslSymbolType t, TPrecision precision, EGlslQualifier q, size_t as = 0 );
	inline virtual ~GlslSymbol() {};

	bool getIsParameter() const { return isParameter; }
	void setIsParameter( bool param ) { isParameter = param; }

	bool getIsGlobal() const { return isGlobal || qual == EqtUniform || qual == EqtMutableUniform; }
	void setIsGlobal(bool global) { isGlobal = global; }
	
	bool getIsMutable() const { return qual == EqtMutableUniform; }

	/// Get mangled name
	const std::string &getName( bool local = true ) const { return ( (local ) ? mutableMangledName : mangledName ); }

	bool hasSemantic() const { return (semantic.size() > 0); }

	int getId() const { return identifier; }

	TPrecision getPrecision() const { return precision; }

	void updateType( EGlslSymbolType t ) { assert( type == EgstSamplerGeneric); type = t; }

	const GlslStruct* getStruct() const { return structPtr; }
	GlslStruct* getStruct() { return structPtr; }
	void setStruct( GlslStruct *s ) { structPtr = s; }

	enum WriteDeclMode {
		kWriteDeclDefault = 0,
		kWriteDeclMutableDecl,
		kWriteDeclMutableInit,
	};
	void writeDecl (std::stringstream& out, WriteDeclMode mode);
	/// Set the mangled name for the symbol
	void mangleName();    

	void addRef() { refCount++; }
	void releaseRef() { assert (refCount >= 0 ); if ( refCount > 0 ) refCount--; }
	size_t getRef() const { return refCount; }

private:
	std::string mangledName;
	std::string mutableMangledName;
	int identifier;
	int mangleCounter;
	GlslStruct *structPtr;
	bool isParameter;
	size_t refCount;
	bool isGlobal;
};

#endif //GLSL_SYMBOL_H

