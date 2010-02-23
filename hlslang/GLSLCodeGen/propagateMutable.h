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
// Definition of TPropagateMutable
//=================================================================================================================================
#ifndef PROPAGATE_MUTABLE_H
#define PROPAGATE_MUTABLE_H

#include <set>

#include "localintermediate.h"

#include "glslOutput.h"

//=================================================================================================================================
/// TPropagateMutable
/// 
/// \brief This class iterates over the intermediate tree and propgates mutable uniforms qualifiers as necessary
///        to the symbols
//=================================================================================================================================
class TPropagateMutable : public TIntermTraverser 
{
private:

   //=========================================================================================================
   /// Propage the mutable uniform qualifier throughout the symbols as needed
   //=========================================================================================================   
   static void traverseSymbol(TIntermSymbol*, TIntermTraverser*);

   // Info sink
   TInfoSink& infoSink;

   // Whether to abort propogation
   bool abort;

   // These are used to go into "propagating mode"
   bool propagating;
   int id;

   // set to prevent infinite loops
   std::set<int> fixedIds;


public:

   //=========================================================================================================
   /// Constructor
   //=========================================================================================================      
   TPropagateMutable( TInfoSink &is) : infoSink(is), abort(false), propagating(false), id(0) 
   {
      visitSymbol = traverseSymbol;
   }

   //=========================================================================================================
   /// This function initiates the propogation.
   //=========================================================================================================         
   static void PropagateMutable( TIntermNode *node, TInfoSink &info);
    
};

#endif //PROPAGATE_MUTABLE_H