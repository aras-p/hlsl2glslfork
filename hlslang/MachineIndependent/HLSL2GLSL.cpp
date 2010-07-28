//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
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
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
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

#include "SymbolTable.h"
#include "ParseHelper.h"

#include "../Include/ShHandle.h"
#include "InitializeDll.h"

#include "../../include/hlsl2glsl.h"
#include "Initialize.h"
#include "../GLSLCodeGen/hlslSupportLib.h"

#include "../GLSLCodeGen/hlslCrossCompiler.h"
#include "../GLSLCodeGen/HlslLinker.h"


// A symbol table for each language.  Each has a different set of built-ins, and we want to preserve that 
// from compile to compile.
TSymbolTable SymbolTables[EShLangCount];


// Global pool allocator (per process)
TPoolAllocator* PerProcessGPA = 0;


/// Initializize the symbol table
/// \param BuiltInStrings
///      Pointer to built in strings.
/// \param language
///      Shading language to initialize symbol table for
/// \param infoSink
///      Information sink (for errors/warnings)
/// \param symbolTables
///      Array of symbol tables (one for each language)
/// \param bUseGlobalSymbolTable
///      Whether to use the global symbol table or the per-language symbol table
/// \return
///      True if succesfully initialized, false otherwise
bool InitializeSymbolTable( TBuiltInStrings* BuiltInStrings, EShLanguage language, TInfoSink& infoSink, 
                            TSymbolTable* symbolTables, bool bUseGlobalSymbolTable )
{
   TIntermediate intermediate(infoSink); 
   TSymbolTable* symbolTable;

   if ( bUseGlobalSymbolTable )
      symbolTable = symbolTables;
   else
      symbolTable = &symbolTables[language];

   TParseContext parseContext(*symbolTable, intermediate, language, infoSink);

   GlobalParseContext = &parseContext;

   setInitialState();

   assert(symbolTable->isEmpty() || symbolTable->atSharedBuiltInLevel());

   //
   // Parse the built-ins.  This should only happen once per
   // language symbol table.
   //
   // Push the symbol table to give it an initial scope.  This
   // push should not have a corresponding pop, so that built-ins
   // are preserved, and the test for an empty table fails.
   //

   symbolTable->push();

   //Initialize the Preprocessor
   int ret = InitPreprocessor();
   if (ret)
   {
      infoSink.info.message(EPrefixInternalError,  "Unable to intialize the Preprocessor");
      return false;
   }

   for (TBuiltInStrings::iterator i  = BuiltInStrings[parseContext.language].begin();
       i != BuiltInStrings[parseContext.language].end();
       ++i)
   {
      const char* builtInShaders = (*i).c_str();

      if (PaParseString(const_cast<char*>(builtInShaders), parseContext) != 0)
      {
         infoSink.info.message(EPrefixInternalError, "Unable to parse built-ins");
         return false;
      }
   }

   if (  !bUseGlobalSymbolTable )
   {
      IdentifyBuiltIns(parseContext.language, *symbolTable);
   }

   FinalizePreprocessor();

   return true;
}


/// Generate the built in symbol table
/// \param infoSink
///      Information sink (for errors/warnings)
/// \param symbolTables
///      Array of symbol tables (one for each language)
/// \param bUseGlobalSymbolTable
///      Whether to use the global symbol table or the per-language symbol table
/// \param language
///      Shading language to build symbol table for
/// \return
///      True if succesfully built, false otherwise
bool GenerateBuiltInSymbolTable(TInfoSink& infoSink, TSymbolTable* symbolTables, EShLanguage language)
{
   TBuiltIns builtIns;

   if ( language != EShLangCount )
   {      
      InitializeSymbolTable(builtIns.getBuiltInStrings(), language, infoSink, symbolTables, true);
   }
   else
   {
      builtIns.initialize();
      InitializeSymbolTable(builtIns.getBuiltInStrings(), EShLangVertex, infoSink, symbolTables, false);
      InitializeSymbolTable(builtIns.getBuiltInStrings(), EShLangFragment, infoSink, symbolTables, false);
   }

   return true;
}



int C_DECL Hlsl2Glsl_Initialize()
{
   TInfoSink infoSink;
   bool ret = true;

   if (!InitProcess())
      return 0;

   // This method should be called once per process. If its called by multiple threads, then 
   // we need to have thread synchronization code around the initialization of per process
   // global pool allocator
   if (!PerProcessGPA)
   {
      TPoolAllocator *builtInPoolAllocator = new TPoolAllocator(true);
      builtInPoolAllocator->push();
      TPoolAllocator* gPoolAllocator = &GlobalPoolAllocator;
      SetGlobalPoolAllocatorPtr(builtInPoolAllocator);

      TSymbolTable symTables[EShLangCount];
      GenerateBuiltInSymbolTable(infoSink, symTables, EShLangCount);

      PerProcessGPA = new TPoolAllocator(true);
      PerProcessGPA->push();
      SetGlobalPoolAllocatorPtr(PerProcessGPA);

      SymbolTables[EShLangVertex].copyTable(symTables[EShLangVertex]);
      SymbolTables[EShLangFragment].copyTable(symTables[EShLangFragment]);

      SetGlobalPoolAllocatorPtr(gPoolAllocator);

      symTables[EShLangVertex].pop();
      symTables[EShLangFragment].pop();

      initializeHLSLSupportLibrary();

      builtInPoolAllocator->popAll();
      delete builtInPoolAllocator;        

   }

   return ret ? 1 : 0;
}



int C_DECL Hlsl2Glsl_Finalize()
{
   if (PerProcessGPA)
   {
      SymbolTables[EShLangVertex].pop();
      SymbolTables[EShLangFragment].pop();

      PerProcessGPA->popAll();
      delete PerProcessGPA;
      PerProcessGPA = NULL;
      finalizeHLSLSupportLibrary();
   }
   return 1;
}


