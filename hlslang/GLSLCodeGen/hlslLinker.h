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
// Definition of HlslLinker
//=================================================================================================================================

#ifndef HLSL_LINKER_H
#define HLSL_LINKER_H

#include <sstream>

#include "../Include/Common.h"
#include "../Include/ShHandle.h"


#include "glslFunction.h"

#define MAX_ATTRIB_NAME 64

// Variable classifiers
enum EClassifier 
{
   EClassNone,
   EClassAttrib,
   EClassVarOut,
   EClassVarIn,
   EClassRes,
   EClassUniform
};


//=================================================================================================================================
/// HlslCrossCompiler
/// 
/// \brief This class provides the implementation of the linker that will actually initiate the generation of the final GLSL
///        code.
//=================================================================================================================================
class HlslLinker : public TLinker 
{
private:

	// GLSL string for vertex & fragment shaders
	std::stringstream shader[2];
   
   // Uniform list
   std::vector<ShUniformInfo> uniforms;

   // Helper string to store shader text
   mutable std::string bs;

   // Table holding the list of user attribute names per semantic
   char userAttribString[EAttrSemCount][MAX_ATTRIB_NAME];

   // For varyings, determines whether the linker attempts to use user or built-in varyings
   bool bUserVaryings;


public:

   //=========================================================================================================
   /// Constructor
   //=========================================================================================================
   HlslLinker(int dOptions);

   //=========================================================================================================
   /// Destructor
   //=========================================================================================================
   ~HlslLinker();
   
   //=========================================================================================================
   /// This function is the main function that initiates code generation for the shader.  
   //=========================================================================================================
   bool link(THandleList&, const char* vertEntry, const char* fragEntry);

   //=========================================================================================================
   /// Strip the semantic string of any modifiers (e.g. _centroid)
   //=========================================================================================================   
   std::string stripSemanticModifier(const std::string &semantic, bool bWarn);

   //=========================================================================================================
   /// Determine the GLSL attribute semantic for a given HLSL semantic
   //=========================================================================================================   
   EAttribSemantic parseAttributeSemantic(const std::string &semantic);

   //=========================================================================================================
   /// If the user elects to use user attributes rather than built-ins, this function will set the user
   /// attribute name the user wishes to use for the semantic passed in.
   //=========================================================================================================   
   virtual bool setUserAttribName ( EAttribSemantic eSemantic, const char *pName );

   //=========================================================================================================
   /// Enable the use of user-varyings rather than built-ins in the output code generation
   //=========================================================================================================   
   virtual void setUseUserVaryings ( bool bUseUserVaryings ) { bUserVaryings = bUseUserVaryings; }

   //=========================================================================================================
   /// Interface to retreive the output GLSL shader text
   //=========================================================================================================   
   const char* getShaderText( EShLanguage lang ) const;
      
   //=========================================================================================================
   /// Get the number of uniforms
   //=========================================================================================================
   int getUniformCount() const { return (int)uniforms.size(); }

   //=========================================================================================================
   /// Return the uniform info table
   //=========================================================================================================
   const ShUniformInfo* getUniformInfo() const  { return ( ( uniforms.size() ) ? &uniforms[0] : 0 ); }
   
   // Info sink for storing output
   TInfoSink infoSink;

   // Debug options
   int debugOptions;

protected:
	typedef std::vector<GlslFunction*> FunctionSet;

   //=========================================================================================================
   /// Add the functions called by a function to the function set
   //=========================================================================================================   
   bool addCalledFunctions( GlslFunction *func, FunctionSet& funcSet, std::vector<GlslFunction*> &funcList);


   //=========================================================================================================
   /// Get the GLSL name, constructor type and padding for a given argument 
   //=========================================================================================================   
   bool getArgumentData( const std::string &name, const std::string &semantic, EGlslSymbolType type,
                               EClassifier c, std::string &outName, std::string &ctor, int &pad, int semanticOffset = 0);

   //=========================================================================================================
   /// Get the GLSL name, constructor type and padding for a given symbol
   //=========================================================================================================   
   bool getArgumentData( GlslSymbol* sym, EClassifier c, std::string &outName,
                  std::string &ctor, int &pad);

};

#endif //HLSL_LINKER_H