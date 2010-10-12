// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


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
