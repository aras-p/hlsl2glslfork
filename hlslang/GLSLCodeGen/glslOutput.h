// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef GLSL_OUTPUT_H
#define GLSL_OUTPUT_H

#include <sstream>

#include "localintermediate.h"
#include "glslCommon.h"
#include "glslStruct.h"
#include "glslSymbol.h"
#include "glslFunction.h"


class TGlslOutputTraverser : public TIntermTraverser 
{
private:
   static void traverseSymbol(TIntermSymbol*, TIntermTraverser*);
   static void traverseParameterSymbol(TIntermSymbol *node, TIntermTraverser *it);
   static void traverseConstantUnion(TIntermConstantUnion*, TIntermTraverser*);
   static void traverseImmediateConstant( TIntermConstantUnion *node, TIntermTraverser *it);
   static bool traverseBinary(bool preVisit, TIntermBinary*, TIntermTraverser*);
   static bool traverseUnary(bool preVisit, TIntermUnary*, TIntermTraverser*);
   static bool traverseSelection(bool preVisit, TIntermSelection*, TIntermTraverser*);
   static bool traverseAggregate(bool preVisit, TIntermAggregate*, TIntermTraverser*);
   static bool traverseLoop(bool preVisit, TIntermLoop*, TIntermTraverser*);
   static bool traverseBranch(bool preVisit, TIntermBranch*,  TIntermTraverser*);

public:
   TGlslOutputTraverser (TInfoSink& i, std::vector<GlslFunction*> &funcList, std::vector<GlslStruct*> &sList, bool usePrecision);
   GlslStruct *createStructFromType( TType *type );
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

   bool m_UsePrecision;
};

#endif //GLSL_OUTPUT_H
