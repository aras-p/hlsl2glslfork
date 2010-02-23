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
//
// ATI Research, Inc.
//
// Definition of GlslSymbol
//=================================================================================================================================

#ifndef GLSL_SYMBOL_H
#define GLSL_SYMBOL_H

#include "glslCommon.h"
#include "glslStruct.h"

//=================================================================================================================================
/// GlslSymbol
/// 
/// \brief This class provides the implementation for GLSL symbols
//=================================================================================================================================
class GlslSymbol 
{
public:

   //=========================================================================================================   
   /// Constructor
   //=========================================================================================================   
   GlslSymbol( const std::string &n, const std::string &s, int id, EGlslSymbolType t, EGlslQualifier q, int as = 0 );
   
   //=========================================================================================================
   /// Destructor
   //=========================================================================================================   
   virtual ~GlslSymbol();
   
   //=========================================================================================================   
   /// Get whether the symbol is a parameter
   //=========================================================================================================      
   bool getIsParameter() { return isParameter; }
   
   //=========================================================================================================   
   /// Set whether the symbol is a parameter
   //=========================================================================================================         
   void setIsParameter( bool param ) { isParameter = param; }
   
   //=========================================================================================================   
   /// Get whether the symbol is a global
   //=========================================================================================================            
   bool getIsGlobal() { return !isLocal(qual); }
   
   //=========================================================================================================   
   /// Get whether the symbol is mutable
   //=========================================================================================================               
   bool getIsMutable() { return qual == EqtMutableUniform; }

   //=========================================================================================================   
   /// Get mangled name of symbol
   //=========================================================================================================                  
   const std::string &getName( bool local = true ) const { return ( (local ) ? mutableMangledName : mangledName ); }
   
   //=========================================================================================================   
   /// Get original name of symbol
   //=========================================================================================================                     
   const std::string &getOrgName() { return name; }
   
   //=========================================================================================================   
   /// Get whether the symbol has an HLSL semantic
   //=========================================================================================================                              
   bool hasSemantic() const { return (semantic.size() > 0); }
   
   //=========================================================================================================   
   /// Get symbol HLSL semantic
   //=========================================================================================================                        
   const std::string &getSemantic() const { return semantic; }
   
   //=========================================================================================================   
   /// Get the symbol ID
   //=========================================================================================================                           
   int getId() { return identifier; }
   
   //=========================================================================================================   
   /// Get whether the symbol is an array
   //=========================================================================================================                                
   bool isArray() const { return (arraySize > 0); }
   
   //=========================================================================================================   
   /// Get the symbol array size
   //=========================================================================================================                                   
   int getArraySize() const { return arraySize; }

   //=========================================================================================================   
   /// Get the symbol GLSL type
   //=========================================================================================================                                     
   EGlslSymbolType getType() const { return type; }
   
   //=========================================================================================================   
   /// Get the symbol GLSL qualifier
   //=========================================================================================================                                        
   EGlslQualifier getQualifier() const { return qual; }
   
   //=========================================================================================================   
   /// Update the type of the symbol
   //=========================================================================================================                                           
   void updateType( EGlslSymbolType t ) { assert( type == EgstSamplerGeneric); type = t; }

   //=========================================================================================================   
   /// Get the GLSL struct for symbol
   //=========================================================================================================                                              
   GlslStruct* getStruct() { return structPtr; }
   
   //=========================================================================================================   
   /// Get the GLSL struct for symbol (const)
   //=========================================================================================================                                                 
   const GlslStruct* getStruct() const { return structPtr; }

   //=========================================================================================================   
   /// Set the GLSL struct for symbol
   //=========================================================================================================                                                    
   void setStruct( GlslStruct *s ) { structPtr = s; }

   //=========================================================================================================   
   /// Get whether the symbol has an initializer
   //=========================================================================================================                                                         
   bool hasInitializer() const { return initializer.size() != 0; }

   //=========================================================================================================   
   /// Get the symbol initializer
   //=========================================================================================================                                                            
   const float* getInitializer() const { return &initializer[0]; }

   //=========================================================================================================   
   /// Get the size of the initializer
   //=========================================================================================================                                                               
   int initializerSize() const { return (int)initializer.size(); }

   //=========================================================================================================
   /// Write symbol declaration to the output stream
   //=========================================================================================================
   void writeDecl( std::stringstream &out, bool local = false );

   //=========================================================================================================
   /// Write symbol initializer to the output stream
   //=========================================================================================================   
   void writeInitializer( std::stringstream &out, int element = 0);


   //=========================================================================================================
   /// Set symbol to be initialized to a constant value
   //=========================================================================================================   
   void setInitializer( const constUnion *ptr );


   //=========================================================================================================
   /// Set the mangled name for the symbol
   //=========================================================================================================
   void mangleName();    

   //=========================================================================================================
   /// Add a reference to the symbol
   //=========================================================================================================
   void addRef() { refCount++; }

   //=========================================================================================================
   /// Release a reference to the symbol
   //=========================================================================================================
   void releaseRef() { assert (refCount >= 0 ); if ( refCount > 0 ) refCount--; }

   //=========================================================================================================
   /// Get the reference count for the symbol
   //=========================================================================================================
   int getRef() { return refCount; }

   
private:

   //=========================================================================================================
   /// Is the variable name a reserved GLSL keyword that could have passed through the lexer?
   //=========================================================================================================
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

