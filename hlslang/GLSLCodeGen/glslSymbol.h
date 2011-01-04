// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef GLSL_SYMBOL_H
#define GLSL_SYMBOL_H

#include "glslCommon.h"
#include "glslStruct.h"


class GlslSymbol 
{
public:

   GlslSymbol( const std::string &n, const std::string &s, int id, EGlslSymbolType t, TPrecision precision, EGlslQualifier q, int as = 0 );
   virtual ~GlslSymbol();
   
   bool getIsParameter() const { return isParameter; }
	
   void setIsParameter( bool param ) { isParameter = param; }
   
   bool getIsGlobal() const { return qual == EqtUniform || qual == EqtMutableUniform; }
   
   bool getIsMutable() const { return qual == EqtMutableUniform; }

   /// Get mangled name
   const std::string &getName( bool local = true ) const { return ( (local ) ? mutableMangledName : mangledName ); }
   /// Get original name
   const std::string &getOrgName() const { return name; }
   
   bool hasSemantic() const { return (semantic.size() > 0); }
   const std::string &getSemantic() const { return semantic; }
   
   int getId() const { return identifier; }
   
   bool isArray() const { return (arraySize > 0); }
   int getArraySize() const { return arraySize; }

   EGlslSymbolType getType() const { return type; }
   EGlslQualifier getQualifier() const { return qual; }
   TPrecision getPrecision() const { return precision; }
   
   void updateType( EGlslSymbolType t ) { assert( type == EgstSamplerGeneric); type = t; }

   const GlslStruct* getStruct() const { return structPtr; }
   GlslStruct* getStruct() { return structPtr; }
   void setStruct( GlslStruct *s ) { structPtr = s; }

   bool hasInitializer() const { return initializer.size() != 0; }
   const float* getInitializer() const { return &initializer[0]; }
   int initializerSize() const { return (int)initializer.size(); }

   void writeDecl( std::stringstream &out, bool local, bool skipUniform );
   void writeInitializer( std::stringstream &out, int element = 0);

   void setInitializer( const constUnion *ptr );


   /// Set the mangled name for the symbol
   void mangleName();    

   void addRef() { refCount++; }
   void releaseRef() { assert (refCount >= 0 ); if ( refCount > 0 ) refCount--; }
   int getRef() const { return refCount; }

   
private:
   /// Is the variable name a reserved GLSL keyword that could have passed through the lexer?
   bool isReservedGlslKeyword ( const std::string &name ) const;

   std::string name;
   std::string mangledName;
   std::string mutableMangledName;
   std::string semantic;
   int identifier;
   EGlslSymbolType type;
   EGlslQualifier qual;
   TPrecision precision;
   int arraySize;
   int mangle;
   GlslStruct *structPtr;
   bool isParameter;
   std::vector<float> initializer; 
   int refCount;
};

#endif //GLSL_SYMBOL_H

