// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


// Implementation of support library to generate GLSL functions to support HLSL
// functions that don't map to built-ins

#include <map>
#include "hlslSupportLib.h"

typedef std::map<TOperator,std::string> CodeMap;
static CodeMap *hlslSupportLib = 0;
static CodeMap *hlslSupportLibESOverrides = 0;

typedef std::map< TOperator, std::pair<std::string,std::string> > CodeExtensionMap;
static CodeExtensionMap *hlslSupportLibExtensions = 0;
static CodeExtensionMap *hlslSupportLibExtensionsESOverrides = 0;

void insertPre130TextureLookups() 
{
    hlslSupportLib->insert( CodeMap::value_type( EOpTex1DBias,
        "vec4 xll_tex1Dbias(sampler1D s, vec4 coord) {\n"
        "  return texture1D( s, coord.x, coord.w);\n"
        "}\n\n" )
        );

    hlslSupportLib->insert( CodeMap::value_type( EOpTex1DLod,
        "vec4 xll_tex1Dlod(sampler1D s, vec4 coord) {\n"
        "  return texture1DLod( s, coord.x, coord.w);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTex1DLod, std::make_pair("","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTex1DGrad,
        "vec4 xll_tex1Dgrad(sampler1D s, float coord, float ddx, float ddy) {\n"
        "  return texture1DGradARB( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTex1DGrad, std::make_pair("GL_ARB_shader_texture_lod","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTex2DBias,
        "vec4 xll_tex2Dbias(sampler2D s, vec4 coord) {\n"
        "  return texture2D( s, coord.xy, coord.w);\n"
        "}\n\n" )
        );

    hlslSupportLib->insert( CodeMap::value_type( EOpTex2DLod,
        "vec4 xll_tex2Dlod(sampler2D s, vec4 coord) {\n"
        "   return texture2DLod( s, coord.xy, coord.w);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTex2DLod, std::make_pair("","GL_ARB_shader_texture_lod")));

    hlslSupportLibESOverrides->insert( CodeMap::value_type( EOpTex2DLod,
        "vec4 xll_tex2Dlod(sampler2D s, vec4 coord) {\n"
        "   return texture2DLodEXT( s, coord.xy, coord.w);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpTex2DLod, std::make_pair("","GL_EXT_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTex2DGrad,
        "vec4 xll_tex2Dgrad(sampler2D s, vec2 coord, vec2 ddx, vec2 ddy) {\n"
        "   return texture2DGradARB( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTex2DGrad, std::make_pair("GL_ARB_shader_texture_lod","GL_ARB_shader_texture_lod")));

    hlslSupportLibESOverrides->insert( CodeMap::value_type( EOpTex2DGrad,
        "vec4 xll_tex2Dgrad(sampler2D s, vec2 coord, vec2 ddx, vec2 ddy) {\n"
        "   return texture2DGradEXT( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpTex2DGrad, std::make_pair("GL_EXT_shader_texture_lod","GL_EXT_shader_texture_lod")));


    hlslSupportLib->insert( CodeMap::value_type( EOpTex3DBias,
        "vec4 xll_tex3Dbias(sampler3D s, vec4 coord) {\n"
        "  return texture3D( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );

    hlslSupportLib->insert( CodeMap::value_type( EOpTex3DLod,
        "vec4 xll_tex3Dlod(sampler3D s, vec4 coord) {\n"
        "  return texture3DLod( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTex3DLod, std::make_pair("","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTex3DGrad,
        "vec4 xll_tex3Dgrad(sampler3D s, vec3 coord, vec3 ddx, vec3 ddy) {\n"
        "  return texture3DGradARB( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTex3DGrad, std::make_pair("GL_ARB_shader_texture_lod","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeBias,   
        "vec4 xll_texCUBEbias(samplerCube s, vec4 coord) {\n"
        "  return textureCube( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );

    hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeLod,   
        "vec4 xll_texCUBElod(samplerCube s, vec4 coord) {\n"
        "  return textureCubeLod( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTexCubeLod, std::make_pair("","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeGrad,  
        "vec4 xll_texCUBEgrad(samplerCUBE s, vec3 coord, vec3 ddx, vec3 ddy) {\n"
        "  return textureCubeGradARB( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTexCubeGrad, std::make_pair("GL_ARB_shader_texture_lod","GL_ARB_shader_texture_lod")));

    // shadow2D / shadow2Dproj
    hlslSupportLib->insert(CodeMap::value_type(EOpShadow2D,
        "float xll_shadow2D(sampler2DShadow s, vec3 coord) { return shadow2D (s, coord).r; }\n"
        ));
    hlslSupportLibESOverrides->insert(CodeMap::value_type(EOpShadow2D,
        "float xll_shadow2D(sampler2DShadow s, vec3 coord) { return shadow2DEXT (s, coord); }\n"
        ));
    hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpShadow2D, std::make_pair("","GL_EXT_shadow_samplers")));

    hlslSupportLib->insert(CodeMap::value_type(EOpShadow2DProj,
        "float xll_shadow2Dproj(sampler2DShadow s, vec4 coord) { return shadow2DProj (s, coord).r; }\n"
        ));
    hlslSupportLibESOverrides->insert(CodeMap::value_type(EOpShadow2DProj,
        "float xll_shadow2Dproj(sampler2DShadow s, vec4 coord) { return shadow2DProjEXT (s, coord); }\n"
        ));
    hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpShadow2DProj, std::make_pair("","GL_EXT_shadow_samplers")));

}

void insertPost120TextureLookups() 
{
    hlslSupportLib->insert( CodeMap::value_type( EOpTex1DBias,
        "vec4 xll_tex1Dbias(sampler1D s, vec4 coord) {\n"
        "  return texture( s, coord.x, coord.w);\n"
        "}\n\n" )
        );

    hlslSupportLib->insert( CodeMap::value_type( EOpTex1DLod,
        "vec4 xll_tex1Dlod(sampler1D s, vec4 coord) {\n"
        "  return textureLod( s, coord.x, coord.w);\n"
        "}\n\n" )
        );
    hlslSupportLibExtensions->insert (std::make_pair(EOpTex1DLod, std::make_pair("","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTex1DGrad,
        "vec4 xll_tex1Dgrad(sampler1D s, float coord, float ddx, float ddy) {\n"
        "  return textureGrad( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensions->insert (std::make_pair(EOpTex1DGrad, std::make_pair("GL_ARB_shader_texture_lod","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTex2DBias,
        "vec4 xll_tex2Dbias(sampler2D s, vec4 coord) {\n"
        "  return texture( s, coord.xy, coord.w);\n"
        "}\n\n" )
        );

    hlslSupportLib->insert( CodeMap::value_type( EOpTex2DLod,
        "vec4 xll_tex2Dlod(sampler2D s, vec4 coord) {\n"
        "   return textureLod( s, coord.xy, coord.w);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensions->insert (std::make_pair(EOpTex2DLod, std::make_pair("","GL_ARB_shader_texture_lod")));

    hlslSupportLibESOverrides->insert( CodeMap::value_type( EOpTex2DLod,
        "vec4 xll_tex2Dlod(sampler2D s, vec4 coord) {\n"
        "   return textureLod( s, coord.xy, coord.w);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpTex2DLod, std::make_pair("","GL_EXT_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTex2DGrad,
        "vec4 xll_tex2Dgrad(sampler2D s, vec2 coord, vec2 ddx, vec2 ddy) {\n"
        "   return textureGrad( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensions->insert (std::make_pair(EOpTex2DGrad, std::make_pair("GL_ARB_shader_texture_lod","GL_ARB_shader_texture_lod")));

    hlslSupportLibESOverrides->insert( CodeMap::value_type( EOpTex2DGrad,
        "vec4 xll_tex2Dgrad(sampler2D s, vec2 coord, vec2 ddx, vec2 ddy) {\n"
        "   return textureGrad( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpTex2DGrad, std::make_pair("GL_EXT_shader_texture_lod","GL_EXT_shader_texture_lod")));


    hlslSupportLib->insert( CodeMap::value_type( EOpTex3DBias,
        "vec4 xll_tex3Dbias(sampler3D s, vec4 coord) {\n"
        "  return texture( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );

    hlslSupportLib->insert( CodeMap::value_type( EOpTex3DLod,
        "vec4 xll_tex3Dlod(sampler3D s, vec4 coord) {\n"
        "  return textureLod( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensions->insert (std::make_pair(EOpTex3DLod, std::make_pair("","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTex3DGrad,
        "vec4 xll_tex3Dgrad(sampler3D s, vec3 coord, vec3 ddx, vec3 ddy) {\n"
        "  return textureGrad( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensions->insert (std::make_pair(EOpTex3DGrad, std::make_pair("GL_ARB_shader_texture_lod","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeBias,   
        "vec4 xll_texCUBEbias(samplerCube s, vec4 coord) {\n"
        "  return texture( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );

    hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeLod,   
        "vec4 xll_texCUBElod(samplerCube s, vec4 coord) {\n"
        "  return textureLod( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensions->insert (std::make_pair(EOpTexCubeLod, std::make_pair("","GL_ARB_shader_texture_lod")));

    hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeGrad,  
        "vec4 xll_texCUBEgrad(samplerCUBE s, vec3 coord, vec3 ddx, vec3 ddy) {\n"
        "  return textureGrad( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
    //hlslSupportLibExtensions->insert (std::make_pair(EOpTexCubeGrad, std::make_pair("GL_ARB_shader_texture_lod","GL_ARB_shader_texture_lod")));

    // shadow2D / shadow2Dproj
    hlslSupportLib->insert(CodeMap::value_type(EOpShadow2D,
        "float xll_shadow2D(sampler2DShadow s, vec3 coord) { return texture (s, coord).r; }\n"
        ));
    hlslSupportLibESOverrides->insert(CodeMap::value_type(EOpShadow2D,
        "float xll_shadow2D(sampler2DShadow s, vec3 coord) { return texture (s, coord); }\n"
        ));
    hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpShadow2D, std::make_pair("","GL_EXT_shadow_samplers")));

    hlslSupportLib->insert(CodeMap::value_type(EOpShadow2DProj,
        "float xll_shadow2Dproj(sampler2DShadow s, vec4 coord) { return textureProj (s, coord).r; }\n"
        ));
    hlslSupportLibESOverrides->insert(CodeMap::value_type(EOpShadow2DProj,
        "float xll_shadow2Dproj(sampler2DShadow s, vec4 coord) { return textureProj (s, coord); }\n"
        ));
    hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpShadow2DProj, std::make_pair("","GL_EXT_shadow_samplers")));

}

void initializeHLSLSupportLibrary(ETargetVersion targetVersion) 
{
    assert (hlslSupportLib == 0);
    assert (hlslSupportLibExtensions == 0);
    assert (hlslSupportLibESOverrides == 0);
    assert (hlslSupportLibExtensionsESOverrides == 0);

    hlslSupportLib = new CodeMap();
    hlslSupportLibExtensions = new CodeExtensionMap();
    hlslSupportLibESOverrides = new CodeMap();
    hlslSupportLibExtensionsESOverrides = new CodeExtensionMap();

    //ACS: some texture lookup types were deprecated after 1.20, and 1.40 won't accept them
    bool usePost120TextureLookups = false;
    if(targetVersion!=ETargetVersionCount) //default
    {
        usePost120TextureLookups = (targetVersion>ETargetGLSL_120); 
    }

   // Initialize GLSL code for the op codes that require support helper functions

   hlslSupportLib->insert( CodeMap::value_type( EOpNull, ""));

   hlslSupportLib->insert( CodeMap::value_type( EOpAbs,
      "mat2 xll_abs(mat2 m) {\n"
      "  return mat2( abs(m[0]), abs(m[1]));\n"
      "}\n\n"
      "mat3 xll_abs(mat3 m) {\n"
      "  return mat3( abs(m[0]), abs(m[1]), abs(m[2]));\n"
      "}\n\n"
      "mat4 xll_abs(mat4 m) {\n"
      "  return mat4( abs(m[0]), abs(m[1]), abs(m[2]), abs(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpAcos,
      "mat2 xll_acos(mat2 m) {\n"
      "  return mat2( acos(m[0]), acos(m[1]));\n"
      "}\n\n"
      "mat3 xll_acos(mat3 m) {\n"
      "  return mat3( acos(m[0]), acos(m[1]), acos(m[2]));\n"
      "}\n\n"
      "mat4 xll_acos(mat4 m) {\n"
      "  return mat4( acos(m[0]), acos(m[1]), acos(m[2]), acos(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpCos,
      "mat2 xll_cos(mat2 m) {\n"
      "  return mat2( cos(m[0]), cos(m[1]));\n"
      "}\n\n"
      "mat3 xll_cos(mat3 m) {\n"
      "  return mat3( cos(m[0]), cos(m[1]), cos(m[2]));\n"
      "}\n\n"
      "mat4 xll_cos(mat4 m) {\n"
      "  return mat4( cos(m[0]), cos(m[1]), cos(m[2]), cos(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpAsin,
      "mat2 xll_asin(mat2 m) {\n"
      "  return mat2( asin(m[0]), asin(m[1]));\n"
      "}\n\n"
      "mat3 xll_asin(mat3 m) {\n"
      "  return mat3( asin(m[0]), asin(m[1]), asin(m[2]));\n"
      "}\n\n"
      "mat4 xll_asin(mat4 m) {\n"
      "  return mat4( asin(m[0]), asin(m[1]), asin(m[2]), asin(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpSin,
      "mat2 xll_sin(mat2 m) {\n"
      "  return mat2( sin(m[0]), sin(m[1]));\n"
      "}\n\n"
      "mat3 xll_sin(mat3 m) {\n"
      "  return mat3( sin(m[0]), sin(m[1]), sin(m[2]));\n"
      "}\n\n"
      "mat4 xll_sin(mat4 m) {\n"
      "  return mat4( sin(m[0]), sin(m[1]), sin(m[2]), sin(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpDPdx,
	  "float xll_dFdx(float f) {\n"
	  "  return dFdx(f);\n"
	  "}\n\n"
	  "vec2 xll_dFdx(vec2 v) {\n"
	  "  return dFdx(v);\n"
	  "}\n\n"
	  "vec3 xll_dFdx(vec3 v) {\n"
	  "  return dFdx(v);\n"
	  "}\n\n"
	  "vec4 xll_dFdx(vec4 v) {\n"
	  "  return dFdx(v);\n"
	  "}\n\n"
      "mat2 xll_dFdx(mat2 m) {\n"
      "  return mat2( dFdx(m[0]), dFdx(m[1]));\n"
      "}\n\n"
      "mat3 xll_dFdx(mat3 m) {\n"
      "  return mat3( dFdx(m[0]), dFdx(m[1]), dFdx(m[2]));\n"
      "}\n\n"
      "mat4 xll_dFdx(mat4 m) {\n"
      "  return mat4( dFdx(m[0]), dFdx(m[1]), dFdx(m[2]), dFdx(m[3]));\n"
      "}\n\n")
      );
	hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpDPdx, std::make_pair("","GL_OES_standard_derivatives")));

   hlslSupportLib->insert( CodeMap::value_type( EOpDPdy,
	  "float xll_dFdy(float f) {\n"
	  "  return dFdy(f);\n"
	  "}\n\n"
	  "vec2 xll_dFdy(vec2 v) {\n"
	  "  return dFdy(v);\n"
	  "}\n\n"
	  "vec3 xll_dFdy(vec3 v) {\n"
	  "  return dFdy(v);\n"
	  "}\n\n"
	  "vec4 xll_dFdy(vec4 v) {\n"
	  "  return dFdy(v);\n"
	  "}\n\n"
      "mat2 xll_dFdy(mat2 m) {\n"
      "  return mat2( dFdy(m[0]), dFdy(m[1]));\n"
      "}\n\n"
      "mat3 xll_dFdy(mat3 m) {\n"
      "  return mat3( dFdy(m[0]), dFdy(m[1]), dFdy(m[2]));\n"
      "}\n\n"
      "mat4 xll_dFdy(mat4 m) {\n"
      "  return mat4( dFdy(m[0]), dFdy(m[1]), dFdy(m[2]), dFdy(m[3]));\n"
      "}\n\n")
      );
	hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpDPdy, std::make_pair("","GL_OES_standard_derivatives")));

   hlslSupportLib->insert( CodeMap::value_type( EOpExp,
      "mat2 xll_exp(mat2 m) {\n"
      "  return mat2( exp(m[0]), exp(m[1]));\n"
      "}\n\n"
      "mat3 xll_exp(mat3 m) {\n"
      "  return mat3( exp(m[0]), exp(m[1]), exp(m[2]));\n"
      "}\n\n"
      "mat4 xll_exp(mat4 m) {\n"
      "  return mat4( exp(m[0]), exp(m[1]), exp(m[2]), exp(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpExp2,
      "mat2 xll_exp2(mat2 m) {\n"
      "  return mat2( exp2(m[0]), exp2(m[1]));\n"
      "}\n\n"
      "mat3 xll_exp2(mat3 m) {\n"
      "  return mat3( exp2(m[0]), exp2(m[1]), exp2(m[2]));\n"
      "}\n\n"
      "mat4 xll_exp2(mat4 m) {\n"
      "  return mat4( exp2(m[0]), exp2(m[1]), exp2(m[2]), exp2(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpLog,
      "mat2 xll_log(mat2 m) {\n"
      "  return mat2( log(m[0]), log(m[1]));\n"
      "}\n\n"
      "mat3 xll_log(mat3 m) {\n"
      "  return mat3( log(m[0]), log(m[1]), log(m[2]));\n"
      "}\n\n"
      "mat4 xll_log(mat4 m) {\n"
      "  return mat4( log(m[0]), log(m[1]), log(m[2]), log(m[3]));\n"
      "}\n\n")
      );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpLog2,
      "mat2 xll_log2(mat2 m) {\n"
      "  return mat2( log2(m[0]), log2(m[1]));\n"
      "}\n\n"
      "mat3 xll_log2(mat3 m) {\n"
      "  return mat3( log2(m[0]), log2(m[1]), log2(m[2]));\n"
      "}\n\n"
      "mat4 xll_log2(mat4 m) {\n"
      "  return mat4( log2(m[0]), log2(m[1]), log2(m[2]), log2(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpTan,
      "mat2 xll_tan(mat2 m) {\n"
      "  return mat2( tan(m[0]), tan(m[1]));\n"
      "}\n\n"
      "mat3 xll_tan(mat3 m) {\n"
      "  return mat3( tan(m[0]), tan(m[1]), tan(m[2]));\n"
      "}\n\n"
      "mat4 xll_tan(mat4 m) {\n"
      "  return mat4( tan(m[0]), tan(m[1]), tan(m[2]), tan(m[3]));\n"
      "}\n\n")
      );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpAtan,
      "mat2 xll_atan(mat2 m) {\n"
      "  return mat2( atan(m[0]), atan(m[1]));\n"
      "}\n\n"
      "mat3 xll_atan(mat3 m) {\n"
      "  return mat3( atan(m[0]), atan(m[1]), atan(m[2]));\n"
      "}\n\n"
      "mat4 xll_atan(mat4 m) {\n"
      "  return mat4( atan(m[0]), atan(m[1]), atan(m[2]), atan(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpDegrees,
      "mat2 xll_degrees(mat2 m) {\n"
      "  return mat2( degrees(m[0]), degrees(m[1]));\n"
      "}\n\n"
      "mat3 xll_degrees(mat3 m) {\n"
      "  return mat3( degrees(m[0]), degrees(m[1]), degrees(m[2]));\n"
      "}\n\n"
      "mat4 xll_degrees(mat4 m) {\n"
      "  return mat4( degrees(m[0]), degrees(m[1]), degrees(m[2]), degrees(m[3]));\n"
      "}\n\n")
      );

    hlslSupportLib->insert( CodeMap::value_type( EOpRadians,
      "mat2 xll_radians(mat2 m) {\n"
      "  return mat2( radians(m[0]), radians(m[1]));\n"
      "}\n\n"
      "mat3 xll_radians(mat3 m) {\n"
      "  return mat3( radians(m[0]), radians(m[1]), radians(m[2]));\n"
      "}\n\n"
      "mat4 xll_radians(mat4 m) {\n"
      "  return mat4( radians(m[0]), radians(m[1]), radians(m[2]), radians(m[3]));\n"
      "}\n\n")
      );

    hlslSupportLib->insert( CodeMap::value_type( EOpSqrt,
      "mat2 xll_sqrt(mat2 m) {\n"
      "  return mat2( sqrt(m[0]), sqrt(m[1]));\n"
      "}\n\n"
      "mat3 xll_sqrt(mat3 m) {\n"
      "  return mat3( sqrt(m[0]), sqrt(m[1]), sqrt(m[2]));\n"
      "}\n\n"
      "mat4 xll_sqrt(mat4 m) {\n"
      "  return mat4( sqrt(m[0]), sqrt(m[1]), sqrt(m[2]), sqrt(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpInverseSqrt,
      "mat2 xll_inversesqrt(mat2 m) {\n"
      "  return mat2( inversesqrt(m[0]), inversesqrt(m[1]));\n"
      "}\n\n"
      "mat3 xll_inversesqrt(mat3 m) {\n"
      "  return mat3( inversesqrt(m[0]), inversesqrt(m[1]), inversesqrt(m[2]));\n"
      "}\n\n"
      "mat4 xll_inversesqrt(mat4 m) {\n"
      "  return mat4( inversesqrt(m[0]), inversesqrt(m[1]), inversesqrt(m[2]), inversesqrt(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpFloor,
      "mat2 xll_floor(mat2 m) {\n"
      "  return mat2( floor(m[0]), floor(m[1]));\n"
      "}\n\n"
      "mat3 xll_floor(mat3 m) {\n"
      "  return mat3( floor(m[0]), floor(m[1]), floor(m[2]));\n"
      "}\n\n"
      "mat4 xll_floor(mat4 m) {\n"
      "  return mat4( floor(m[0]), floor(m[1]), floor(m[2]), floor(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpSign,
      "mat2 xll_sign(mat2 m) {\n"
      "  return mat2( sign(m[0]), sign(m[1]));\n"
      "}\n\n"
      "mat3 xll_sign(mat3 m) {\n"
      "  return mat3( sign(m[0]), sign(m[1]), sign(m[2]));\n"
      "}\n\n"
      "mat4 xll_sign(mat4 m) {\n"
      "  return mat4( sign(m[0]), sign(m[1]), sign(m[2]), sign(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpCeil,
      "mat2 xll_ceil(mat2 m) {\n"
      "  return mat2( ceil(m[0]), ceil(m[1]));\n"
      "}\n\n"
      "mat3 xll_ceil(mat3 m) {\n"
      "  return mat3( ceil(m[0]), ceil(m[1]), ceil(m[2]));\n"
      "}\n\n"
      "mat4 xll_ceil(mat4 m) {\n"
      "  return mat4( ceil(m[0]), ceil(m[1]), ceil(m[2]), ceil(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpFract,
      "mat2 xll_fract(mat2 m) {\n"
      "  return mat2( fract(m[0]), fract(m[1]));\n"
      "}\n\n"
      "mat3 xll_fract(mat3 m) {\n"
      "  return mat3( fract(m[0]), fract(m[1]), fract(m[2]));\n"
      "}\n\n"
      "mat4 xll_fract(mat4 m) {\n"
      "  return mat4( fract(m[0]), fract(m[1]), fract(m[2]), fract(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpFwidth,
	  "float xll_fwidth(float f) {\n"
	  "  return fwidth(f);\n"
	  "}\n\n"
	  "vec2 xll_fwidth(vec2 v) {\n"
	  "  return fwidth(v);\n"
	  "}\n\n"
	  "vec3 xll_fwidth(vec3 v) {\n"
	  "  return fwidth(v);\n"
	  "}\n\n"
	  "vec4 xll_fwidth(vec4 v) {\n"
	  "  return fwidth(v);\n"
	  "}\n\n"
      "mat2 xll_fwidth(mat2 m) {\n"
      "  return mat2( fwidth(m[0]), fwidth(m[1]));\n"
      "}\n\n"
      "mat3 xll_fwidth(mat3 m) {\n"
      "  return mat3( fwidth(m[0]), fwidth(m[1]), fwidth(m[2]));\n"
      "}\n\n"
      "mat4 xll_fwidth(mat4 m) {\n"
      "  return mat4( fwidth(m[0]), fwidth(m[1]), fwidth(m[2]), fwidth(m[3]));\n"
      "}\n\n")
      );
    hlslSupportLibExtensionsESOverrides->insert (std::make_pair(EOpFwidth, std::make_pair("","GL_OES_standard_derivatives")));

   hlslSupportLib->insert( CodeMap::value_type( EOpFclip,
	   "void xll_clip(float x) {\n"
	   "  if ( x<0.0 ) discard;\n"
	   "}\n")
	   );

   hlslSupportLib->insert( CodeMap::value_type( EOpPow,
      "mat2 xll_pow(mat2 m, mat2 y) {\n"
      "  return mat2( pow(m[0],y[0]), pow(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xll_pow(mat3 m, mat3 y) {\n"
      "  return mat3( pow(m[0],y[0]), pow(m[1],y[1]), pow(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xll_pow(mat4 m, mat4 y) {\n"
      "  return mat4( pow(m[0],y[0]), pow(m[1],y[1]), pow(m[2],y[2]), pow(m[3],y[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpAtan2,
      "mat2 xll_atan2(mat2 m, mat2 y) {\n"
      "  return mat2( atan(m[0],y[0]), atan(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xll_atan2(mat3 m, mat3 y) {\n"
      "  return mat3( atan(m[0],y[0]), atan(m[1],y[1]), atan(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xll_atan2(mat4 m, mat4 y) {\n"
      "  return mat4( atan(m[0],y[0]), atan(m[1],y[1]), atan(m[2],y[2]), atan(m[3],y[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpMin,
      "mat2 xll_min(mat2 m, mat2 y) {\n"
      "  return mat2( min(m[0],y[0]), min(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xll_min(mat3 m, mat3 y) {\n"
      "  return mat3( min(m[0],y[0]), min(m[1],y[1]), min(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xll_min(mat4 m, mat4 y) {\n"
      "  return mat4( min(m[0],y[0]), min(m[1],y[1]), min(m[2],y[2]), min(m[3],y[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpMax,
      "mat2 xll_max(mat2 m, mat2 y) {\n"
      "  return mat2( max(m[0],y[0]), max(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xll_max(mat3 m, mat3 y) {\n"
      "  return mat3( max(m[0],y[0]), max(m[1],y[1]), max(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xll_max(mat4 m, mat4 y) {\n"
      "  return mat4( max(m[0],y[0]), max(m[1],y[1]), max(m[2],y[2]), max(m[3],y[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpTranspose,
        "mat2 xll_transpose(mat2 m) {\n"
        "  return mat2( m[0][0], m[1][0], m[0][1], m[1][1]);\n"
        "}\n\n"
        "mat3 xll_transpose(mat3 m) {\n"
        "  return mat3( m[0][0], m[1][0], m[2][0],\n"
        "               m[0][1], m[1][1], m[2][1],\n"
        "               m[0][2], m[1][2], m[2][2]);\n"
        "}\n\n"
        "mat4 xll_transpose(mat4 m) {\n"
        "  return mat4( m[0][0], m[1][0], m[2][0], m[3][0],\n"
        "               m[0][1], m[1][1], m[2][1], m[3][1],\n"
        "               m[0][2], m[1][2], m[2][2], m[3][2],\n"
        "               m[0][3], m[1][3], m[2][3], m[3][3]);\n"
        "}\n")
        );

	// Note: constructing temporary vector and assigning individual components; seems to avoid
	// some GLSL bugs on AMD (Win7, Radeon HD 58xx, Catalyst 10.5).
	hlslSupportLib->insert( CodeMap::value_type( EOpMatrixIndex,
		"vec2 xll_matrixindex (mat2 m, int i) { vec2 v; v.x=m[0][i]; v.y=m[1][i]; return v; }\n"
		"vec3 xll_matrixindex (mat3 m, int i) { vec3 v; v.x=m[0][i]; v.y=m[1][i]; v.z=m[2][i]; return v; }\n"
		"vec4 xll_matrixindex (mat4 m, int i) { vec4 v; v.x=m[0][i]; v.y=m[1][i]; v.z=m[2][i]; v.w=m[3][i]; return v; }\n")
		);
	
	// The GLSL ES implementation on NaCl does not support dynamic indexing 
	// (except when the operand is a uniform in vertex shaders). The GLSL specification 
	// leaves it open to vendors to support this or not. So, for NaCl we use if statements to 
	// simulate the indexing.
	hlslSupportLib->insert( CodeMap::value_type( EOpMatrixIndexDynamic,
		"#if defined(SHADER_API_GLES) && defined(SHADER_API_DESKTOP)\n"
		"vec2 xll_matrixindexdynamic (mat2 m, int i) {\n"
		"	mat2 m2 = xll_transpose(m);\n"
		"	return i==0?m2[0]:m2[1];\n"
		"}\n"
		"vec3 xll_matrixindexdynamic (mat3 m, int i) {\n"
		"	mat3 m2 = xll_transpose(m);\n"
		"	return i < 2?(i==0?m2[0]:m2[1]):(m2[2]);\n"
		"}\n"
		"vec4 xll_matrixindexdynamic (mat4 m, int i) {\n"
		"	mat4 m2 = xll_transpose(m);\n"
		"	return i < 2?(i==0?m2[0]:m2[1]):(i==3?m2[3]:m2[2]);\n"
		"}\n"
		"#else\n"
		"vec2 xll_matrixindexdynamic (mat2 m, int i) { return xll_matrixindex (m, i); }\n"
		"vec3 xll_matrixindexdynamic (mat3 m, int i) { return xll_matrixindex (m, i); }\n"
		"vec4 xll_matrixindexdynamic (mat4 m, int i) { return xll_matrixindex (m, i); }\n"
		"#endif\n")
		);


	// Used in pre-GLSL 1.20
	hlslSupportLib->insert( CodeMap::value_type( EOpConstructMat2x2FromMat,
		"mat2 xll_constructMat2( mat3 m) {\n"
		"  return mat2( vec2( m[0]), vec2( m[1]));\n"
		"}\n\n"
		"mat2 xll_constructMat2( mat4 m) {\n"
		"  return mat2( vec2( m[0]), vec2( m[1]));\n"
		"}\n")
		);
	hlslSupportLib->insert( CodeMap::value_type( EOpConstructMat3x3FromMat,
		"mat3 xll_constructMat3( mat4 m) {\n"
		"  return mat3( vec3( m[0]), vec3( m[1]), vec3( m[2]));\n"
		"}\n")
		);

   hlslSupportLib->insert( CodeMap::value_type( EOpDeterminant,
        "float xll_determinant( mat2 m) {\n"
        "    return m[0][0]*m[1][1] - m[0][1]*m[1][0];\n"
        "}\n\n"
        "float xll_determinant( mat3 m) {\n"
        "    vec3 temp;\n"    
        "    temp.x = m[1][1]*m[2][2] - m[1][2]*m[2][1];\n"
        "    temp.y = - (m[0][1]*m[2][2] - m[0][2]*m[2][1]);\n"
        "    temp.z = m[0][1]*m[1][2] - m[0][2]*m[1][1];\n"
        "    return dot( m[0], temp);\n"
        "}\n\n"
        "float xll_determinant( mat4 m) {\n"
        "    vec4 temp;\n"
        "    temp.x = xll_determinant( mat3( m[1].yzw, m[2].yzw, m[3].yzw));\n"
        "    temp.y = -xll_determinant( mat3( m[0].yzw, m[2].yzw, m[3].yzw));\n"
        "    temp.z = xll_determinant( mat3( m[0].yzw, m[1].yzw, m[3].yzw));\n"
        "    temp.w = -xll_determinant( mat3( m[0].yzw, m[1].yzw, m[2].yzw));\n"    
        "    return dot( m[0], temp);\n"
        "}\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpSaturate,
        "float xll_saturate( float x) {\n"
        "  return clamp( x, 0.0, 1.0);\n"
        "}\n\n"
        "vec2 xll_saturate( vec2 x) {\n"
        "  return clamp( x, 0.0, 1.0);\n"
        "}\n\n"
        "vec3 xll_saturate( vec3 x) {\n"
        "  return clamp( x, 0.0, 1.0);\n"
        "}\n\n"
        "vec4 xll_saturate( vec4 x) {\n"
        "  return clamp( x, 0.0, 1.0);\n"
        "}\n\n"
        "mat2 xll_saturate(mat2 m) {\n"
        "  return mat2( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0));\n"
        "}\n\n"
        "mat3 xll_saturate(mat3 m) {\n"
        "  return mat3( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0), clamp(m[2], 0.0, 1.0));\n"
        "}\n\n"
        "mat4 xll_saturate(mat4 m) {\n"
        "  return mat4( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0), clamp(m[2], 0.0, 1.0), clamp(m[3], 0.0, 1.0));\n"
        "}\n\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpMod,
	   "float xll_mod( float x, float y ) {\n"
	   "  float d = x / y;\n"
	   "  float f = fract (abs(d)) * y;\n"
	   "  return d >= 0.0 ? f : -f;\n"
	   "}\n\n"
	   "vec2 xll_mod( vec2 x, vec2 y ) {\n"
	   "  vec2 d = x / y;\n"
	   "  vec2 f = fract (abs(d)) * y;\n"
	   "  return vec2 (d.x >= 0.0 ? f.x : -f.x, d.y >= 0.0 ? f.y : -f.y);\n"
	   "}\n\n"
	   "vec3 xll_mod( vec3 x, vec3 y ) {\n"
	   "  vec3 d = x / y;\n"
	   "  vec3 f = fract (abs(d)) * y;\n"
	   "  return vec3 (d.x >= 0.0 ? f.x : -f.x, d.y >= 0.0 ? f.y : -f.y, d.z >= 0.0 ? f.z : -f.z);\n"
	   "}\n\n"
	   "vec4 xll_mod( vec4 x, vec4 y ) {\n"
	   "  vec4 d = x / y;\n"
	   "  vec4 f = fract (abs(d)) * y;\n"
	   "  return vec4 (d.x >= 0.0 ? f.x : -f.x, d.y >= 0.0 ? f.y : -f.y, d.z >= 0.0 ? f.z : -f.z, d.w >= 0.0 ? f.w : -f.w);\n"
	   "}\n\n"
	   )
	   );

   hlslSupportLib->insert( CodeMap::value_type( EOpModf,
        "float xll_modf( float x, out int ip) {\n"
		"  ip = int (x);\n"
		"  return x-float(ip);\n"
        "}\n\n"
        "float xll_modf( float x, out float ip) {\n"
		"  int i = int (x);\n"
		"  ip = float(i);\n"
		"  return x-ip;\n"
        "}\n\n"
		"vec2 xll_modf( vec2 x, out ivec2 ip) {\n"
		"  ip = ivec2 (x);\n"
		"  return x-vec2(ip);\n"
		"}\n\n"
		"vec2 xll_modf( vec2 x, out vec2 ip) {\n"
		"  ivec2 i = ivec2 (x);\n"
		"  ip = vec2(i);\n"
		"  return x-ip;\n"
		"}\n\n"
		"vec3 xll_modf( vec3 x, out ivec3 ip) {\n"
		"  ip = ivec3 (x);\n"
		"  return x-vec3(ip);\n"
		"}\n\n"
		"vec3 xll_modf( vec3 x, out vec3 ip) {\n"
		"  ivec3 i = ivec3 (x);\n"
		"  ip = vec3(i);\n"
		"  return x-ip;\n"
		"}\n\n"
		"vec4 xll_modf( vec4 x, out ivec4 ip) {\n"
		"  ip = ivec4 (x);\n"
		"  return x-vec4(ip);\n"
		"}\n\n"
		"vec4 xll_modf( vec4 x, out vec4 ip) {\n"
		"  ivec4 i = ivec4 (x);\n"
		"  ip = vec4(i);\n"
		"  return x-ip;\n"
		"}\n\n"
		)
        );
	
	hlslSupportLib->insert (CodeMap::value_type(EOpRound,
		"float xll_round (float x) { return floor (x+0.5); }\n"
		"vec2 xll_round (vec2 x) { return floor (x+vec2(0.5)); }\n"
		"vec3 xll_round (vec3 x) { return floor (x+vec3(0.5)); }\n"
		"vec4 xll_round (vec4 x) { return floor (x+vec4(0.5)); }\n"
	));
	
	hlslSupportLib->insert (CodeMap::value_type(EOpTrunc,
		"float xll_trunc (float x) { return x < 0.0 ? -floor(-x) : floor(x); }\n"
		"vec2 xll_trunc (vec2 v) { return vec2(\n"
		"  v.x < 0.0 ? -floor(-v.x) : floor(v.x),\n"
		"  v.y < 0.0 ? -floor(-v.y) : floor(v.y)\n"
		"); }\n"
		"vec3 xll_trunc (vec3 v) { return vec3(\n"
		"  v.x < 0.0 ? -floor(-v.x) : floor(v.x),\n"
		"  v.y < 0.0 ? -floor(-v.y) : floor(v.y),\n"
		"  v.z < 0.0 ? -floor(-v.z) : floor(v.z)\n"
		"); }\n"
		"vec4 xll_trunc (vec4 v) { return vec4(\n"
		"  v.x < 0.0 ? -floor(-v.x) : floor(v.x),\n"
		"  v.y < 0.0 ? -floor(-v.y) : floor(v.y),\n"
		"  v.z < 0.0 ? -floor(-v.z) : floor(v.z),\n"
		"  v.w < 0.0 ? -floor(-v.w) : floor(v.w)\n"
		"); }\n"
	));
	
   hlslSupportLib->insert( CodeMap::value_type( EOpLdexp,
        "float xll_ldexp( float x, float expon) {\n"
        "  return x * exp2 ( expon );\n"
        "}\n\n"
        "float2 xll_ldexp( vec2 x, vec2 expon) {\n"
        "  return x * exp2 ( expon );\n"
        "}\n\n"
        "float3 xll_ldexp( vec3 x, vec3 expon) {\n"
        "  return x * exp2 ( expon );\n"
        "}\n\n"
        "float4 xll_ldexp( vec4 x, vec4 expon) {\n"
        "  return x * exp2 ( expon );\n"
        "}\n\n"
        "float2x2 xll_ldexp( mat2 x, mat2 expon) {\n"
        "  return x * mat2( exp2 ( expon[0] ), exp2 ( expon[1] ) );\n"
        "}\n\n"
        "float3x3 xll_ldexp( mat3 x, mat3 expon) {\n"
        "  return x * mat3( exp2 ( expon[0] ), exp2 ( expon[1] ), exp2 ( expon[2] ) );\n"
        "}\n\n"
        "float4x4 xll_ldexp( mat4 x, mat4 expon) {\n"
        "  return x * mat4( exp2 ( expon[0] ), exp2 ( expon[1] ), exp2 ( expon[2] ), exp2 ( expon[3] ) );\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpSinCos,
        "void xll_sincos( float x, out float s, out float c) {\n"
        "  s = sin(x); \n"
        "  c = cos(x); \n"
        "}\n\n"
        "void xll_sincos( vec2 x, out vec2 s, out vec2 c) {\n"
        "  s = sin(x); \n"
        "  c = cos(x); \n"
        "}\n\n"        
        "void xll_sincos( vec3 x, out vec3 s, out vec3 c) {\n"
        "  s = sin(x); \n"
        "  c = cos(x); \n"
        "}\n\n"        
        "void xll_sincos( vec4 x, out vec4 s, out vec4 c) {\n"
        "  s = sin(x); \n"
        "  c = cos(x); \n"
        "}\n\n"        
        "void xll_sincos( mat2 x, out mat2 s, out mat2 c) {\n"
        "  s = mat2( sin ( x[0] ), sin ( x[1] ) ); \n"
        "  c = mat2( cos ( x[0] ), cos ( x[1] ) ); \n"
        "}\n\n"        
        "void xll_sincos( mat3 x, out mat3 s, out mat3 c) {\n"
        "  s = mat3( sin ( x[0] ), sin ( x[1] ), sin ( x[2] ) ); \n"
        "  c = mat3( cos ( x[0] ), cos ( x[1] ), cos ( x[2] ) ); \n"
        "}\n\n"        
        "void xll_sincos( mat4 x, out mat4 s, out mat4 c) {\n"
        "  s = mat4( sin ( x[0] ), sin ( x[1] ), sin ( x[2] ), sin ( x[3] ) ); \n"
        "  c = mat4( cos ( x[0] ), cos ( x[1] ), cos ( x[2] ), cos ( x[3] ) ); \n"
        "}\n\n" )
        );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpLog10,
        "float xll_log10( float x ) {\n"
        "  return log2 ( x ) / 3.32192809; \n"
        "}\n\n"
        "vec2 xll_log10( vec2 x ) {\n"
        "  return log2 ( x ) / vec2 ( 3.32192809 ); \n"
        "}\n\n"
        "vec3 xll_log10( vec3 x ) {\n"
        "  return log2 ( x ) / vec3 ( 3.32192809 ); \n"
        "}\n\n"
        "vec4 xll_log10( vec4 x ) {\n"
        "  return log2 ( x ) / vec4 ( 3.32192809 ); \n"
        "}\n\n"
        "mat2 xll_log10(mat2 m) {\n"
        "  return mat2( xll_log10(m[0]), xll_log10(m[1]));\n"
        "}\n\n"
        "mat3 xll_log10(mat3 m) {\n"
        "  return mat3( xll_log10(m[0]), xll_log10(m[1]), xll_log10(m[2]));\n"
        "}\n\n"
        "mat4 xll_log10(mat4 m) {\n"
        "  return mat4( xll_log10(m[0]), xll_log10(m[1]), xll_log10(m[2]), xll_log10(m[3]));\n"
        "}\n\n")
        );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpMix,
        "mat2 xll_mix( mat2 x, mat2 y, mat2 s ) {\n"
        "  return mat2( mix(x[0],y[0],s[0]), mix(x[1],y[1],s[1]) ); \n"
        "}\n\n"
        "mat3 xll_mix( mat3 x, mat3 y, mat3 s ) {\n"
        "  return mat3( mix(x[0],y[0],s[0]), mix(x[1],y[1],s[1]), mix(x[2],y[2],s[2]) ); \n"
        "}\n\n"
        "mat4 xll_mix( mat4 x, mat4 y, mat4 s ) {\n"
        "  return mat4( mix(x[0],y[0],s[0]), mix(x[1],y[1],s[1]), mix(x[2],y[2],s[2]), mix(x[3],y[3],s[3]) ); \n"
        "}\n\n")      
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpLit,
        "vec4 xll_lit( float n_dot_l, float n_dot_h, float m ) {\n"
        "   return vec4(1, max(0.0, n_dot_l), pow(max(0.0, n_dot_h) * step(0.0, n_dot_l), m), 1.0);\n"
        "}\n\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpSmoothStep,
        "mat2 xll_smoothstep( mat2 x, mat2 y, mat2 s ) {\n"
        "  return mat2( smoothstep(x[0],y[0],s[0]), smoothstep(x[1],y[1],s[1]) ); \n"
        "}\n\n"
        "mat3 xll_smoothstep( mat3 x, mat3 y, mat3 s ) {\n"
        "  return mat3( smoothstep(x[0],y[0],s[0]), smoothstep(x[1],y[1],s[1]), smoothstep(x[2],y[2],s[2]) ); \n"
        "}\n\n"
        "mat4 xll_smoothstep( mat4 x, mat4 y, mat4 s ) {\n"
        "  return mat4( smoothstep(x[0],y[0],s[0]), smoothstep(x[1],y[1],s[1]), smoothstep(x[2],y[2],s[2]), smoothstep(x[3],y[3],s[3]) ); \n"
        "}\n\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpClamp,
        "mat2 xll_clamp( mat2 x, mat2 y, mat2 s ) {\n"
        "  return mat2( clamp(x[0],y[0],s[0]), clamp(x[1],y[1],s[1]) ); \n"
        "}\n\n"
        "mat3 xll_clamp( mat3 x, mat3 y, mat3 s ) {\n"
        "  return mat3( clamp(x[0],y[0],s[0]), clamp(x[1],y[1],s[1]), clamp(x[2],y[2],s[2]) ); \n"
        "}\n\n"
        "mat4 xll_clamp( mat4 x, mat4 y, mat4 s ) {\n"
        "  return mat4( clamp(x[0],y[0],s[0]), clamp(x[1],y[1],s[1]), clamp(x[2],y[2],s[2]), clamp(x[3],y[3],s[3]) ); \n"
        "}\n\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpStep,
      "mat2 xll_step(mat2 m, mat2 y) {\n"
      "  return mat2( step(m[0],y[0]), step(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xll_step(mat3 m, mat3 y) {\n"
      "  return mat3( step(m[0],y[0]), step(m[1],y[1]), step(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xll_step(mat4 m, mat4 y) {\n"
      "  return mat4( step(m[0],y[0]), step(m[1],y[1]), step(m[2],y[2]), step(m[3],y[3]));\n"
      "}\n\n")
      );
   
   //ACS: if we're post-1.20 use the newer, non-deprecated, texture lookups
   if(usePost120TextureLookups) {
       insertPost120TextureLookups();
   }
   else {
       insertPre130TextureLookups();
   }

   hlslSupportLib->insert( CodeMap::value_type( EOpD3DCOLORtoUBYTE4,  
        "ivec4 xll_D3DCOLORtoUBYTE4(vec4 x) {\n"
        "  return ivec4 ( x.zyxw * 255.001953 );\n"
        "}\n\n" )
        );
	
	hlslSupportLib->insert( CodeMap::value_type( EOpVecTernarySel,  
		"vec2 xll_vecTSel (bvec2 a, vec2 b, vec2 c) {\n"
		"  return vec2 (a.x ? b.x : c.x, a.y ? b.y : c.y);\n"
		"}\n"
		"vec3 xll_vecTSel (bvec3 a, vec3 b, vec3 c) {\n"
		"  return vec3 (a.x ? b.x : c.x, a.y ? b.y : c.y, a.z ? b.z : c.z);\n"
		"}\n"
		"vec4 xll_vecTSel (bvec4 a, vec4 b, vec4 c) {\n"
		"  return vec4 (a.x ? b.x : c.x, a.y ? b.y : c.y, a.z ? b.z : c.z, a.w ? b.w : c.w);\n"
		"}\n\n"
		)
   );	
}


