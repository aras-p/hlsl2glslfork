// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef PROPAGATE_MUTABLE_H
#define PROPAGATE_MUTABLE_H

#include <set>
#include "localintermediate.h"
#include "glslOutput.h"


/// Iterates over the intermediate tree and propagates mutable uniforms qualifiers as necessary
/// to the symbols.
class TPropagateMutable : public TIntermTraverser 
{
private:

   /// Propage the mutable uniform qualifier throughout the symbols as needed
   static void traverseSymbol(TIntermSymbol*, TIntermTraverser*);

   TInfoSink& infoSink;

   bool abort;

   // These are used to go into "propagating mode"
   bool propagating;
   int id;

   std::set<int> fixedIds; // to prevent infinite loops


public:

   TPropagateMutable( TInfoSink &is) : infoSink(is), abort(false), propagating(false), id(0) 
   {
      visitSymbol = traverseSymbol;
   }

   static void PropagateMutable( TIntermNode *node, TInfoSink &info);
    
};


#endif //PROPAGATE_MUTABLE_H
