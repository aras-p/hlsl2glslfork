// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


//
// Build the intermediate representation.
//

#include "localintermediate.h"
#include "RemoveTree.h"
#include <float.h>
#include <limits.h>

static TPrecision GetHigherPrecision (TPrecision left, TPrecision right) {
	return left > right ? left : right;
}


// First set of functions are to help build the intermediate representation.
// These functions are not member functions of the nodes.
// They are called from parser productions.


// Add a terminal node for an identifier in an expression.
TIntermSymbol* ir_add_symbol(const TVariable* var, TSourceLoc line)
{
	TIntermSymbol* node = ir_add_symbol_internal(var->getUniqueId(), var->getName(), var->getInfo(), var->getType(), line);
	node->setGlobal(var->isGlobal());
	return node;
}

TIntermSymbol* ir_add_symbol_internal(int id, const TString& name, const TTypeInfo *info, const TType& type, TSourceLoc line)
{
	TIntermSymbol* node = new TIntermSymbol(id, name, info, type);
	node->setLine(line);
	return node;
}


// Connect two nodes with a new parent that does a binary operation on the nodes.
TIntermTyped* ir_add_binary_math(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc line, TInfoSink& infoSink)
{
	if (!left || !right)
		return 0;
	
	switch (op)
	{
	case EOpLessThan:
	case EOpGreaterThan:
	case EOpLessThanEqual:
	case EOpGreaterThanEqual:
		if (left->getType().isMatrix() || left->getType().isArray() || left->getType().getBasicType() == EbtStruct)
		{
			return 0;
		}
		break;
	case EOpLogicalOr:
	case EOpLogicalXor:
	case EOpLogicalAnd:
		if (left->getType().isMatrix() || left->getType().isArray())
			return 0;
		
		if ( left->getBasicType() != EbtBool )
		{
			if ( left->getType().getBasicType() != EbtInt && left->getType().getBasicType() != EbtFloat )
				return 0;
			else
			{
				// If the left is a float or int, convert to a bool.  This is the conversion that HLSL
				// does
				left = ir_add_conversion(EOpConstructBool, 
									 TType ( EbtBool, left->getPrecision(), left->getQualifier(),
											left->getColsCount(), left->getRowsCount(), left->isMatrix(), left->isArray()), 
									 left, infoSink);
				
				if ( left == 0 )
					return 0;
			}
			
		}
		
		if (right->getType().isMatrix() || right->getType().isArray() || right->getType().isVector())
			return 0;
		
		if ( right->getBasicType() != EbtBool )
		{
			if ( right->getType().getBasicType() != EbtInt && right->getType().getBasicType() != EbtFloat )
				return 0;
			else
			{
				// If the right is a float or int, convert to a bool.  This is the conversion that HLSL
				// does
				right = ir_add_conversion(EOpConstructBool, 
									  TType ( EbtBool, right->getPrecision(), right->getQualifier(),
											 right->getColsCount(), right->getRowsCount(), right->isMatrix(), right->isArray()), 
									  right, infoSink);
				
				if ( right == 0 )
					return 0;
			}
			
		}
		break;
	case EOpAdd:
	case EOpSub:
	case EOpDiv:
	case EOpMul:
	case EOpMod:
		{
			TBasicType ltype = left->getType().getBasicType();
			TBasicType rtype = right->getType().getBasicType();
			if (ltype == EbtStruct)
				return 0;
			
			// If left or right type is a bool, convert to float.
			bool leftToFloat = (ltype == EbtBool);
			bool rightToFloat = (rtype == EbtBool);
			// For modulus, if either is an integer, convert to float as well.
			if (op == EOpMod)
			{
				leftToFloat |= (ltype == EbtInt);
				rightToFloat |= (rtype == EbtInt);
			}
				
			if (leftToFloat)
			{
				left = ir_add_conversion (EOpConstructFloat, TType (EbtFloat, left->getPrecision(), left->getQualifier(), left->getColsCount(), left->getRowsCount(), left->isMatrix(), left->isArray()), left, infoSink);
				if (left == 0)
					return 0;
			}
			if (rightToFloat)
			{
				right = ir_add_conversion (EOpConstructFloat, TType (EbtFloat, right->getPrecision(), right->getQualifier(), right->getColsCount(), right->getRowsCount(), right->isMatrix(), right->isArray()), right, infoSink);
				if (right == 0)
					return 0;
			}
		}
		break;
	default:
		break;
	}
	
	// 
	// First try converting the children to compatible types.
	//
	
	if (!(left->getType().getStruct() && right->getType().getStruct()))
	{
		TIntermTyped* child = 0;
		bool useLeft = true; //default to using the left child as the type to promote to
		
		//need to always convert up
		if ( left->getType().getBasicType() != EbtFloat)
		{
			if ( right->getType().getBasicType() == EbtFloat)
			{
				useLeft = false;
			}
			else
			{
				if ( left->getType().getBasicType() != EbtInt)
				{
					if ( right->getType().getBasicType() == EbtInt)
						useLeft = false;
				}
			}
		}
		
		if (useLeft)
		{
			child = ir_add_conversion(op, left->getType(), right, infoSink);
			if (child)
				right = child;
			else
			{
				child = ir_add_conversion(op, right->getType(), left, infoSink);
				if (child)
					left = child;
				else
					return 0;
			}
		}
		else
		{
			child = ir_add_conversion(op, right->getType(), left, infoSink);
			if (child)
				left = child;
			else
			{
				child = ir_add_conversion(op, left->getType(), right, infoSink);
				if (child)
					right = child;
				else
					return 0;
			}
		}
		
	}
	else
	{
		if (left->getType() != right->getType())
			return 0;
	}
	
	
	//
	// Need a new node holding things together then.  Make
	// one and promote it to the right type.
	//
	TIntermBinary* node = new TIntermBinary(op);
	if (line.line == 0)
		line = right->getLine();
	node->setLine(line);
	
	node->setLeft(left);
	node->setRight(right);
	if (! node->promote(infoSink))
		return 0;
	
	//
	// See if we can fold constants
	
	TIntermConstant* constA = left->getAsConstant();
	TIntermConstant* constB = right->getAsConstant();
	
	if (constA && constB)
	{
		TIntermConstant* FoldBinaryConstantExpression(TOperator op, TIntermConstant* nodeA, TIntermConstant* nodeB);
		TIntermConstant* res = FoldBinaryConstantExpression(node->getOp(), constA, constB);
		if (res)
			return res;
	}	
	
	return node;
}


