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


enum EClassifier 
{
   EClassNone,
   EClassAttrib,
   EClassVarOut,
   EClassVarIn,
   EClassRes,
   EClassUniform
};


class HlslLinker
{
private:
	// GLSL string for additional extension prepropressor directives.
	// This is used for extensions that expose built-in variables.
	std::stringstream additionalExtensions;

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

   bool link(HlslCrossCompiler*, const char* entry, bool usePrecision);

   std::string stripSemanticModifier(const std::string &semantic, bool bWarn);

   EAttribSemantic parseAttributeSemantic(const std::string &semantic);

   bool setUserAttribName ( EAttribSemantic eSemantic, const char *pName );

   void setUseUserVaryings ( bool bUseUserVaryings ) { bUserVaryings = bUseUserVaryings; }

   const char* getShaderText() const;
      
   int getUniformCount() const { return (int)uniforms.size(); }

   const ShUniformInfo* getUniformInfo() const  { return ( ( uniforms.size() ) ? &uniforms[0] : 0 ); }
   
   TInfoSink& infoSink;

protected:
	typedef std::vector<GlslFunction*> FunctionSet;

   bool addCalledFunctions( GlslFunction *func, FunctionSet& funcSet, std::vector<GlslFunction*> &funcList);

   bool getArgumentData2( const std::string &name, const std::string &semantic, EGlslSymbolType type,
                               EClassifier c, std::string &outName, std::string &ctor, int &pad, int semanticOffset);

   bool getArgumentData( GlslSymbol* sym, EClassifier c, std::string &outName,
                  std::string &ctor, int &pad);
	
	typedef std::set<const char*> ExtensionSet;
	void addRequiredExtensions(EAttribSemantic sem, ExtensionSet& extensions);
	
private:
	bool linkerSanityCheck(HlslCrossCompiler* compiler, const char* entryFunc);
	bool buildFunctionLists(HlslCrossCompiler* comp, EShLanguage lang, const std::string& entryPoint, std::vector<GlslFunction*>& globalList, std::vector<GlslFunction*>& functionList, FunctionSet& calledFunctions, GlslFunction*& funcMain);
	void buildUniformsAndLibFunctions(const FunctionSet& calledFunctions, std::vector<GlslSymbol*>& constants, std::set<TOperator>& libFunctions);
	void emitLibraryFunctions(const std::set<TOperator>& libFunctions, EShLanguage lang, bool usePrecision);
	void emitStructs(HlslCrossCompiler* comp);
	bool emitInputNonStructParam(GlslSymbol* sym, EShLanguage lang, bool usePrecision, EAttribSemantic attrSem, std::stringstream& attrib, std::stringstream& varying, std::stringstream& preamble, std::stringstream& call);
};

#endif //HLSL_LINKER_H
