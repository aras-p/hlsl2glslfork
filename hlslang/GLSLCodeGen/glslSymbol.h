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

#ifndef GLSL_SYMBOL_H
#define GLSL_SYMBOL_H

#include "glslCommon.h"
#include "glslStruct.h"


class GlslSymbol 
{
public:

   GlslSymbol( const std::string &n, const std::string &s, int id, EGlslSymbolType t, EGlslQualifier q, int as = 0 );   
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
   
   void updateType( EGlslSymbolType t ) { assert( type == EgstSamplerGeneric); type = t; }

   const GlslStruct* getStruct() const { return structPtr; }
   GlslStruct* getStruct() { return structPtr; }
   void setStruct( GlslStruct *s ) { structPtr = s; }

   bool hasInitializer() const { return initializer.size() != 0; }
   const float* getInitializer() const { return &initializer[0]; }
   int initializerSize() const { return (int)initializer.size(); }

   void writeDecl( std::stringstream &out, bool local = false );
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
   int arraySize;
   int mangle;
   GlslStruct *structPtr;
   bool isParameter;
   std::vector<float> initializer; 
   int refCount;
};

#endif //GLSL_SYMBOL_H