// Connect two nodes through an assignment.
TIntermTyped* ir_add_assign(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc line, TInfoSink& infoSink)
{
   //
   // Like adding binary math, except the conversion can only go
   // from right to left.
   //
   TIntermBinary* node = new TIntermBinary(op);
   if (line.line == 0)
      line = left->getLine();
   node->setLine(line);

   TIntermTyped* child = ir_add_conversion(op, left->getType(), right, infoSink);
   if (child == 0)
      return 0;

   node->setLeft(left);
   node->setRight(child);
   if (! node->promote(infoSink))
      return 0;

   return node;
}


// Connect two nodes through an index operator, where the left node is the base
// of an array or struct, and the right node is a direct or indirect offset.
//
// The caller should set the type of the returned node.
TIntermTyped* ir_add_index(TOperator op, TIntermTyped* base, TIntermTyped* index, TSourceLoc line)
{
	TIntermBinary* node = new TIntermBinary(op);
	if (line.line == 0)
		line = index->getLine();
	node->setLine(line);
	node->setLeft(base);
	node->setRight(index);

	// caller should set the type

	return node;
}


// Add one node as the parent of another that it operates on.
TIntermTyped* ir_add_unary_math(TOperator op, TIntermNode* childNode, TSourceLoc line, TInfoSink& infoSink)
{
   TIntermUnary* node;
   TIntermTyped* child = childNode->getAsTyped();

   if (child == 0)
   {
      infoSink.info.message(EPrefixInternalError, "Bad type in AddUnaryMath", line);
      return 0;
   }

   switch (op)
   {
   case EOpLogicalNot:
      if (!child->isScalar())
         return 0;
      break;

   case EOpPostIncrement:
   case EOpPreIncrement:
   case EOpPostDecrement:
   case EOpPreDecrement:
   case EOpNegative:
      if (child->getType().getBasicType() == EbtStruct || child->getType().isArray())
         return 0;
   default: break;
   }

   //
   // Do we need to promote the operand?
   //
   // Note: Implicit promotions were removed from the language.
   //
   TBasicType newType = EbtVoid;
   switch (op)
   {
   case EOpConstructInt:   newType = EbtInt;   break;
   case EOpConstructBool:  newType = EbtBool;  break;
   case EOpConstructFloat: newType = EbtFloat; break;
   case EOpLogicalNot:     newType = EbtBool; break;
   default: break;
   }

   if (newType != EbtVoid)
   {
      child = ir_add_conversion(op, TType(newType, child->getPrecision(), EvqTemporary, child->getColsCount(), child->getRowsCount(), 
                                      child->isMatrix(), 
                                      child->isArray()),
                            child, infoSink);
      if (child == 0)
         return 0;
   }

   //
   // For constructors, we are now done, it's all in the conversion.
   //
   switch (op)
   {
   case EOpConstructInt:
   case EOpConstructBool:
   case EOpConstructFloat:
      return child;
   default: break;
   }

   TIntermConstant* childConst = child->getAsConstant();

   //
   // Make a new node for the operator.
   //
   node = new TIntermUnary(op);
   if (line.line == 0)
      line = child->getLine();
   node->setLine(line);
   node->setOperand(child);

   if (! node->promote(infoSink))
      return 0;
	
	
	//
	// See if we can fold constants
	
	if (childConst)
	{
		TIntermConstant* FoldUnaryConstantExpression(TOperator op, TIntermConstant* node);
		TIntermConstant* res = FoldUnaryConstantExpression(node->getOp(), childConst);
		if (res)
			return res;
	}	
	

	return node;
}


// This is the safe way to change the operator on an aggregate, as it
// does lots of error checking and fixing.  Especially for establishing
// a function call's operation on it's set of parameters.  Sequences
// of instructions are also aggregates, but they just direnctly set
// their operator to EOpSequence.
//
// Returns an aggregate node, which could be the one passed in if
// it was already an aggregate.
TIntermAggregate* ir_set_aggregate_op(TIntermNode* node, TOperator op, TSourceLoc line)
{
   TIntermAggregate* aggNode;

   //
   // Make sure we have an aggregate.  If not turn it into one.
   //
   if (node)
   {
      aggNode = node->getAsAggregate();
      if (aggNode == 0 || aggNode->getOp() != EOpNull)
      {
         //
         // Make an aggregate containing this node.
         //
         aggNode = new TIntermAggregate();
         aggNode->getNodes().push_back(node);
         if (line.line == 0)
            line = node->getLine();
      }
   }
   else
      aggNode = new TIntermAggregate();

   //
   // Set the operator.
   //
   aggNode->setOperator(op);
   if (line.line != 0)
      aggNode->setLine(line);

   return aggNode;
}


