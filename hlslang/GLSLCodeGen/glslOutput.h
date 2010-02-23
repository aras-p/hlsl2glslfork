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
// Definition of TGlslOutputTraverser.  Interface for a traverser designed to iterate the tree and generate valid
// GLSL code. 
//=================================================================================================================================
#ifndef GLSL_OUTPUT_H
#define GLSL_OUTPUT_H

#include <sstream>

#include "localintermediate.h"
#include "glslCommon.h"
#include "glslStruct.h"
#include "glslSymbol.h"
#include "glslFunction.h"


//=================================================================================================================================
/// TGlslOutputTraverser
/// 
/// \brief This class implements the tree traverser that outputs GLSL code for the tree
//=================================================================================================================================
class TGlslOutputTraverser : public TIntermTraverser 
{
private:
   //=========================================================================================================
   /// Traverse a symbol node, add the name to the output buffer
   //=========================================================================================================
   static void traverseSymbol(TIntermSymbol*, TIntermTraverser*);

   //=========================================================================================================   
   /// Traverse a symbol node which is a function parameter, add the name to the output buffer
   //=========================================================================================================   
   static void traverseParameterSymbol(TIntermSymbol *node, TIntermTraverser *it);

   //=========================================================================================================   
   /// Traverse a node in the tree representing a constant
   //=========================================================================================================   
   static void traverseConstantUnion(TIntermConstantUnion*, TIntermTraverser*);

   //=========================================================================================================      
   /// Traverse a node in the tree representing a constant intended for immediate use, such as a direct reference
   //=========================================================================================================      
   static void traverseImmediateConstant( TIntermConstantUnion *node, TIntermTraverser *it);

   //=========================================================================================================
   /// Traverse a binary node generating the output GLSL code
   //=========================================================================================================
   static bool traverseBinary(bool preVisit, TIntermBinary*, TIntermTraverser*);
   
   //=========================================================================================================
   /// Traverse a unary node generating the output GLSL code
   //=========================================================================================================
   static bool traverseUnary(bool preVisit, TIntermUnary*, TIntermTraverser*);

   //=========================================================================================================
   /// Traverse a selection generating the output GLSL code
   //=========================================================================================================
   static bool traverseSelection(bool preVisit, TIntermSelection*, TIntermTraverser*);

   //=========================================================================================================
   /// Traverse an aggregate node generating the output GLSL code
   //=========================================================================================================
   static bool traverseAggregate(bool preVisit, TIntermAggregate*, TIntermTraverser*);

   //=========================================================================================================
   /// Traverse a loop node generating the output GLSL code
   //=========================================================================================================
   static bool traverseLoop(bool preVisit, TIntermLoop*, TIntermTraverser*);

   //=========================================================================================================
   /// Traverse a branch node generating the output GLSL code
   //=========================================================================================================   
   static bool traverseBranch(bool preVisit, TIntermBranch*,  TIntermTraverser*);

public:

   //=========================================================================================================
   /// Constructor
   //=========================================================================================================   
   TGlslOutputTraverser(TInfoSink& i, std::vector<GlslFunction*> &funcList, std::vector<GlslStruct*> &sList);


   //=========================================================================================================
   /// Create a GLSL structure from a structure TType
   //=========================================================================================================
   GlslStruct *createStructFromType( TType *type );

   //=========================================================================================================
   /// Parse a binary node that represents an variable initalizer
   //=========================================================================================================  
   bool parseInitializer( TIntermBinary *node );

   // Info Sink
   TInfoSink& infoSink;

   // Global function
   GlslFunction *global;

   // Current function
   GlslFunction *current;

   // Are we currently generating code?
   bool generatingCode;

   // List of functions
   std::vector<GlslFunction*> &functionList;

   // List of structures
   std::vector<GlslStruct*> &structList;

   // Map of structure names to GLSL structures
   std::map<std::string,GlslStruct*> structMap;

   // Persistent data for collecting indices
   std::vector<int> indexList;

};

#endif //GLSL_OUTPUT_H