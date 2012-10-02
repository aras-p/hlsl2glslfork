// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef _LOCAL_INTERMEDIATE_INCLUDED_
#define _LOCAL_INTERMEDIATE_INCLUDED_

#include "../Include/intermediate.h"
#include "../../include/hlsl2glsl.h"
#include "SymbolTable.h"

struct TVectorFields
{
   int offsets[4];
   int num;
};

//
// Set of helper functions to help parse and build the tree.
//
class TInfoSink;

TIntermSymbol* ir_add_symbol(int Id, const TString&, const TType&, TSourceLoc);
TIntermSymbol* ir_add_symbol(int id, const TString& name, const TTypeInfo *info, const TType& type, TSourceLoc line);
TIntermConstant* ir_add_constant(const TType&, TSourceLoc);
TIntermTyped* ir_add_index(TOperator op, TIntermTyped* base, TIntermTyped* index, TSourceLoc);
TIntermTyped* ir_add_comma(TIntermTyped* left, TIntermTyped* right, TSourceLoc);
TIntermNode*  ir_add_selection(TIntermTyped* cond, TIntermNodePair code, TSourceLoc, TInfoSink& infoSink);
TIntermTyped* ir_add_selection(TIntermTyped* cond, TIntermTyped* trueBlock, TIntermTyped* falseBlock, TSourceLoc, TInfoSink& infoSink);

TIntermTyped* ir_add_conversion(TOperator, const TType&, TIntermTyped*, TInfoSink& infoSink);

TIntermTyped* ir_promote_constant(TBasicType, TIntermConstant*, TInfoSink& infoSink);
TIntermAggregate* ir_grow_aggregate(TIntermNode* left, TIntermNode* right, TSourceLoc);
TIntermAggregate* ir_make_aggregate(TIntermNode* node, TSourceLoc);
TIntermAggregate* ir_set_aggregate_op(TIntermNode*, TOperator, TSourceLoc);


class TIntermediate
{
public:    
	POOL_ALLOCATOR_NEW_DELETE(GlobalPoolAllocator)

	TIntermediate(TInfoSink& i) : infoSink(i)
	{
	}
	TIntermTyped* addBinaryMath(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc);
	TIntermTyped* addAssign(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc);
	TIntermTyped* addUnaryMath(TOperator op, TIntermNode* child, TSourceLoc);

	TIntermDeclaration* addDeclaration(TIntermSymbol* symbol, TIntermTyped* initializer, TSourceLoc line);
	TIntermDeclaration* addDeclaration(TSymbol* symbol, TIntermTyped* initializer, TSourceLoc line);
	TIntermDeclaration* growDeclaration(TIntermDeclaration* declaration, TIntermSymbol* symbol, TIntermTyped* initializer = 0);
	TIntermDeclaration* growDeclaration(TIntermDeclaration* declaration, TSymbol* symbol, TIntermTyped* initializer = 0);
	TIntermNode* addLoop(TLoopType type, TIntermTyped* cond, TIntermTyped* expr, TIntermNode* body, TSourceLoc line);
	TIntermBranch* addBranch(TOperator, TSourceLoc);
	TIntermBranch* addBranch(TOperator, TIntermTyped*, TSourceLoc);
	TIntermTyped* addSwizzle(TVectorFields&, TSourceLoc);
	void remove(TIntermNode*);
	void outputTree(TIntermNode*);

private:
	TInfoSink& infoSink;

private:
	void operator=(TIntermediate&); // prevent assignments
};

#endif // _LOCAL_INTERMEDIATE_INCLUDED_