// Convert one type to another.
//
// Returns the node representing the conversion, which could be the same
// node passed in if no conversion was needed. Returns NULL if conversion can't be done.
TIntermTyped* ir_add_conversion(TOperator op, const TType& type, TIntermTyped* node, TInfoSink& infoSink)
{
    if (!node)
        return 0;

   //
   // Does the base type allow operation?
   //
   switch (node->getBasicType())
   {
   case EbtVoid:
   case EbtSampler1D:
   case EbtSampler2D:
   case EbtSampler3D:
   case EbtSamplerCube:
   case EbtSampler1DShadow:
   case EbtSampler2DShadow:
   case EbtSamplerRect:        // ARB_texture_rectangle
   case EbtSamplerRectShadow:  // ARB_texture_rectangle
      return 0;
   default: break;
   }

   //
   // Otherwise, if types are identical, no problem
   //
   if (type == node->getType())
      return node;

   // if basic types are identical, promotions will handle everything
   if (type.getBasicType() == node->getTypePointer()->getBasicType())
      return node;

   //
   // If one's a structure, then no conversions.
   //
   if (type.getStruct() || node->getType().getStruct())
      return 0;

   //
   // If one's an array, then no conversions.
   //
   if (type.isArray() || node->getType().isArray())
      return 0;

   TBasicType promoteTo;

   switch (op)
   {
   //
   // Explicit conversions
   //
   case EOpConstructBool:
      promoteTo = EbtBool;
      break;
   case EOpConstructFloat:
      promoteTo = EbtFloat;
      break;
   case EOpConstructInt:
      promoteTo = EbtInt;
      break;
   default:
      //
      // implicit conversions are required for hlsl
      //
      promoteTo = type.getBasicType();
   }

   if (node->getAsConstant())
   {

      return ir_promote_constant(promoteTo, node->getAsConstant(), infoSink);
   }
   else
   {

      //
      // Add a new newNode for the conversion.
      //
      TIntermUnary* newNode = 0;

      TOperator newOp = EOpNull;
      switch (promoteTo)
      {
      case EbtFloat:
         switch (node->getBasicType())
         {
         case EbtInt:   newOp = EOpConvIntToFloat;  break;
         case EbtBool:  newOp = EOpConvBoolToFloat; break;
         default:
            infoSink.info.message(EPrefixInternalError, "Bad promotion node", node->getLine());
            return 0;
         }
         break;
      case EbtBool:
         switch (node->getBasicType())
         {
         case EbtInt:   newOp = EOpConvIntToBool;   break;
         case EbtFloat: newOp = EOpConvFloatToBool; break;
         default:
            infoSink.info.message(EPrefixInternalError, "Bad promotion node", node->getLine());
            return 0;
         }
         break;
      case EbtInt:
         switch (node->getBasicType())
         {
         case EbtBool:   newOp = EOpConvBoolToInt;  break;
         case EbtFloat:  newOp = EOpConvFloatToInt; break;
         default:
            infoSink.info.message(EPrefixInternalError, "Bad promotion node", node->getLine());
            return 0;
         }
         break;
      default:
         infoSink.info.message(EPrefixInternalError, "Bad promotion type", node->getLine());
         return 0;
      }

      TType type(promoteTo, node->getPrecision(), EvqTemporary, node->getColsCount(), node->getRowsCount(), node->isMatrix(), node->isArray());
      newNode = new TIntermUnary(newOp, type);
      newNode->setLine(node->getLine());
      newNode->setOperand(node);

      return newNode;
   }
}

TIntermDeclaration* ir_add_declaration(TIntermSymbol* symbol, TIntermTyped* initializer, TSourceLoc line, TInfoSink& infoSink)
{
	TIntermDeclaration* decl = new TIntermDeclaration(symbol->getType());
	decl->setLine(line);
	
	if (!initializer)
		decl->getDeclaration() = symbol;
	else
		decl->getDeclaration() = ir_add_assign(EOpAssign, symbol, initializer, line, infoSink);
	
	return decl;
}

TIntermDeclaration* ir_add_declaration(TSymbol* symbol, TIntermTyped* initializer, TSourceLoc line, TInfoSink& infoSink)
{
	TVariable* var = static_cast<TVariable*>(symbol);
	TIntermSymbol* sym = ir_add_symbol(var, line);

	return ir_add_declaration(sym, initializer, line, infoSink);
}


TIntermDeclaration* ir_grow_declaration(TIntermDeclaration* declaration, TSymbol* symbol, TIntermTyped* initializer, TInfoSink& infoSink)
{
	TVariable* var = static_cast<TVariable*>(symbol);
	TIntermSymbol* sym = ir_add_symbol(var, var->getType().getLine());
	
	return ir_grow_declaration(declaration, sym, initializer, infoSink);
}

TIntermDeclaration* ir_grow_declaration(TIntermDeclaration* declaration, TIntermSymbol *symbol, TIntermTyped *initializer, TInfoSink& infoSink)
{
	TIntermTyped* added_decl = symbol;
	if (initializer)
		added_decl = ir_add_assign(EOpAssign, symbol, initializer, symbol->getLine(), infoSink);
	
	if (declaration->isSingleDeclaration()) {
		TIntermTyped* current = declaration->getDeclaration();
		TIntermAggregate* aggregate = ir_make_aggregate(current, current->getLine());
		aggregate->setOperator(EOpComma);
		declaration->getDeclaration() = aggregate;
	}
		
	TIntermAggregate* aggregate = ir_grow_aggregate(declaration->getDeclaration(), added_decl, added_decl->getLine());
	aggregate->setOperator(EOpComma);
	declaration->getDeclaration() = aggregate;
	
	return declaration;
}

bool TIntermDeclaration::containsArrayInitialization() {
	const TType& t = *this->getTypePointer();
	if (isSingleInitialization() && t.isArray())
		return true;
	
	if (t.isArray() && isMultipleDeclaration()) {
		TNodeArray& decls = _declaration->getAsAggregate()->getNodes();
		unsigned n_decls = decls.size();
		for (unsigned i = 0; i != n_decls; ++i) {
			if (decls[i]->getAsBinaryNode())
				return true;
		}
	}
	
	return false;
}


