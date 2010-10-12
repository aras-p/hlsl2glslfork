// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "propagateMutable.h"



void TPropagateMutable::traverseSymbol( TIntermSymbol *node, TIntermTraverser *it )
{
   TPropagateMutable* sit = static_cast<TPropagateMutable*>(it);

   if (sit->abort)
      return;

   if (sit->propagating && sit->id == node->getId())
   {
      node->getTypePointer()->changeQualifier( EvqMutableUniform );
   }
   else if (!sit->propagating && sit->fixedIds.find(node->getId()) == sit->fixedIds.end() )
   {
      if (node->getQualifier() == EvqMutableUniform)
      {
         sit->abort = true;
         sit->id = node->getId();
         sit->fixedIds.insert(sit->id);
      }
   }
}


void TPropagateMutable::PropagateMutable( TIntermNode *node, TInfoSink &info )
{
   TPropagateMutable st(info);

   do
   {
      st.abort = false;
      node->traverse( &st);

      // If we aborted, try to type the node we aborted for
      if (st.abort)
      {
         st.propagating = true;
         st.abort = false;
         node->traverse( &st);
         st.propagating = false;
         st.abort = true;
      }
   } while (st.abort);
}
