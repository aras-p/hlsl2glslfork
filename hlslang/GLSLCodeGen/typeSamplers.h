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

#ifndef TYPE_SAMPLERS_H
#define TYPE_SAMPLERS_H

#include "localintermediate.h"
#include "glslOutput.h"

/// Iterates over the intermediate tree and sets untyped sampler symbols to have types based on the
/// type of texture operation the samplers are used for
class TSamplerTraverser : public TIntermTraverser 
{
private:

   static void traverseSymbol(TIntermSymbol*, TIntermTraverser*);
   static void traverseParameterSymbol(TIntermSymbol *node, TIntermTraverser *it);
   static bool traverseBinary(bool preVisit, TIntermBinary*, TIntermTraverser*);
   static bool traverseUnary(bool preVisit, TIntermUnary*, TIntermTraverser*);
   static bool traverseSelection(bool preVisit, TIntermSelection*, TIntermTraverser*);
   static bool traverseAggregate(bool preVisit, TIntermAggregate*, TIntermTraverser*);
   static bool traverseLoop(bool preVisit, TIntermLoop*, TIntermTraverser*);
   static bool traverseBranch(bool preVisit, TIntermBranch*,  TIntermTraverser*);
   
   /// Set the type for the sampler
   void typeSampler( TIntermTyped *node, TBasicType samp);

   TInfoSink& infoSink;

   bool abort;

   // These are used to go into "typing mode"
   bool typing;
   int id;
   TBasicType sampType;

   std::map<std::string,TIntermSequence* > functionMap;

   std::string currentFunction;
	
public:
   TSamplerTraverser( TInfoSink &is) : infoSink(is), abort(false), typing(false), id(0), sampType(EbtSamplerGeneric) 
   {
      visitSymbol = traverseSymbol;
      //visitConstantUnion = traverseConstantUnion;
      visitBinary = traverseBinary;
      visitUnary = traverseUnary;
      visitSelection = traverseSelection;
      visitAggregate = traverseAggregate;
      visitLoop = traverseLoop;
      visitBranch = traverseBranch;
   }
   
   static void TypeSamplers( TIntermNode *node, TInfoSink &info);    
};

#endif //TYPE_SAMPLERS_H
