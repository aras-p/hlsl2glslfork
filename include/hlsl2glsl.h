// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

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


/// Types of languages the HLSL2GLSL translator can consume.
typedef enum
{
   EShLangVertex,
   EShLangFragment,
   EShLangCount,
} EShLanguage;

/// Binding table.  This can be used for locating attributes, uniforms, globals, etc., as needed.
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


/// GLSL shader variable types
/// NOTE: these are ordered to exactly match the internal enums
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


/// HLSL attribute semantics
/// NOTE: these are ordered to exactly match the internal tables
enum EAttribSemantic
{
   EAttrSemNone,
   EAttrSemPosition,
   EAttrSemVPos,
   EAttrSemVFace,
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


/// Uniform info struct
typedef struct
{
   char *name;
   char *semantic;
   EShType type;
   int arraySize;
   float *init;
   void *annt;
} ShUniformInfo;


/// Translation options
enum TTranslateOptions
{
   ETranslateOpNone = 0,
   ETranslateOpIntermediate = (1<<0),
   ETranslateOpUsePrecision = (1<<1),
};



/// Generic opaque handle.  This type is used for handles to the parser/translator.
/// If handle creation fails, 0 will be returned.
class HlslCrossCompiler;
typedef HlslCrossCompiler* ShHandle;


/// Initialize the HLSL2GLSL translator.  This function must be called once prior to calling any other
/// HLSL2GLSL translator functions
/// \return
///   1 on success, 0 on failure
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_Initialize();


/// Finalize the HLSL2GLSL translator.  This function should be called to de-initialize the HLSL2GLSL 
/// translator and should only be called once on shutdown.
/// \return
///   1 on success, 0 on failure
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_Finalize();


/// Construct a compiler for the given language (one per shader)
SH_IMPORT_EXPORT ShHandle C_DECL Hlsl2Glsl_ConstructCompiler( const EShLanguage language );  


SH_IMPORT_EXPORT void C_DECL Hlsl2Glsl_DestructCompiler( ShHandle handle );



/// Parse HLSL shader to prepare it for final translation.
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_Parse( const ShHandle handle,
                                             const char* shaderString,
                                             int option );



/// After parsing a HLSL shader, do the final translation to GLSL.
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_Translate( const ShHandle handle, const char* entry );


/// After translating HLSL shader(s), retrieve the translated GLSL source.
SH_IMPORT_EXPORT const char* C_DECL Hlsl2Glsl_GetShader( const ShHandle handle );


SH_IMPORT_EXPORT const char* C_DECL Hlsl2Glsl_GetInfoLog( const ShHandle handle );


/// After translating, retrieve the number of uniforms
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_GetUniformCount( const ShHandle handle );


/// After translating, retrieve the uniform info table
SH_IMPORT_EXPORT const ShUniformInfo* C_DECL Hlsl2Glsl_GetUniformInfo( const ShHandle handle );


/// Instead of mapping HLSL attributes to GLSL fixed-function attributes, this function can be used to 
/// override the  attribute mapping.  This tells the code generator to use user-defined attributes for 
/// the semantics that are specified.
///
/// \param handle
///      Handle to the compiler.  This should be called BEFORE calling Hlsl2Glsl_Translate
/// \param pSemanticEnums 
///      Array of semantic enums to set
/// \param pSemanticNames 
///      Array of user attribute names to use
/// \param nNumSemantics 
///      Number of semantics to set in the arrays
/// \return
///      1 on success, 0 on failure
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_SetUserAttributeNames ( ShHandle handle, 
                                                              const EAttribSemantic *pSemanticEnums, 
                                                              const char *pSemanticNames[], 
                                                              int nNumSemantics );


/// Instead of using OpenGL fixed-function varyings (such as gl_TexCoord[x]), use user named varyings 
/// instead.
/// 
/// \param handle
///      Handle to the compiler.  This should be called BEFORE calling Hlsl2Glsl_Translate
/// \param bUseUserVarying 
///      If true, all user varyings will be used.  If false, the translator will attempt to use
///      GL fixed-function varyings
/// \return
///      1 on success, 0 on failure
SH_IMPORT_EXPORT int C_DECL Hlsl2Glsl_UseUserVaryings ( ShHandle handle, 
                                                        bool bUseUserVaryings );


#ifdef __cplusplus
}
#endif

#endif // _HLSL2GLSL_INTERFACE_INCLUDED_
