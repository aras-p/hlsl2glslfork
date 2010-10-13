// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef HLSL_LINKER_H
#define HLSL_LINKER_H

#include <sstream>

#include "../Include/Common.h"

#include "glslFunction.h"

class HlslCrossCompiler;


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


/// Implementation of the linker that will actually initiate the generation of the final GLSL code.
class HlslLinker
{
private:

	// GLSL string for generated shader
	std::stringstream shader;
   
   // Uniform list
   std::vector<ShUniformInfo> uniforms;

   // Helper string to store shader text
   mutable std::string bs;

   // Table holding the list of user attribute names per semantic
   char userAttribString[EAttrSemCount][MAX_ATTRIB_NAME];

   // For varyings, determines whether the linker attempts to use user or built-in varyings
   bool bUserVaryings;


public:

   HlslLinker(TInfoSink& infoSink);
   ~HlslLinker();
	
   TInfoSink& getInfoSink() { return infoSink; }

   /// Main function that initiates code generation for the shader.  
   bool link(HlslCrossCompiler*, const char* entry, bool usePrecision);

   /// Strip the semantic string of any modifiers (e.g. _centroid)
   std::string stripSemanticModifier(const std::string &semantic, bool bWarn);

   /// Determine the GLSL attribute semantic for a given HLSL semantic
   EAttribSemantic parseAttributeSemantic(const std::string &semantic);

   /// If the user elects to use user attributes rather than built-ins, this function will set the user
   /// attribute name the user wishes to use for the semantic passed in.
   bool setUserAttribName ( EAttribSemantic eSemantic, const char *pName );

   /// Enable the use of user-varyings rather than built-ins in the output code generation
   void setUseUserVaryings ( bool bUseUserVaryings ) { bUserVaryings = bUseUserVaryings; }

   /// Interface to retreive the output GLSL shader text
   const char* getShaderText() const;
      
   /// Get the number of uniforms
   int getUniformCount() const { return (int)uniforms.size(); }

   /// Return the uniform info table
   const ShUniformInfo* getUniformInfo() const  { return ( ( uniforms.size() ) ? &uniforms[0] : 0 ); }
   
   // Info sink for storing output
   TInfoSink& infoSink;

protected:
	typedef std::vector<GlslFunction*> FunctionSet;

   /// Add the functions called by a function to the function set
   bool addCalledFunctions( GlslFunction *func, FunctionSet& funcSet, std::vector<GlslFunction*> &funcList);

   /// Get the GLSL name, constructor type and padding for a given argument 
   bool getArgumentData2( const std::string &name, const std::string &semantic, EGlslSymbolType type,
                               EClassifier c, std::string &outName, std::string &ctor, int &pad, TPrecision prec, int semanticOffset);

   /// Get the GLSL name, constructor type and padding for a given symbol
   bool getArgumentData( GlslSymbol* sym, EClassifier c, std::string &outName,
                  std::string &ctor, int &pad);

};

#endif //HLSL_LINKER_H