ShHandle C_DECL Hlsl2Glsl_ConstructCompiler( const EShLanguage language, int debugOptions )
{
   if (!InitThread())
      return 0;

   HlslCrossCompiler* compiler = new HlslCrossCompiler(language, debugOptions);
   return compiler;
}

void C_DECL Hlsl2Glsl_DestructCompiler( ShHandle handle )
{
   if (handle == 0)
      return;

   HlslCrossCompiler* compiler = handle;
   delete handle;
}


int C_DECL Hlsl2Glsl_Parse( const ShHandle handle,
                            const char* shaderString,
                            int debugOptions )
{
   if (!InitThread())
      return 0;

   if (handle == 0)
      return 0;

   HlslCrossCompiler* compiler = handle;

   GlobalPoolAllocator.push();
   compiler->infoSink.info.erase();
   compiler->infoSink.debug.erase();

   if (!shaderString)
	   return 1;

   TIntermediate intermediate(compiler->infoSink);
   TSymbolTable symbolTable(SymbolTables[compiler->getLanguage()]);

   GenerateBuiltInSymbolTable(compiler->infoSink, &symbolTable, compiler->getLanguage());

   TParseContext parseContext(symbolTable, intermediate, compiler->getLanguage(), compiler->infoSink);

   GlobalParseContext = &parseContext;

   setInitialState();

   InitPreprocessor();    
   //
   // Parse the application's shaders.  All the following symbol table
   // work will be throw-away, so push a new allocation scope that can
   // be thrown away, then push a scope for the current shader's globals.
   //
   bool success = true;

   symbolTable.push();
   if (!symbolTable.atGlobalLevel())
      parseContext.infoSink.info.message(EPrefixInternalError, "Wrong symbol table level");


   int ret = PaParseString(const_cast<char*>(shaderString), parseContext);
   if (ret)
      success = false;

   if (success && parseContext.treeRoot)
   {
      success = intermediate.postProcess(parseContext.treeRoot, parseContext.language);

      if (success)
      {

         if (debugOptions & EDebugOpIntermediate)
            intermediate.outputTree(parseContext.treeRoot);

         //
         // Call the machine dependent compiler
         //
         if (! compiler->compile(parseContext.treeRoot))
            success = false;
      }      
   }
   else if (!success)
   {
      parseContext.infoSink.info.prefix(EPrefixError);
      parseContext.infoSink.info << parseContext.numErrors << " compilation errors.  No code generated.\n\n";
      success = false;
      if (debugOptions & EDebugOpIntermediate)
         intermediate.outputTree(parseContext.treeRoot);
   }

   intermediate.remove(parseContext.treeRoot);

   //
   // Ensure symbol table is returned to the built-in level,
   // throwing away all but the built-ins.
   //
   while (! symbolTable.atSharedBuiltInLevel())
      symbolTable.pop();

   FinalizePreprocessor();
   //
   // Throw away all the temporary memory used by the compilation process.
   //
   GlobalPoolAllocator.pop();

   return success ? 1 : 0;
}


int C_DECL Hlsl2Glsl_Translate( const ShHandle handle, const char* entry )
{
   if (handle == 0)
      return 0;

   HlslCrossCompiler* compiler = handle;
   compiler->infoSink.info.erase();
	if (!compiler->linkable())
	{
		compiler->infoSink.info.message(EPrefixError, "Shader does not have valid object code.");
		return 0;
	}

   bool ret = compiler->GetLinker()->link(compiler, entry);

   return ret ? 1 : 0;
}


const char* C_DECL Hlsl2Glsl_GetShader( const ShHandle handle )
{
	if (!handle)
		return 0;
	return handle->GetLinker()->getShaderText();
}


const char* C_DECL Hlsl2Glsl_GetInfoLog( const ShHandle handle )
{
   if (!InitThread())
      return 0;
   if (handle == 0)
      return 0;
   HlslCrossCompiler* base = static_cast<HlslCrossCompiler*>(handle);
   TInfoSink* infoSink = &(base->getInfoSink());
   infoSink->info << infoSink->debug.c_str();
   return infoSink->info.c_str();
}


int C_DECL Hlsl2Glsl_GetUniformCount( const ShHandle handle )
{
	if (!handle)
		return 0;
   const HlslLinker *linker = reinterpret_cast<const HlslLinker*>(handle);
   if (!linker)
      return 0;
   return linker->getUniformCount();
}


const ShUniformInfo* C_DECL Hlsl2Glsl_GetUniformInfo( const ShHandle handle )
{
	if (!handle)
		return 0;
   const HlslLinker *linker = reinterpret_cast<const HlslLinker*>(handle);
   if (!linker)
      return 0;
   return linker->getUniformInfo();
}


int C_DECL Hlsl2Glsl_SetUserAttributeNames ( ShHandle handle, 
                                             const EAttribSemantic *pSemanticEnums, 
                                             const char *pSemanticNames[], 
                                             int nNumSemantics )
{
	if (!handle)
		return 0;
	HlslLinker* linker = handle->GetLinker();

   for (int i = 0; i < nNumSemantics; i++ )
   {
      bool bError = linker->setUserAttribName ( pSemanticEnums[i], pSemanticNames[i] );

      if ( bError == false )
         return false;
   }

   return true;
}


int C_DECL Hlsl2Glsl_UseUserVaryings ( ShHandle handle, bool bUseUserVaryings )
{
	if (!handle)
		return 0;
	HlslLinker* linker = handle->GetLinker();
   linker->setUseUserVaryings ( bUseUserVaryings );
   return 1;
}
