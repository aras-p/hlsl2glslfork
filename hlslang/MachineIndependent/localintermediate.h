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

TIntermTyped* ir_add_unary_math(TOperator op, TIntermNode* child, TSourceLoc, TInfoSink& infoSink);
TIntermTyped* ir_add_binary_math(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc, TInfoSink& infoSink);
TIntermNode*  ir_add_selection(TIntermTyped* cond, TIntermNodePair code, TSourceLoc, TInfoSink& infoSink);
TIntermTyped* ir_add_selection(TIntermTyped* cond, TIntermTyped* trueBlock, TIntermTyped* falseBlock, TSourceLoc, TInfoSink& infoSink);
TIntermBranch* ir_add_branch(TOperator, TSourceLoc);
TIntermBranch* ir_add_branch(TOperator, TIntermTyped*, TSourceLoc);
TIntermNode* ir_add_loop(TLoopType type, TIntermTyped* cond, TIntermTyped* expr, TIntermNode* body, TSourceLoc line);
TIntermTyped* ir_add_swizzle(TVectorFields&, TSourceLoc);
TIntermTyped* ir_add_assign(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc, TInfoSink& infoSink);
TIntermDeclaration* ir_add_declaration(TIntermSymbol* symbol, TIntermTyped* initializer, TSourceLoc line, TInfoSink& infoSink);
TIntermDeclaration* ir_add_declaration(TSymbol* symbol, TIntermTyped* initializer, TSourceLoc line, TInfoSink& infoSink);

TIntermTyped* ir_add_conversion(TOperator, const TType&, TIntermTyped*, TInfoSink& infoSink);

TIntermTyped* ir_promote_constant(TBasicType, TIntermConstant*, TInfoSink& infoSink);
TIntermAggregate* ir_grow_aggregate(TIntermNode* left, TIntermNode* right, TSourceLoc);
TIntermAggregate* ir_make_aggregate(TIntermNode* node, TSourceLoc);
TIntermAggregate* ir_set_aggregate_op(TIntermNode*, TOperator, TSourceLoc);
TIntermDeclaration* ir_grow_declaration(TIntermDeclaration* declaration, TIntermSymbol* symbol, TIntermTyped* initializer, TInfoSink& infoSink);
TIntermDeclaration* ir_grow_declaration(TIntermDeclaration* declaration, TSymbol* symbol, TIntermTyped* initializer, TInfoSink& infoSink);


class TIntermediate
{
public:    
	POOL_ALLOCATOR_NEW_DELETE(GlobalPoolAllocator)

	TIntermediate(TInfoSink& i) : infoSink(i)
	{
	}

	void remove(TIntermNode*);
	void outputTree(TIntermNode*);

private:
	TInfoSink& infoSink;

private:
	void operator=(TIntermediate&); // prevent assignments
};

#endif // _LOCAL_INTERMEDIATE_INCLUDED_

