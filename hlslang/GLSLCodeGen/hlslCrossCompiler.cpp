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
// Implementation of HlslCrossCompiler
//=================================================================================================================================

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================

#include "hlslCrossCompiler.h"

#include "glslOutput.h"
#include "typeSamplers.h"
#include "propagateMutable.h"

//=================================================================================================================================
//
//          Constructor(s) / Destructor(s) Block 
//
//=================================================================================================================================

//=========================================================================================================
/// Constructor
//=========================================================================================================
HlslCrossCompiler::HlslCrossCompiler(EShLanguage l, int dOptions) : 
   TCompiler(l, infoSink), 
   debugOptions(dOptions)
{
}

//=========================================================================================================
/// Destructor
//=========================================================================================================
HlslCrossCompiler::~HlslCrossCompiler()
{
   for ( std::vector<GlslFunction*>::iterator it = functionList.begin() ; it != functionList.end(); it++)
   {
      delete *it;
   }

   for ( std::vector<GlslStruct*>::iterator it = structList.begin() ; it != structList.end(); it++)
   {
      delete *it;
   }   
}

//=================================================================================================================================
//
//          Public Methods Block
//
//=================================================================================================================================

//=========================================================================================================
/// Generate code from the given parse tree
/// \param root
///   Root node of the tree to compile
/// \return 
///   True on succesful compilation of tree, false otherwise
//=========================================================================================================
bool HlslCrossCompiler::compile( TIntermNode *root )
{
   haveValidObjectCode = true;

   TGlslOutputTraverser glslTraverse( infoSink, functionList, structList);

   TSamplerTraverser::TypeSamplers( root, infoSink);
   TPropagateMutable::PropagateMutable( root, infoSink);

   root->traverse(&glslTraverse);

   return haveValidObjectCode;
}