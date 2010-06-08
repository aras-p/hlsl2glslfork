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

//=================================================================================================================================
//   ATI Research, Inc.
//
//   This file contains the interface to the HLSL2GLSL translator.  
//=================================================================================================================================

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================
#include "SymbolTable.h"
#include "ParseHelper.h"

#include "../Include/ShHandle.h"
#include "InitializeDll.h"

#include "../../include/hlsl2glsl.h"
#include "Initialize.h"
#include "../GLSLCodeGen/hlslSupportLib.h"

//=========================================================================================================
// A symbol table for each language.  Each has a different set of built-ins, and we want to preserve that 
// from compile to compile.
//=========================================================================================================
TSymbolTable SymbolTables[EShLangCount];

//=========================================================================================================
// Global pool allocator (per process)
//=========================================================================================================
TPoolAllocator* PerProcessGPA = 0;


//=================================================================================================================================
//
//          Private Helper Functions
//
//=================================================================================================================================

//=========================================================================================================
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
//=========================================================================================================
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
      const char* builtInShaders[1];
      int builtInLengths[1];

      builtInShaders[0] = (*i).c_str();
      builtInLengths[0] = (int) (*i).size();

      if (PaParseStrings(const_cast<char**>(builtInShaders), builtInLengths, 1, parseContext) != 0)
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

//=========================================================================================================
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
//=========================================================================================================
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

//=================================================================================================================================
//
//          Public Functions
//
//=================================================================================================================================

//=========================================================================================================
/// Initialize the HLSL2GLSL translator.  This function must be called once prior to calling any other
/// HLSL2GLSL translator functions
/// 
/// \return
///   1 on success, 0 on failure
//=========================================================================================================
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

//=========================================================================================================
/// Finalize the HLSL2GLSL translator.  This function should be called to de-initialize the HLSL2GLSL 
/// translator and should only be called once on shutdown.
/// 
/// \return
///   1 on success, 0 on failure
//=========================================================================================================
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

//=========================================================================================================
/// Construct a parser for the given language (one per shader)
///
/// \param language
///      The HLSL shader type of the input (EShLangVertex or EShLangFragment).  This should
///      be the type of the HLSL shader to translate.
/// \param debugOptions
///      Debug options (see TDebugOptions)
/// \return
///      Handle to a new parser, or 0 on failure.
//=========================================================================================================
ShHandle C_DECL Hlsl2Glsl_ConstructParser( const EShLanguage language, int debugOptions )
{
   if (!InitThread())
      return 0;

   TShHandleBase* base = static_cast<TShHandleBase*>(ConstructCompiler(language, debugOptions));

   return reinterpret_cast<void*>(base);
}

//=========================================================================================================
/// Construct a translator (one for each set of shaders to translate). Note that you can translate multiple 
/// vertex and fragment shaders together.  The Hlsl2Glsl_Translate function will take in a list of parsed 
/// shaders and link them together to produce the final translated output.
///
/// \return
///      Handle to a new translator, or 0 on failure.
//=========================================================================================================
ShHandle C_DECL Hlsl2Glsl_ConstructTranslator( int debugOptions )
{
   if (!InitThread())
      return 0;

   TShHandleBase* base = static_cast<TShHandleBase*>(ConstructLinker(debugOptions));

   return reinterpret_cast<void*>(base);
}

//=========================================================================================================
/// Destroy a parser or translator
///
/// \param handle
///      Handle to a parser or translator.
//=========================================================================================================
void C_DECL Hlsl2Glsl_Destruct( ShHandle handle )
{
   if (handle == 0)
      return;

   TShHandleBase* base = static_cast<TShHandleBase*>(handle);

   if (base->getAsCompiler())
      DeleteCompiler(base->getAsCompiler());
   else if (base->getAsLinker())
      DeleteLinker(base->getAsLinker());
}


