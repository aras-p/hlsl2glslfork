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
//   Implementation of HLSLTranslator class
//=================================================================================================================================

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================

#include "stdafx.h"
#include "HLSLTranslator.h"

//=================================================================================================================================
//
//          Constructor(s) / Destructor(s) Block 
//
//=================================================================================================================================

//=================================================================================================================================
/// Constructor
//=================================================================================================================================
HLSLTranslator::HLSLTranslator()
{
}

//=================================================================================================================================
/// Destructor
//=================================================================================================================================
HLSLTranslator::~HLSLTranslator()
{
}

//=================================================================================================================================
//
//          Public methods block
//
//=================================================================================================================================

//=================================================================================================================================
/// Translate an HLSL shader to GLSL
//=================================================================================================================================
bool HLSLTranslator::Translate ( const CString& rHlslSource, const CString& rMainFunction, const CString& rShaderType,
                                 CString& rGlslSource, CString& rErrorString, bool bUseUserVaryings, bool bUseDefaultPrecision,
                                 int nNumSemantics, const EAttribSemantic *pAttribSemantics, const char* pAttribNames[] )
{
   ShHandle hTranslator = 0;
   ShHandle hParser = 0;
   EShLanguage eShaderLang;
    
   Hlsl2Glsl_Initialize ();

   // Construct a parser based on the language type
   if ( rShaderType == "Vertex" )
   {
      eShaderLang = EShLangVertex;      
   }
   else
   {
      eShaderLang = EShLangFragment;      
   }
   

   hParser = Hlsl2Glsl_ConstructParser ( eShaderLang, 0 );
   if ( hParser == 0 )
   {
      rErrorString =  "Error creating compiler for HLSL to GLSL translation";
      return false;
   }


   // Parse the HLSL source
   const char* shaderStrings[1] = { rHlslSource };         
   if ( !Hlsl2Glsl_Parse( hParser, shaderStrings, 1, 0) )
   {
      rErrorString = Hlsl2Glsl_GetInfoLog ( hParser ) ;
      Hlsl2Glsl_Destruct ( hParser );
      return false;
   }
   
   // Now construct a translator
   hTranslator = Hlsl2Glsl_ConstructTranslator ( 0 );
   if ( hTranslator == 0 )
   {
      rErrorString = "Error creating linker for HLSL to GLSL translation";;
      return false;
   }

   // Enable user varyings
   if ( bUseUserVaryings )
      Hlsl2Glsl_UseUserVaryings ( hTranslator, true );

   if ( bUseDefaultPrecision )
   {
      const char *pShaderHeader = 
         { 
            "#ifdef GL_ES\n"
            "  precision highp float;\n"
            "#endif\n"
         };

      Hlsl2Glsl_SetShaderHeader ( hTranslator, true, pShaderHeader );
   }

   // If user attribute semantics set
   if ( nNumSemantics > 0 )
   {
      Hlsl2Glsl_SetUserAttributeNames ( hTranslator, pAttribSemantics, pAttribNames, nNumSemantics );      
   }

   if ( !Hlsl2Glsl_Translate( hTranslator, &hParser, 1, rMainFunction, rMainFunction ) )
   {
      rErrorString = Hlsl2Glsl_GetInfoLog ( hTranslator );
      Hlsl2Glsl_Destruct ( hParser );
      Hlsl2Glsl_Destruct ( hTranslator );
      return false;
   }

   // Set the GLSL source
   rGlslSource = Hlsl2Glsl_GetShader ( hTranslator, eShaderLang );

   Hlsl2Glsl_Destruct ( hParser );
   Hlsl2Glsl_Destruct ( hTranslator );
   Hlsl2Glsl_Finalize ();
   return true;
}

//=================================================================================================================================
//
//          Protected methods block
//
//=================================================================================================================================



//=================================================================================================================================
//
//          Private methods block
//
//=================================================================================================================================

