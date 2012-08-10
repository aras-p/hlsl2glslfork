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
class TIntermediate
{
public:    
	POOL_ALLOCATOR_NEW_DELETE(GlobalPoolAllocator)

	TIntermediate(TInfoSink& i) : infoSink(i)
	{
	}
	TIntermSymbol* addSymbol(int Id, const TString&, const TType&, TSourceLoc);
	TIntermSymbol* addSymbol(int id, const TString& name, const TTypeInfo *info, const TType& type, TSourceLoc line);
	TIntermTyped* addConversion(TOperator, const TType&, TIntermTyped*);
	TIntermTyped* addBinaryMath(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc);
	TIntermTyped* addAssign(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc);
	TIntermTyped* addIndex(TOperator op, TIntermTyped* base, TIntermTyped* index, TSourceLoc);
	TIntermTyped* addUnaryMath(TOperator op, TIntermNode* child, TSourceLoc);

	TIntermDeclaration* addDeclaration(TIntermSymbol* symbol, TIntermTyped* initializer, TSourceLoc line);
	TIntermDeclaration* addDeclaration(TSymbol* symbol, TIntermTyped* initializer, TSourceLoc line);
	TIntermDeclaration* growDeclaration(TIntermDeclaration* declaration, TIntermSymbol* symbol, TIntermTyped* initializer = 0);
	TIntermDeclaration* growDeclaration(TIntermDeclaration* declaration, TSymbol* symbol, TIntermTyped* initializer = 0);
	TIntermAggregate* growAggregate(TIntermNode* left, TIntermNode* right, TSourceLoc);
	TIntermAggregate* makeAggregate(TIntermNode* node, TSourceLoc);
	TIntermAggregate* setAggregateOperator(TIntermNode*, TOperator, TSourceLoc);
	TIntermNode*  addSelection(TIntermTyped* cond, TIntermNodePair code, TSourceLoc);
	TIntermTyped* addSelection(TIntermTyped* cond, TIntermTyped* trueBlock, TIntermTyped* falseBlock, TSourceLoc);
	TIntermTyped* addComma(TIntermTyped* left, TIntermTyped* right, TSourceLoc);
	TIntermConstant* addConstant(const TType&, TSourceLoc);
	TIntermTyped* promoteConstant(TBasicType, TIntermConstant*) ;
	TIntermNode* addLoop(TIntermNode*, TIntermTyped*, TIntermTyped*, bool testFirst, TSourceLoc);
	TIntermBranch* addBranch(TOperator, TSourceLoc);
	TIntermBranch* addBranch(TOperator, TIntermTyped*, TSourceLoc);
	TIntermTyped* addSwizzle(TVectorFields&, TSourceLoc);
	void remove(TIntermNode*);
	void outputTree(TIntermNode*);

protected:
	TInfoSink& infoSink;

private:
	void operator=(TIntermediate&); // prevent assignments
};

#endif // _LOCAL_INTERMEDIATE_INCLUDED_

