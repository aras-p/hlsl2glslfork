// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "../Include/intermediate.h"


// Limited constant folding functionality; we mostly want it for array sizes
// with constant expressions.
// Returns the node to keep using, or NULL.


TIntermConstant* FoldBinaryConstantExpression(TOperator op, TIntermConstant* nodeA, TIntermConstant* nodeB)
{
	if (!nodeA || !nodeB)
		return NULL;
	if (nodeA->getType() != nodeB->getType())
		return NULL;
	
	// for now, only support integers; we really only need constant folding for array sizes
	if (nodeA->getBasicType() != EbtInt)
		return NULL;
		
	
	TIntermConstant* newNode = new TIntermConstant(nodeA->getType());
	
	switch (op)
	{
		case EOpAdd:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeA->getValue(i).asInt + nodeB->getValue(i).asInt);
			break;
		case EOpSub:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeA->getValue(i).asInt - nodeB->getValue(i).asInt);
			break;
		case EOpMul:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeA->getValue(i).asInt * nodeB->getValue(i).asInt);
			break;
		case EOpDiv:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeB->getValue(i).asInt ? nodeA->getValue(i).asInt / nodeB->getValue(i).asInt : 0);
			break;
		case EOpMod:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeB->getValue(i).asInt ? nodeA->getValue(i).asInt % nodeB->getValue(i).asInt : 0);
			break;
		case EOpRightShift:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeA->getValue(i).asInt >> nodeB->getValue(i).asInt);
			break;
		case EOpLeftShift:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeA->getValue(i).asInt << nodeB->getValue(i).asInt);
			break;
		case EOpAnd:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeA->getValue(i).asInt & nodeB->getValue(i).asInt);
			break;
		case EOpInclusiveOr:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeA->getValue(i).asInt | nodeB->getValue(i).asInt);
			break;
		case EOpExclusiveOr:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, nodeA->getValue(i).asInt ^ nodeB->getValue(i).asInt);
			break;
		default:
			delete newNode;
			return NULL;
	}
	newNode->setLine(nodeA->getLine());
	return newNode;
}


TIntermConstant* FoldUnaryConstantExpression(TOperator op, TIntermConstant* node)
{
	if (!node)
		return NULL;
	
	// for now, only support integers; we really only need constant folding for array sizes
	if (node->getBasicType() != EbtInt)
		return NULL;
	
	
	TIntermConstant* newNode = new TIntermConstant(node->getType());
	switch (op)
	{
		case EOpNegative:
			for (unsigned i = 0; i < newNode->getCount(); ++i)
				newNode->setValue(i, -node->getValue(i).asInt);
			break;
		default:
			delete newNode;
			return NULL;
	}
	newNode->setLine(node->getLine());
	return newNode;
}