// Safe way to combine two nodes into an aggregate.  Works with null pointers, 
// a node that's not a aggregate yet, etc.
//
// Returns the resulting aggregate, unless 0 was passed in for
// both existing nodes.
TIntermAggregate* ir_grow_aggregate(TIntermNode* left, TIntermNode* right, TSourceLoc line)
{
   if (left == 0 && right == 0)
      return 0;

   TIntermAggregate* aggNode = 0;
   if (left)
      aggNode = left->getAsAggregate();
   if (!aggNode || aggNode->getOp() != EOpNull)
   {
      aggNode = new TIntermAggregate;
      if (left)
         aggNode->getNodes().push_back(left);
   }

   if (right)
      aggNode->getNodes().push_back(right);

   if (line.line != 0)
      aggNode->setLine(line);

   return aggNode;
}


// Turn an existing node into an aggregate.
TIntermAggregate* ir_make_aggregate(TIntermNode* node, TSourceLoc line)
{
	if (node == 0)
		return 0;

	TIntermAggregate* aggNode = new TIntermAggregate;
	if (node->getAsTyped())
		aggNode->setType(*node->getAsTyped()->getTypePointer());
	
	aggNode->getNodes().push_back(node);

	if (line.line != 0)
		aggNode->setLine(line);
	else
		aggNode->setLine(node->getLine());

	return aggNode;
}


// For "if" test nodes.  There are three children; a condition,
// a true path, and a false path.  The two paths are in the
// nodePair.
TIntermNode* ir_add_selection(TIntermTyped* cond, TIntermNodePair nodePair, TSourceLoc line, TInfoSink& infoSink)
{   
   // Convert float/int to bool
   if ( cond->getBasicType() != EbtBool)
   {
      cond = ir_add_conversion (EOpConstructBool,
                             TType (EbtBool, cond->getPrecision(), cond->getQualifier(), cond->getColsCount(), cond->getRowsCount(), cond->isMatrix(), cond->isArray()),
                             cond, infoSink);
   }

   TIntermSelection* node = new TIntermSelection(cond, nodePair.node1, nodePair.node2);
   node->setLine(line);

   return node;
}


TIntermTyped* ir_add_comma(TIntermTyped* left, TIntermTyped* right, TSourceLoc line)
{
	if (left->getType().getQualifier() == EvqConst && right->getType().getQualifier() == EvqConst)
	{
		return right;
	}
	else
	{
		TIntermTyped *commaAggregate = ir_grow_aggregate(left, right, line);
		commaAggregate->getAsAggregate()->setOperator(EOpComma);    
		commaAggregate->setType(right->getType());
		commaAggregate->getTypePointer()->changeQualifier(EvqTemporary);
		return commaAggregate;
	}
}

// For "?:" test nodes.  There are three children; a condition,
// a true path, and a false path.  The two paths are specified
// as separate parameters.
TIntermTyped* ir_add_selection(TIntermTyped* cond, TIntermTyped* trueBlock, TIntermTyped* falseBlock, TSourceLoc line, TInfoSink& infoSink)
{
   bool bPromoteFromTrueBlockType = true;

   if (cond->getBasicType() != EbtBool)
   {
	   cond = ir_add_conversion (EOpConstructBool, 
		   TType (EbtBool, cond->getPrecision(), cond->getQualifier(), cond->getColsCount(), cond->getRowsCount(), cond->isMatrix(), cond->isArray()),
		   cond, infoSink);
   }

   // Choose which one to try to promote to based on which has more precision
   // By default, it will promote from the falseBlock type to the trueBlock type.  However,
   // what we want to do is promote to the type with the most precision of the two.  So here,
   // check whether the false block has more precision than the true block, and if so use
   // its type instead.
   if ( trueBlock->getBasicType() == EbtBool )
   {
      if ( falseBlock->getBasicType() == EbtInt ||
           falseBlock->getBasicType() == EbtFloat )
      {
         bPromoteFromTrueBlockType = false;
      }
   }
   else if ( trueBlock->getBasicType() == EbtInt )
   {
      if ( falseBlock->getBasicType() == EbtFloat )
      {
         bPromoteFromTrueBlockType = false;
      }
   }

   //
   // Get compatible types.
   //
   if ( bPromoteFromTrueBlockType )
   {
      TIntermTyped* child = ir_add_conversion(EOpSequence, trueBlock->getType(), falseBlock, infoSink);
      if (child)
         falseBlock = child;
      else
      {
         child = ir_add_conversion(EOpSequence, falseBlock->getType(), trueBlock, infoSink);
         if (child)
            trueBlock = child;
         else
            return 0;
      }
   }
   else
   {
      TIntermTyped* child = ir_add_conversion(EOpSequence, falseBlock->getType(), trueBlock, infoSink);
      if (child)
         trueBlock = child;
      else
      {
         child = ir_add_conversion(EOpSequence, trueBlock->getType(), falseBlock, infoSink);
         if (child)
            falseBlock = child;
         else
            return 0;
      }
   }

   //
   // Make a selection node.
   //
   TIntermSelection* node = new TIntermSelection(cond, trueBlock, falseBlock, trueBlock->getType());
   node->setLine(line);

    if (!node->promoteTernary(infoSink))
        return 0;


   return node;
}


// Constant terminal nodes.  Has a union that contains bool, float or int constants
TIntermConstant* ir_add_constant(const TType& t, TSourceLoc line)
{
	TIntermConstant* node = new TIntermConstant(t);
	node->setLine(line);
	return node;
}


TIntermTyped* ir_add_swizzle(TVectorFields& fields, TSourceLoc line)
{
	TIntermAggregate* node = new TIntermAggregate(EOpSequence);

	node->setLine(line);
	TIntermConstant* constIntNode;
	TNodeArray& nodes = node->getNodes();

	for (int i = 0; i < fields.num; i++)
	{
		TIntermConstant* constant = ir_add_constant(TType(EbtInt, EbpUndefined, EvqConst), line);
		constant->setValue(fields.offsets[i]);
		nodes.push_back(constant);
	}

	return node;
}