//=========================================================================================================
/// Parse HLSL shader(s) to prepare it for final translation.  This function can take a string containing 
/// multiple HLSL shaders which will effectively be concatenated into a single shader to parse.
///
/// \param handle
///      Handle to a valid parser (created with HLSL2GLSL_ConstructParser)
/// \param shaderStrings
///      Array of HLSL shader source to parse
/// \param numStrings
///      Number of HLSL shader strings in the shaderStrings array
/// \param debugOptions
///      Debug options (see TDebugOptions)
/// \return 
///      The return value of Hlsl2Glsl_Parse is 1 on success, 0 on failure
///  The info-log should be written by Hlsl2Glsl_Parse into ShHandle, so it can answer future queries.
//=========================================================================================================
int C_DECL Hlsl2Glsl_Parse( const ShHandle handle,
                            const char* const shaderStrings[],
                            const int numStrings,                                     
                            int debugOptions )
{
   if (!InitThread())
      return 0;

   if (handle == 0)
      return 0;

   TShHandleBase* base = reinterpret_cast<TShHandleBase*>(handle);
   TCompiler* compiler = base->getAsCompiler();
   if (compiler == 0)
      return 0;

   GlobalPoolAllocator.push();
   compiler->infoSink.info.erase();
   compiler->infoSink.debug.erase();

   if (numStrings == 0)
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


   int ret = PaParseStrings(const_cast<char**>(shaderStrings), 0, numStrings, parseContext);
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

//=========================================================================================================
/// After parsing one or more HLSL shaders, do the final translation to GLSL.  This function also has
/// the ability to link together multiple HLSL parsed shaders.  So, for example, you can use this function
/// to generate both a translated vertex and fragment shader.  
///
/// \param translatorHandle
///      Handle to a valid translator (created with HLSL2GLSL_ConstructTranslator)
/// \param parserHandles
///      Array of parsed HLSL shaders (see Hlsl2Glsl_Parse)
/// \param numHandles
///      Number of HLSL parsed shaders in the parserHandles[] array
/// \param vertexEntry
///      The string name of the vertex shader main entry point (e.g. "main", "VS").  NULL if none.
/// \param fragmentEntry
///      The string name of the fragment shader main entry point (e.g. "main", "PS"). NULL if none.
/// \return 
///      The return value of Hlsl2Glsl_Translate is 1 on success, 0 on failure
///  The info-log should be written by Hlsl2Glsl_Translate into ShHandle, so it can answer future queries.
//=========================================================================================================
int C_DECL Hlsl2Glsl_Translate( const ShHandle translatorHandle,
                                const ShHandle parserHandles[],
                                const int numHandles,
                                const char* vertexEntry,
                                const char* fragmentEntry)
{
   if (translatorHandle == 0 || numHandles == 0)
      return 0;

   THandleList cObjects;

   {// support MSVC++6.0
      for (int i = 0; i < numHandles; ++i)
      {
         if (parserHandles[i] == 0)
            return 0;
         TShHandleBase* base = reinterpret_cast<TShHandleBase*>(parserHandles[i]);
         if (base->getAsLinker())
         {
            cObjects.push_back(base->getAsLinker());
         }
         if (base->getAsCompiler())
            cObjects.push_back(base->getAsCompiler());


         if (cObjects[i] == 0)
            return 0;
      }
   }

   TShHandleBase* base = reinterpret_cast<TShHandleBase*>(translatorHandle);
   TLinker* linker = static_cast<TLinker*>(base->getAsLinker());

   if (linker == 0)
      return 0;

   linker->infoSink.info.erase();

   {// support MSVC++6.0
      for (int i = 0; i < numHandles; ++i)
      {
         if (cObjects[i]->getAsCompiler())
         {
            if (! cObjects[i]->getAsCompiler()->linkable())
            {
               linker->infoSink.info.message(EPrefixError, "Not all shaders have valid object code.");                
               return 0;
            }
         }
      }
   }

   bool ret = linker->link(cObjects, vertexEntry, fragmentEntry);

   return ret ? 1 : 0;
}

//=========================================================================================================
/// After translating HLSL shader(s), retrieve the translated GLSL source.
///
/// \param handle
///      Handle to a translator, should be used only after calling Hlsl2Glsl_Translate
/// \param lang
///      Language to get the shader source for (vertex of fragment)
/// \return 
///      As a string, the translated GLSL source.  NULL if the GLSL source is not available.
//=========================================================================================================
const char* C_DECL Hlsl2Glsl_GetShader( const ShHandle handle, EShLanguage lang )
{
   const TShHandleBase *base = reinterpret_cast<const TShHandleBase*>(handle);
   const TLinker *linker = base->getAsLinker();

   if (!linker)
      return 0;

   return linker->getShaderText(lang);
}


//=========================================================================================================
/// After parsing or translating, retrieve the info log to get information.
///
/// \param handle
///      Handle to a parser or translator
/// \return 
///      As a string, the information log.  0 if the information is not available or the object is bad.
//=========================================================================================================
const char* C_DECL Hlsl2Glsl_GetInfoLog( const ShHandle handle )
{
   if (!InitThread())
      return 0;

   if (handle == 0)
      return 0;

   TShHandleBase* base = static_cast<TShHandleBase*>(handle);
   TInfoSink* infoSink;

   if (base->getAsCompiler())
      infoSink = &(base->getAsCompiler()->getInfoSink());
   else if (base->getAsLinker())
      infoSink = &(base->getAsLinker()->getInfoSink());

   infoSink->info << infoSink->debug.c_str();
   return infoSink->info.c_str();
}

//=========================================================================================================
/// After translating, retrieve the number of uniforms
///
/// \param handle
///      Handle to a translator, should be used only after calling Hlsl2Glsl_Translate
/// \return 
///      The number of uniforms in the translated shader.
//=========================================================================================================
int C_DECL Hlsl2Glsl_GetUniformCount( const ShHandle handle )
{
   const TShHandleBase *base = reinterpret_cast<const TShHandleBase*>(handle);
   const TLinker *linker = base->getAsLinker();

   if (!linker)
      return 0;

   return linker->getUniformCount();
}

//=========================================================================================================
/// After translating, retrieve the uniform info table
///
/// \param handle
///      Handle to a translator, should be used only after calling Hlsl2Glsl_Translate
/// \return 
///      The table of uniforms in the translated shader.  NULL if none.
//=========================================================================================================
const ShUniformInfo* C_DECL Hlsl2Glsl_GetUniformInfo( const ShHandle handle )
{
   const TShHandleBase *base = reinterpret_cast<const TShHandleBase*>(handle);
   const TLinker *linker = base->getAsLinker();

   if (!linker)
      return 0;

   return linker->getUniformInfo();
}

//=========================================================================================================
/// Instead of mapping HLSL attributes to GLSL fixed-function attributes, this function can be used to 
/// override the  attribute mapping.  This tells the code generator to use user-defined attributes for 
/// the semantics that are specified.
///
/// \param handle
///      Handle to the translator.  This should be called BEFORE calling Hlsl2Glsl_Translate
/// \param pSemanticEnums 
///      Array of semantic enums to set
/// \param pSemanticNames 
///      Array of user attribute names to use
/// \param nNumSemantics 
///      Number of semantics to set in the arrays
/// \return
///      1 on success, 0 on failure
//=========================================================================================================
int C_DECL Hlsl2Glsl_SetUserAttributeNames ( ShHandle handle, 
                                             const EAttribSemantic *pSemanticEnums, 
                                             const char *pSemanticNames[], 
                                             int nNumSemantics )
{
   TShHandleBase *base = reinterpret_cast<TShHandleBase*>(handle);
   TLinker *linker = base->getAsLinker();

   if (!linker)
      return 0;

   for (int i = 0; i < nNumSemantics; i++ )
   {
      bool bError = linker->setUserAttribName ( pSemanticEnums[i], pSemanticNames[i] );

      if ( bError == false )
         return false;
   }

   return true;
}

//=========================================================================================================
/// Instead of using OpenGL fixed-function varyings (such as gl_TexCoord[x]), use user named varyings 
/// instead.
/// 
/// \param handle
///      Handle to the translator.  This should be called BEFORE calling Hlsl2Glsl_Translate
/// \param bUseUserVarying 
///      If true, all user varyings will be used.  If false, the translator will attempt to use
///      GL fixed-function varyings
/// \return
///      1 on success, 0 on failure
//=========================================================================================================
int C_DECL Hlsl2Glsl_UseUserVaryings ( ShHandle handle, bool bUseUserVaryings )
{
   TShHandleBase *base = reinterpret_cast<TShHandleBase*>(handle);
   TLinker *linker = base->getAsLinker();

   if (!linker)
      return 0;

   linker->setUseUserVaryings ( bUseUserVaryings );
   return 1;
}