void finalizeHLSLSupportLibrary() 
{
	delete hlslSupportLib;
	hlslSupportLib = 0;
	delete hlslSupportLibExtensions;
	hlslSupportLibExtensions = 0;
	delete hlslSupportLibESOverrides;
	hlslSupportLibESOverrides = 0;
	delete hlslSupportLibExtensionsESOverrides;
	hlslSupportLibExtensionsESOverrides = 0;
}

std::string getHLSLSupportCode (TOperator op, ExtensionSet& extensions, bool vertexShader, bool gles)
{
	assert (hlslSupportLibExtensions);
	assert (hlslSupportLibExtensionsESOverrides);

	// if we're using gles, attempt to find the ES version first
	bool found = false;
	if (gles)
	{
		CodeExtensionMap::iterator eit = hlslSupportLibExtensionsESOverrides->find(op);
		if (eit != hlslSupportLibExtensionsESOverrides->end())
		{
			const std::string& ext = vertexShader ? eit->second.first : eit->second.second;
			if (!ext.empty())
				extensions.insert(ext);
			found = true;
		}
	}

	if (!found)
	{
		CodeExtensionMap::iterator eit = hlslSupportLibExtensions->find(op);
		if (eit != hlslSupportLibExtensions->end())
		{
			const std::string& ext = vertexShader ? eit->second.first : eit->second.second;
			if (!ext.empty())
				extensions.insert(ext);
		}
	}
	
	assert (hlslSupportLib);
	assert (hlslSupportLibESOverrides);

	// same as above, search for a gles version first
	bool tex2DLodVSHack = false;
	if (vertexShader && op == EOpTex2DLod)
		tex2DLodVSHack = true;
	if (gles && !tex2DLodVSHack)
	{
		CodeMap::iterator it = hlslSupportLibESOverrides->find(op);
		if (it != hlslSupportLibESOverrides->end())
			return it->second;
	}

	CodeMap::iterator it = hlslSupportLib->find(op);
	if (it == hlslSupportLib->end())
		it = hlslSupportLib->find(EOpNull); // this always exists

	return it->second;
}