// This function returns the tree representation for the vector field(s) being accessed from contant vector.
// If only one component of vector is accessed (v.x or v[0] where v is a contant vector), then a contant node is
// returned, else an aggregate node is returned (for v.xy). The input to this function could either be the symbol
// node or it could be the intermediate tree representation of accessing fields in a constant structure or column of 
// a constant matrix.
TIntermTyped* ir_add_const_vector_swizzle(const TVectorFields& fields, TIntermTyped* node, TSourceLoc line)
{
	TIntermConstant* constNode = node->getAsConstant();
	if (!constNode)
		return NULL;
	
	TIntermConstant* res = ir_add_constant(node->getType(), line);
	for (int i = 0; i < fields.num; ++i)
	{
		int index = fields.offsets[i];
		assert(index >= 0 && index < constNode->getCount());
		res->setValue(i, constNode->getValue (index));
	}
	
	res->setType(TType(node->getBasicType(), node->getPrecision(), EvqConst, fields.num));
	
	return res;
}


TIntermTyped* ir_add_vector_swizzle(TVectorFields& fields, TIntermTyped* arg, TSourceLoc lineDot, TSourceLoc lineIndex)
{	
	// swizzle on a constant -> fold it
	if (arg->getType().getQualifier() == EvqConst)
	{
		TIntermTyped* res = ir_add_const_vector_swizzle(fields, arg, lineIndex);
		if (res)
			return res;
	}
		
	TIntermTyped* res = NULL;
	if (fields.num == 1)
	{
		TIntermConstant* index = ir_add_constant(TType(EbtInt, EbpUndefined, EvqConst), lineIndex);
		index->setValue(fields.offsets[0]);
		res = ir_add_index(EOpIndexDirect, arg, index, lineDot);
		res->setType(TType(arg->getBasicType(), arg->getPrecision()));
	}
	else
	{
		TIntermTyped* index = ir_add_swizzle(fields, lineIndex);
		res = ir_add_index(EOpVectorSwizzle, arg, index, lineDot);
		res->setType(TType(arg->getBasicType(), arg->getPrecision(), EvqTemporary, 1, fields.num));
	}
	return res;
}



// Create loop nodes
TIntermNode* ir_add_loop(TLoopType type, TIntermTyped* cond, TIntermTyped* expr, TIntermNode* body, TSourceLoc line)
{
	TIntermNode* node = new TIntermLoop(type, cond, expr, body);
	node->setLine(line);
	return node;
}


// Add branches.
TIntermBranch* ir_add_branch(TOperator branchOp, TSourceLoc line)
{
   return ir_add_branch(branchOp, 0, line);
}

TIntermBranch* ir_add_branch(TOperator branchOp, TIntermTyped* expression, TSourceLoc line)
{
	TIntermBranch* node = new TIntermBranch(branchOp, expression);
	node->setLine(line);
	return node;
}



// ------------------------------------------------------------------
// Member functions of the nodes used for building the tree.


//
// Say whether or not an operation node changes the value of a variable.
//
// Returns true if state is modified.
//
bool TIntermOperator::modifiesState() const
{
   switch (op)
   {
   case EOpPostIncrement:
   case EOpPostDecrement:
   case EOpPreIncrement:
   case EOpPreDecrement:
   case EOpAssign:
   case EOpAddAssign:
   case EOpSubAssign:
   case EOpMulAssign:
   case EOpVectorTimesMatrixAssign:
   case EOpVectorTimesScalarAssign:
   case EOpMatrixTimesScalarAssign:
   case EOpMatrixTimesMatrixAssign:
   case EOpDivAssign:
   case EOpModAssign:
   case EOpAndAssign:
   case EOpInclusiveOrAssign:
   case EOpExclusiveOrAssign:
   case EOpLeftShiftAssign:
   case EOpRightShiftAssign:
      return true;
   default:
      return false;
   }
}

//
// returns true if the operator is for one of the constructors
//
bool TIntermOperator::isConstructor() const
{
   switch (op)
   {
   case EOpConstructVec2:
   case EOpConstructVec3:
   case EOpConstructVec4:
   case EOpConstructMat2x2:
   case EOpConstructMat2x3:
   case EOpConstructMat2x4:
   case EOpConstructMat3x2:
   case EOpConstructMat3x3:
   case EOpConstructMat3x4:
   case EOpConstructMat4x2:
   case EOpConstructMat4x3:
   case EOpConstructMat4x4:
   case EOpConstructFloat:
   case EOpConstructIVec2:
   case EOpConstructIVec3:
   case EOpConstructIVec4:
   case EOpConstructInt:
   case EOpConstructBVec2:
   case EOpConstructBVec3:
   case EOpConstructBVec4:
   case EOpConstructBool:
   case EOpConstructStruct:
      return true;
   default:
      return false;
   }
}
//
// Make sure the type of a unary operator is appropriate for its
// combination of operation and operand type.
//
// Returns false in nothing makes sense.
//
bool TIntermUnary::promote(TInfoSink&)
{
   switch (op)
   {
   case EOpLogicalNot:
      if (operand->getBasicType() != EbtBool)
         return false;
      break;
   case EOpBitwiseNot:
      if (operand->getBasicType() != EbtInt)
         return false;
      break;
   case EOpNegative:
   case EOpPostIncrement:
   case EOpPostDecrement:
   case EOpPreIncrement:
   case EOpPreDecrement:
      if (operand->getBasicType() == EbtBool)
         return false;
      break;

      // operators for built-ins are already type checked against their prototype
   case EOpAny:
   case EOpAll:
   case EOpVectorLogicalNot:
      return true;

   default:
      if (operand->getBasicType() != EbtFloat)
         return false;
   }

   setType(operand->getType());

   return true;
}

