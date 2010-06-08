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

#ifndef _HLSL2GLSL_INTERFACE_INCLUDED_
#define _HLSL2GLSL_INTERFACE_INCLUDED_

#ifdef _WIN32
   #define C_DECL __cdecl
   #define SH_IMPORT_EXPORT
#else
   #define SH_IMPORT_EXPORT
   #define __fastcall
   #define C_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

//=================================================================================================================================
//
//          Types / Enums
//
//=================================================================================================================================

//=========================================================================================================
/// Types of languages the HLSL2GLSL translator can consume.
//=========================================================================================================
typedef enum
{
   EShLangVertex,
   EShLangFragment,
   EShLangCount,
} EShLanguage;

//=========================================================================================================
/// Binding table.  This can be used for locating attributes, uniforms, globals, etc., as needed.
//=========================================================================================================
typedef struct
{
   char* name;
   int binding;
} ShBinding;

typedef struct
{
   int numBindings;
   ShBinding* bindings;  // array of bindings
} ShBindingTable;

//=========================================================================================================
/// GLSL shader variable types
/// NOTE: these are ordered to exactly match the internal enums
//=========================================================================================================
typedef enum
{
   EShTypeVoid,
   EShTypeBool,
   EShTypeBVec2,
   EShTypeBVec3,
   EShTypeBVec4,
   EShTypeInt,
   EShTypeIVec2,
   EShTypeIVec3,
   EShTypeIVec4,
   EShTypeFloat,
   EShTypeVec2,
   EShTypeVec3,
   EShTypeVec4,
   EShTypeMat2,
   EShTypeMat3,
   EShTypeMat4,
   EShTypeSampler,
   EShTypeSampler1D,
   EShTypeSampler2D,
   EShTypeSampler3D,
   EShTypeSamplerCube,
   EShTypeStruct
} EShType;

//=========================================================================================================
/// HLSL attribute semantics
/// NOTE: these are ordered to exactly match the internal tables
//=========================================================================================================
enum EAttribSemantic
{
   EAttrSemNone,
   EAttrSemPosition,
   EAttrSemNormal,
   EAttrSemColor0,
   EAttrSemColor1,
   EAttrSemColor2,
   EAttrSemColor3,
   EAttrSemTex0,
   EAttrSemTex1,
   EAttrSemTex2,
   EAttrSemTex3,
   EAttrSemTex4,
   EAttrSemTex5,
   EAttrSemTex6,
   EAttrSemTex7,
   EAttrSemTex8,
   EAttrSemTex9,
   EAttrSemTangent,
   EAttrSemBinormal,
   EAttrSemBlendWeight,
   EAttrSemBlendIndices,
   EAttrSemDepth,
   EAttrSemUnknown,
   EAttrSemCount
};


//=========================================================================================================
/// Uniform info struct
//=========================================================================================================
typedef struct
{
   char *name;
   char *semantic;
   EShType type;
   int arraySize;
   float *init;
   void *annt;
} ShUniformInfo;

//=========================================================================================================
/// Debug options
//=========================================================================================================
enum TDebugOptions
{
   EDebugOpNone               = 0x000,
   EDebugOpIntermediate       = 0x001,      
};



//=========================================================================================================
/// Generic opaque handle.  This type is used for handles to the parser/translator.
///
/// If handle creation fails, 0 will be returned.
///
//=========================================================================================================
typedef void* ShHandle;

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
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_Initialize();

//=========================================================================================================
/// Finalize the HLSL2GLSL translator.  This function should be called to de-initialize the HLSL2GLSL 
/// translator and should only be called once on shutdown.
/// 
/// \return
///   1 on success, 0 on failure
//=========================================================================================================
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_Finalize();

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
SH_IMPORT_EXPORT ShHandle C_DECL Hlsl2Glsl_ConstructParser( const EShLanguage language, 
                                                            int debugOptions );  

//=========================================================================================================
/// Construct a translator (one for each set of shaders to translate). Note that you can translate multiple 
/// vertex and fragment shaders together.  The Hlsl2Glsl_Translate function will take in a list of parsed 
/// shaders and link them together to produce the final translated output.
///
/// \return
///      Handle to a new translator, or 0 on failure.
//=========================================================================================================
SH_IMPORT_EXPORT ShHandle C_DECL Hlsl2Glsl_ConstructTranslator( int debugOptions ); 


//=========================================================================================================
/// Destroy a parser or translator
///
/// \param handle
///      Handle to a parser or translator.
//=========================================================================================================
SH_IMPORT_EXPORT void C_DECL Hlsl2Glsl_Destruct( ShHandle handle );


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
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_Parse( const ShHandle handle,
                                             const char* const shaderStrings[],
                                             const int numStrings,                                     
                                             int debugOptions );



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
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_Translate( const ShHandle translatorHandle,
                                                 const ShHandle parserHandles[],
                                                 const int numHandles,
                                                 const char* vertexEntry,
                                                 const char* fragmentEntry );


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
SH_IMPORT_EXPORT const char* C_DECL Hlsl2Glsl_GetShader( const ShHandle handle, 
                                                         EShLanguage lang );

//=========================================================================================================
/// After parsing or translating, retrieve the info log to get information.
///
/// \param handle
///      Handle to a parser or translator
/// \return 
///      As a string, the information log.  0 if the information is not available or the object is bad.
//=========================================================================================================
SH_IMPORT_EXPORT const char* C_DECL Hlsl2Glsl_GetInfoLog( const ShHandle handle );

//=========================================================================================================
/// After translating, retrieve the number of uniforms
///
/// \param handle
///      Handle to a translator, should be used only after calling Hlsl2Glsl_Translate
/// \return 
///      The number of uniforms in the translated shader.
//=========================================================================================================
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_GetUniformCount( const ShHandle handle );

//=========================================================================================================
/// After translating, retrieve the uniform info table
///
/// \param handle
///      Handle to a translator, should be used only after calling Hlsl2Glsl_Translate
/// \return 
///      The table of uniforms in the translated shader.  NULL if none.
//=========================================================================================================
SH_IMPORT_EXPORT const ShUniformInfo* C_DECL Hlsl2Glsl_GetUniformInfo( const ShHandle handle );

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
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_SetUserAttributeNames ( ShHandle handle, 
                                                              const EAttribSemantic *pSemanticEnums, 
                                                              const char *pSemanticNames[], 
                                                              int nNumSemantics );

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
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_UseUserVaryings ( ShHandle handle, 
                                                        bool bUseUserVaryings );


#ifdef __cplusplus
}
#endif

#endif // _HLSL2GLSL_INTERFACE_INCLUDED_

