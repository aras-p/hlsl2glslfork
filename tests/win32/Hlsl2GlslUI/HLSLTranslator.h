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
//   Header for HLSLTranslator class
//=================================================================================================================================

#ifndef _HLSLTRANSLATOR_H_
#define _HLSLTRANSLATOR_H_

// Include HLSL -> GLSL translator
#include <HLSL2GLSL.h>

//=========================================================================================================
/// \brief Converts an HLSL shader to GLSL using a translation library
/// 
//=========================================================================================================
class HLSLTranslator
{
public:
   /// Constructor
   HLSLTranslator();

   /// Destructor
   virtual ~HLSLTranslator();

   /// Translate an HLSL shader to GLSL
   bool Translate ( const CString& rHlslSource, const CString& rMainFunction, const CString& rShaderType,
                    CString& rGlslSource, CString& rErrorString, bool bUseUserVaryings, bool bUseDefaultPrecision,
                    int nNumSemantics, const EAttribSemantic *pAttribSemantics, const char* pAttribNames[] );

private:

};


#endif // _SU_FXHLSLTOGLSL_H_