static TOperator getMatrixConstructOp(const TIntermTyped& intermediate)
{
    switch (intermediate.getColsCount())
    {
    case 2:
        switch (intermediate.getRowsCount())
        {
        case 2: return EOpConstructMat2x2;
        case 3: return EOpConstructMat2x3;
        case 4: return EOpConstructMat2x4;
        } break;
    case 3:
        switch (intermediate.getRowsCount())
        {
        case 2: return EOpConstructMat3x2;
        case 3: return EOpConstructMat3x3;
        case 4: return EOpConstructMat3x4;
        } break;
    case 4:
        switch (intermediate.getRowsCount())
        {
        case 2: return EOpConstructMat4x2;
        case 3: return EOpConstructMat4x3;
        case 4: return EOpConstructMat4x4;
        } break;
    }
    assert(false);
    return EOpNull;
}

static TOperator getDownConvertOp(const TIntermTyped& intermediate)
{
    return getMatrixConstructOp(intermediate);
    /* implement EOpConstructMat?x?FromMat if support for glsl <1.20 is needed,
    switch (intermediate.getColsCount())
    {
    case 2:
        switch (intermediate.getRowsCount())
        {
        case 2: return EOpConstructMat2x2FromMat;
        case 3: return EOpConstructMat2x3FromMat;
        case 4: return EOpConstructMat2x4FromMat;
        } break;
    case 3:
        switch (intermediate.getRowsCount())
        {
        case 2: return EOpConstructMat3x2FromMat;
        case 3: return EOpConstructMat3x3FromMat;
        case 4: return EOpConstructMat3x4FromMat;
        } break;
    case 4:
        switch (intermediate.getRowsCount())
        {
        case 2: return EOpConstructMat4x2FromMat;
        case 3: return EOpConstructMat4x3FromMat;
        case 4: return EOpConstructMat4x4; //should never need to down convert to mat4
        } break;
    }
    assert(false);
    return EOpNull;
    */
}


//
// Establishes the type of the resultant operation, as well as
// makes the operator the correct one for the operands.
//
// Returns false if operator can't work on operands.
//
bool TIntermBinary::promote(TInfoSink& infoSink)
{
   TBasicType type = left->getBasicType();

   //
   // Arrays have to be exact matches.
   //
   if ((left->isArray() || right->isArray()) && (left->getType() != right->getType()))
      return false;

   //
   // Base assumption:  just make the type the same as the left
   // operand.  Then only deviations from this need be coded.
   //
   setType(TType(type, left->getPrecision(), EvqTemporary, left->getColsCount(), left->getRowsCount(), left->isMatrix()));

   // The result gets promoted to the highest precision.
   TPrecision higherPrecision = GetHigherPrecision(left->getPrecision(), right->getPrecision());
   getTypePointer()->setPrecision(higherPrecision);


   //
   // Array operations.
   //
   if (left->isArray())
   {

      switch (op)
      {

      //
      // Promote to conditional
      //
      case EOpEqual:
      case EOpNotEqual:
         setType(TType(EbtBool, EbpUndefined));
         break;

         //
         // Set array information.
         //
      case EOpAssign:
         getType().setArraySize(left->getType().getArraySize());
         getType().setArrayInformationType(left->getType().getArrayInformationType());
         break;

      default:
         return false;
      }

      return true;
   }

   //
   // All scalars.  Code after this test assumes this case is removed!
   //
   if (left->isScalar() && right->isScalar())
   {

      switch (op)
      {

      //
      // Promote to conditional
      //
      case EOpEqual:
      case EOpNotEqual:
      case EOpLessThan:
      case EOpGreaterThan:
      case EOpLessThanEqual:
      case EOpGreaterThanEqual:
         setType(TType(EbtBool, EbpUndefined));
         break;

         //
         // And and Or operate on conditionals
         //
      case EOpLogicalAnd:
      case EOpLogicalOr:
         if (left->getBasicType() != EbtBool || right->getBasicType() != EbtBool)
            return false;
         setType(TType(EbtBool, EbpUndefined));
         break;

         //
         // Check for integer only operands.
         //
      case EOpRightShift:
      case EOpLeftShift:
      case EOpAnd:
      case EOpInclusiveOr:
      case EOpExclusiveOr:
         if (left->getBasicType() != EbtInt || right->getBasicType() != EbtInt)
            return false;
         break;
      case EOpModAssign:
      case EOpAndAssign:
      case EOpInclusiveOrAssign:
      case EOpExclusiveOrAssign:
      case EOpLeftShiftAssign:
      case EOpRightShiftAssign:
         if (left->getBasicType() != EbtInt || right->getBasicType() != EbtInt)
            return false;
         // fall through

         //
         // Everything else should have matching types
         //
      default:
         if (left->getBasicType() != right->getBasicType() ||
             left->isMatrix()     != right->isMatrix())
            return false;
      }

      return true;
   }

   // this is not an allowed promotion : float3x4 * float4x3
   if (left->getRowsCount() != right->getRowsCount() && left->getColsCount() != right->getColsCount() &&
       (left->getRowsCount() > right->getRowsCount()) != (left->getColsCount() > right->getColsCount()))
       return false;

   //determine if this is an assignment
   bool assignment = ( op >= EOpAssign && op <= EOpRightShiftAssign) ? true : false;

   // find size of the resulting value
   int cols = 0;
   int rows = 0;

   if (!left->isScalar() && !right->isScalar()) // no scalars, so downcast of the larger type
   {
       cols = std::min(left->getColsCount(), right->getColsCount());
       rows = std::min(left->getRowsCount(), right->getRowsCount());
   }
   else
   {
       cols = std::max(left->getColsCount(), right->getColsCount());
       rows = std::max(left->getRowsCount(), right->getRowsCount());
   }
   assert(cols > 0);
   assert(rows > 0);

   //
   // Downcast needed ?
   //
   if ( left->getColsCount() > cols || left->getRowsCount() > rows)
   {
       if (assignment)
           return false; //can't promote the destination

       //down convert left to match right
       TOperator convert = EOpNull;
       if (left->getTypePointer()->isMatrix())
       {
           convert = getDownConvertOp(*right);
       }
       else if (left->getTypePointer()->isVector())
       {
           switch (right->getTypePointer()->getBasicType())
           {
           case EbtBool:  convert = TOperator( EOpConstructBVec2 + rows - 2); break;
           case EbtInt:   convert = TOperator( EOpConstructIVec2 + rows - 2); break;
           case EbtFloat: convert = TOperator( EOpConstructVec2 +  rows - 2); break;
           }
       }
       else
       {
           assert(0); //size 1 case should have been handled
       }
       TIntermAggregate *node = new TIntermAggregate(convert);
       node->setLine(left->getLine());
       node->setType(TType(left->getBasicType(), left->getPrecision(), EvqTemporary,
                           right->getColsCount(), right->getRowsCount(), left->isMatrix()));
       node->getNodes().push_back(left);
       left = node;
       //now reset this node's type
       setType(TType(left->getBasicType(), left->getPrecision(), EvqTemporary,
                     right->getColsCount(), right->getRowsCount(), left->isMatrix()));
   }
   else if ( right->getColsCount() > cols || right->getRowsCount() > rows)
   {
       //down convert right to match left
       TOperator convert = EOpNull;
       if (right->getTypePointer()->isMatrix())
       {
           convert = getDownConvertOp(*left);
       }
       else if (right->getTypePointer()->isVector())
       {
           switch (left->getTypePointer()->getBasicType())
           {
           case EbtBool:  convert = TOperator( EOpConstructBVec2 + rows - 2); break;
           case EbtInt:   convert = TOperator( EOpConstructIVec2 + rows - 2); break;
           case EbtFloat: convert = TOperator( EOpConstructVec2  + rows - 2); break;
           }
       }
       else
       {
           assert(0); //size 1 case should have been handled
       }
       TIntermAggregate *node = new TIntermAggregate(convert);
       node->setLine(right->getLine());
       node->setType(TType(right->getBasicType(), right->getPrecision(), EvqTemporary,
                           left->getColsCount(), left->getRowsCount(), right->isMatrix()));
       node->getNodes().push_back(right);
       right = node;
   }

   //
   // Can these two operands be combined?
   //
   switch (op)
   {
   case EOpMul:
      if (!left->isMatrix() && right->isMatrix())
      {
         if (left->isVector())
            op = EOpVectorTimesMatrix;
         else
         {
            op = EOpMatrixTimesScalar;
            setType(TType(type, higherPrecision, EvqTemporary, cols, rows, true));
         }
      }
      else if (left->isMatrix() && !right->isMatrix())
      {
         if (right->isVector())
         {
            op = EOpMatrixTimesVector;
            setType(TType(type, higherPrecision, EvqTemporary, cols, rows, false));
         }
         else
         {
            op = EOpMatrixTimesScalar;
         }
      }
      else if (left->isMatrix() && right->isMatrix())
      {
         op = EOpMatrixTimesMatrix;
      }
      else if (!left->isMatrix() && !right->isMatrix())
      {
         if (left->isVector() && right->isVector())
         {
            // leave as component product
         }
         else if (left->isVector() || right->isVector())
         {
            op = EOpVectorTimesScalar;
            setType(TType(type, higherPrecision, EvqTemporary, cols, rows, false));
         }
      }
      else
      {
         infoSink.info.message(EPrefixInternalError, "Missing elses", getLine());
         return false;
      }
      break;
   case EOpMulAssign:
      if (!left->isMatrix() && right->isMatrix())
      {
         if (left->isVector())
            op = EOpVectorTimesMatrixAssign;
         else
         {
            return false;
         }
      }
      else if (left->isMatrix() && !right->isMatrix())
      {
         if (right->isVector())
         {
            return false;
         }
         else
         {
            op = EOpMatrixTimesScalarAssign;
         }
      }
      else if (left->isMatrix() && right->isMatrix())
      {
         op = EOpMatrixTimesMatrixAssign;
      }
      else if (!left->isMatrix() && !right->isMatrix())
      {
         if (left->isVector() && right->isVector())
         {
            // leave as component product
         }
         else if (left->isVector() || right->isVector())
         {
            if (! left->isVector())
               return false;
            op = EOpVectorTimesScalarAssign;
            setType(TType(type, higherPrecision, EvqTemporary, cols, rows, false));
         }
      }
      else
      {
         infoSink.info.message(EPrefixInternalError, "Missing elses", getLine());
         return false;
      }
      break;
   case EOpAssign:
      if ( left->getColsCount() != right->getColsCount() ||
           left->getRowsCount() != right->getRowsCount())
      {
         //right needs to be forced to match left
         TOperator convert = EOpNull;

         if (left->isMatrix() )
         {
             convert = getMatrixConstructOp(*left);
         }
         else if (left->isVector() )
         {
            switch (left->getTypePointer()->getBasicType())
            {
            case EbtBool:  convert = TOperator( EOpConstructBVec2 + left->getRowsCount() - 2); break;
            case EbtInt:   convert = TOperator( EOpConstructIVec2 + left->getRowsCount() - 2); break;
            case EbtFloat: convert = TOperator( EOpConstructVec2  + left->getRowsCount() - 2); break;
            }
         }
         else
         {
            switch (left->getTypePointer()->getBasicType())
            {
            case EbtBool:  convert = EOpConstructBool; break;
            case EbtInt:   convert = EOpConstructInt; break;
            case EbtFloat: convert = EOpConstructFloat; break;
            }
         }

         assert( convert != EOpNull);
         TIntermAggregate *node = new TIntermAggregate(convert);
         node->setLine(right->getLine());
         node->setType(TType(left->getBasicType(), left->getPrecision(), right->getQualifier() == EvqConst ? EvqConst : EvqTemporary,
                             left->getColsCount(), left->getRowsCount(), left->isMatrix()));
         node->getNodes().push_back(right);
         right = node;
         cols = right->getColsCount();
         rows = right->getRowsCount();
      }
      // fall through
   case EOpMod:
   case EOpAdd:
   case EOpSub:
   case EOpDiv:
   case EOpAddAssign:
   case EOpSubAssign:
   case EOpDivAssign:
   case EOpModAssign:
      if (op == EOpMod)
		  type = EbtFloat;
      if (left->isMatrix() && right->isVector() ||
          left->isVector() && right->isMatrix() ||
          left->getBasicType() != right->getBasicType())
         return false;
      setType(TType(type, left->getPrecision(), EvqTemporary, cols, rows, left->isMatrix() || right->isMatrix()));
      break;

   case EOpEqual:
   case EOpNotEqual:
   case EOpLessThan:
   case EOpGreaterThan:
   case EOpLessThanEqual:
   case EOpGreaterThanEqual:
      if (left->isMatrix() && right->isVector() ||
          left->isVector() && right->isMatrix() ||
          left->getBasicType() != right->getBasicType())
         return false;
      setType(TType(EbtBool, higherPrecision, EvqTemporary, cols, rows, false));
      break;

   default:
      return false;
   }

   //
   // One more check for assignment.  The Resulting type has to match the left operand.
   //
   switch (op)
   {
   case EOpAssign:
   case EOpAddAssign:
   case EOpSubAssign:
   case EOpMulAssign:
   case EOpDivAssign:
   case EOpModAssign:
   case EOpAndAssign:
   case EOpInclusiveOrAssign:
   case EOpExclusiveOrAssign:
   case EOpLeftShiftAssign:
   case EOpRightShiftAssign:
      if (getType() != left->getType())
         return false;
      break;
   default:
      break;
   }

   return true;
}


bool TIntermSelection::promoteTernary(TInfoSink& infoSink)
{
	if (!condition->isVector())
		return true;
	
	int size = condition->getRowsCount();
	TIntermTyped* trueb = trueBlock->getAsTyped();
	TIntermTyped* falseb = falseBlock->getAsTyped();
	if (!trueb || !falseb)
		return false;
	
	if (trueb->getRowsCount() == size && falseb->getRowsCount() == size)
		return true;
	
	// Base assumption: just make the type a float vector
	TPrecision higherPrecision = GetHigherPrecision(trueb->getPrecision(), falseb->getPrecision());
	setType(TType(EbtFloat, higherPrecision, EvqTemporary, 1, size, condition->isMatrix()));
	
	TOperator convert = EOpNull;	
	{
		convert = TOperator( EOpConstructVec2 + size - 2);
		TIntermAggregate *node = new TIntermAggregate(convert);
		node->setLine(trueb->getLine());
		node->setType(TType(condition->getBasicType(), higherPrecision, trueb->getQualifier() == EvqConst ? EvqConst : EvqTemporary, 1, size, condition->isMatrix()));
		node->getNodes().push_back(trueb);
		trueBlock = node;
	}
	{
		convert = TOperator( EOpConstructVec2 + size - 2);
		TIntermAggregate *node = new TIntermAggregate(convert);
		node->setLine(falseb->getLine());
		node->setType(TType(condition->getBasicType(), higherPrecision, falseb->getQualifier() == EvqConst ? EvqConst : EvqTemporary, 1, size, condition->isMatrix()));
		node->getNodes().push_back(falseb);
		falseBlock = node;
	}
	
	return true;
}

TIntermTyped* ir_promote_constant(TBasicType promoteTo, TIntermConstant* right, TInfoSink& infoSink)
{
	unsigned size = right->getCount();
	const TType& t = right->getType();
	TIntermConstant* left = ir_add_constant(TType(promoteTo, t.getPrecision(), t.getQualifier(), t.getColsCount(), t.getRowsCount(), t.isMatrix(), t.isArray()), right->getLine());
	for (unsigned i = 0; i != size; ++i) {
		TIntermConstant::Value& value = right->getValue(i);
		
		switch (promoteTo)
		{
		case EbtFloat:
			switch (value.type) {
			case EbtInt:
				left->setValue(i, (float)value.asInt);
				break;
			case EbtBool:
				left->setValue(i, (float)value.asBool);
				break;
			case EbtFloat:
				left->setValue(i, value.asFloat);
				break;
			default: 
				infoSink.info.message(EPrefixInternalError, "Cannot promote", right->getLine());
				return 0;
			}                
			break;
		case EbtInt:
			switch (value.type) {
			case EbtInt:
				left->setValue(i, value.asInt);
				break;
			case EbtBool:
				left->setValue(i, (int)value.asBool);
				break;
			case EbtFloat:
				left->setValue(i, (int)value.asFloat);
				break;
			default: 
				infoSink.info.message(EPrefixInternalError, "Cannot promote", right->getLine());
				return 0;
			}                
			break;
		case EbtBool:
			switch (value.type) {
			case EbtInt:
				left->setValue(i, value.asInt != 0);
				break;
			case EbtBool:
				left->setValue(i, value.asBool);
				break;
			case EbtFloat:
				left->setValue(i, value.asFloat != 0.0f);
				break;
			default: 
				infoSink.info.message(EPrefixInternalError, "Cannot promote", right->getLine());
				return 0;
			}                
			break;
		default:
			infoSink.info.message(EPrefixInternalError, "Incorrect data type found", right->getLine());
			return 0;
		}
	}

	return left;
}
