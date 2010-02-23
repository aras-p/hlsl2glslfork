//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
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
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
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

//
// Build the intermediate representation.
//

#include "localintermediate.h"
#include "QualifierAlive.h"
#include "RemoveTree.h"
#include <float.h>

bool CompareStructure(const TType& leftNodeType, constUnion* rightUnionArray, constUnion* leftUnionArray);

////////////////////////////////////////////////////////////////////////////
//
// First set of functions are to help build the intermediate representation.
// These functions are not member functions of the nodes.
// They are called from parser productions.
//
/////////////////////////////////////////////////////////////////////////////

//
// Add a terminal node for an identifier in an expression.
//
// Returns the added node.
//
TIntermSymbol* TIntermediate::addSymbol(int id, const TString& name, const TType& type, TSourceLoc line)
{
   TIntermSymbol* node = new TIntermSymbol(id, name, type);
   node->setLine(line);

   return node;
}

//
// Add a terminal node for an identifier in an expression.
//
// Returns the added node.
//
TIntermSymbol* TIntermediate::addSymbol(int id, const TString& name, const TTypeInfo *info, const TType& type, TSourceLoc line)
{
   TIntermSymbol* node = new TIntermSymbol(id, name, info, type);
   node->setLine(line);

   return node;
}

//
// Connect two nodes with a new parent that does a binary operation on the nodes.
//
// Returns the added node.
//
TIntermTyped* TIntermediate::addBinaryMath(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc line, TSymbolTable& symbolTable)
{
   if ( !left || !right)
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
      if (left->getType().isMatrix() || left->getType().isArray() || left->getType().isVector())
         return 0;

      if ( left->getBasicType() != EbtBool )
      {
         if ( left->getType().getBasicType() != EbtInt && left->getType().getBasicType() != EbtFloat )
            return 0;
         else
         {
            // If the left is a float or int, convert to a bool.  This is the conversion that HLSL
            // does
            left = addConversion( EOpConstructBool, 
                                  TType ( EbtBool, left->getQualifier(),
                                          left->getNominalSize(), left->isMatrix(), left->isArray()), 
                                  left );

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
            right = addConversion( EOpConstructBool, 
                                   TType ( EbtBool, right->getQualifier(),
                                           right->getNominalSize(), right->isMatrix(), right->isArray()), 
                                   right );

            if ( right == 0 )
               return 0;
         }

      }
      break;
   case EOpAdd:
   case EOpSub:
   case EOpDiv:
   case EOpMul:
      if (left->getType().getBasicType() == EbtStruct || left->getType().getBasicType() == EbtBool)
         return 0;
   default: break; 
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
         child = addConversion(op, left->getType(), right);
         if (child)
            right = child;
         else
         {
            child = addConversion(op, right->getType(), left);
            if (child)
               left = child;
            else
               return 0;
         }
      }
      else
      {
         child = addConversion(op, right->getType(), left);
         if (child)
            left = child;
         else
         {
            child = addConversion(op, left->getType(), right);
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
   if (line == 0)
      line = right->getLine();
   node->setLine(line);

   node->setLeft(left);
   node->setRight(right);
   if (! node->promote(infoSink))
      return 0;

   TIntermConstantUnion *leftTempConstant = left->getAsConstantUnion();
   TIntermConstantUnion *rightTempConstant = right->getAsConstantUnion();
 
   if (leftTempConstant)
      leftTempConstant = left->getAsConstantUnion();

   if (rightTempConstant)
      rightTempConstant = right->getAsConstantUnion();

   //
   // See if we can fold constants.
   //

   TIntermTyped* typedReturnNode = 0;
   if ( leftTempConstant && rightTempConstant)
   {
      typedReturnNode = leftTempConstant->fold(node->getOp(), rightTempConstant, infoSink);
      
      if (typedReturnNode)
         return typedReturnNode;
   }

   return node;
}

//
// Connect two nodes through an assignment.
//
// Returns the added node.
//
TIntermTyped* TIntermediate::addAssign(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc line)
{
   //
   // Like adding binary math, except the conversion can only go
   // from right to left.
   //
   TIntermBinary* node = new TIntermBinary(op);
   if (line == 0)
      line = left->getLine();
   node->setLine(line);

   TIntermTyped* child = addConversion(op, left->getType(), right);
   if (child == 0)
      return 0;

   node->setLeft(left);
   node->setRight(child);
   if (! node->promote(infoSink))
      return 0;

   return node;
}

//
// Connect two nodes through an index operator, where the left node is the base
// of an array or struct, and the right node is a direct or indirect offset.
//
// Returns the added node.
// The caller should set the type of the returned node.
//
TIntermTyped* TIntermediate::addIndex(TOperator op, TIntermTyped* base, TIntermTyped* index, TSourceLoc line)
{
   TIntermBinary* node = new TIntermBinary(op);
   if (line == 0)
      line = index->getLine();
   node->setLine(line);
   node->setLeft(base);
   node->setRight(index);

   // caller should set the type

   return node;
}

//
// Add one node as the parent of another that it operates on.
//
// Returns the added node.
//
TIntermTyped* TIntermediate::addUnaryMath(TOperator op, TIntermNode* childNode, TSourceLoc line, TSymbolTable& symbolTable)
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
      if (child->getType().getBasicType() != EbtBool || child->getType().isMatrix() || child->getType().isArray() || child->getType().isVector())
      {
         return 0;
      }
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
   default: break;
   }

   if (newType != EbtVoid)
   {
      child = addConversion(op, TType(newType, EvqTemporary, child->getNominalSize(), 
                                      child->isMatrix(), 
                                      child->isArray()),
                            child);
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

   TIntermConstantUnion *childTempConstant = 0;
   if (child->getAsConstantUnion())
      childTempConstant = child->getAsConstantUnion();

   //
   // Make a new node for the operator.
   //
   node = new TIntermUnary(op);
   if (line == 0)
      line = child->getLine();
   node->setLine(line);
   node->setOperand(child);

   if (! node->promote(infoSink))
      return 0;

   if (childTempConstant)
   {
      TIntermTyped* newChild = childTempConstant->fold(op, 0, infoSink);

      if (newChild)
         return newChild;
   }

   return node;
}

//
// This is the safe way to change the operator on an aggregate, as it
// does lots of error checking and fixing.  Especially for establishing
// a function call's operation on it's set of parameters.  Sequences
// of instructions are also aggregates, but they just direnctly set
// their operator to EOpSequence.
//
// Returns an aggregate node, which could be the one passed in if
// it was already an aggregate.
//
TIntermAggregate* TIntermediate::setAggregateOperator(TIntermNode* node, TOperator op, TSourceLoc line)
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
         aggNode->getSequence().push_back(node);
         if (line == 0)
            line = node->getLine();
      }
   }
   else
      aggNode = new TIntermAggregate();

   //
   // Set the operator.
   //
   aggNode->setOperator(op);
   if (line != 0)
      aggNode->setLine(line);

   return aggNode;
}

//
// Convert one type to another.
//
// Returns the node representing the conversion, which could be the same
// node passed in if no conversion was needed.
//
// Return 0 if a conversion can't be done.
//
TIntermTyped* TIntermediate::addConversion(TOperator op, const TType& type, TIntermTyped* node)
{
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

   if (node->getAsConstantUnion())
   {

      return(promoteConstantUnion(promoteTo, node->getAsConstantUnion()));
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

      TType type(promoteTo, EvqTemporary, node->getNominalSize(), node->isMatrix(), node->isArray());
      newNode = new TIntermUnary(newOp, type);
      newNode->setLine(node->getLine());
      newNode->setOperand(node);

      return newNode;
   }
}

//
// Safe way to combine two nodes into an aggregate.  Works with null pointers, 
// a node that's not a aggregate yet, etc.
//
// Returns the resulting aggregate, unless 0 was passed in for 
// both existing nodes.
//
TIntermAggregate* TIntermediate::growAggregate(TIntermNode* left, TIntermNode* right, TSourceLoc line)
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
         aggNode->getSequence().push_back(left);
   }

   if (right)
      aggNode->getSequence().push_back(right);

   if (line != 0)
      aggNode->setLine(line);

   return aggNode;
}

//
// Turn an existing node into an aggregate.
//
// Returns an aggregate, unless 0 was passed in for the existing node.
//
TIntermAggregate* TIntermediate::makeAggregate(TIntermNode* node, TSourceLoc line)
{
   if (node == 0)
      return 0;

   TIntermAggregate* aggNode = new TIntermAggregate;
   aggNode->getSequence().push_back(node);

   if (line != 0)
      aggNode->setLine(line);
   else
      aggNode->setLine(node->getLine());

   return aggNode;
}

//
// For "if" test nodes.  There are three children; a condition,
// a true path, and a false path.  The two paths are in the
// nodePair.
//
// Returns the selection node created.
//
TIntermNode* TIntermediate::addSelection(TIntermTyped* cond, TIntermNodePair nodePair, TSourceLoc line)
{   
   // Convert float/int to bool
   switch ( cond->getBasicType() )
   {
   case EbtFloat:
   case EbtInt:
      cond = addConversion ( EOpConstructBool, 
                             TType (EbtBool, cond->getQualifier(), cond->getNominalSize(), cond->isMatrix(), cond->isArray()),
                             cond );
      break;
   default:
      // Do nothing
      break;
   }

   TIntermSelection* node = new TIntermSelection(cond, nodePair.node1, nodePair.node2);
   node->setLine(line);

   return node;
}


TIntermTyped* TIntermediate::addComma(TIntermTyped* left, TIntermTyped* right, TSourceLoc line)
{
   if (left->getType().getQualifier() == EvqConst && right->getType().getQualifier() == EvqConst)
   {
      return right;
   }
   else
   {
      TIntermTyped *commaAggregate = growAggregate(left, right, line);
      commaAggregate->getAsAggregate()->setOperator(EOpComma);    
      commaAggregate->setType(right->getType());
      commaAggregate->getTypePointer()->changeQualifier(EvqTemporary);
      return commaAggregate;
   }
}

//
// For "?:" test nodes.  There are three children; a condition,
// a true path, and a false path.  The two paths are specified
// as separate parameters.
//
// Returns the selection node created, or 0 if one could not be.
//
TIntermTyped* TIntermediate::addSelection(TIntermTyped* cond, TIntermTyped* trueBlock, TIntermTyped* falseBlock, TSourceLoc line)
{
   bool bPromoteFromTrueBlockType = true;

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
      TIntermTyped* child = addConversion(EOpSequence, trueBlock->getType(), falseBlock);
      if (child)
         falseBlock = child;
      else
      {
         child = addConversion(EOpSequence, falseBlock->getType(), trueBlock);
         if (child)
            trueBlock = child;
         else
            return 0;
      }
   }
   else
   {
      TIntermTyped* child = addConversion(EOpSequence, falseBlock->getType(), trueBlock);
      if (child)
         trueBlock = child;
      else
      {
         child = addConversion(EOpSequence, trueBlock->getType(), falseBlock);
         if (child)
            falseBlock = child;
         else
            return 0;
      }
   }

   //
   // See if all the operands are constant, then fold it otherwise not.
   //

   if (cond->getAsConstantUnion() && trueBlock->getAsConstantUnion() && falseBlock->getAsConstantUnion())
   {
      if (cond->getAsConstantUnion()->getUnionArrayPointer()->getBConst())
         return trueBlock;
      else
         return falseBlock;
   }

   //
   // Make a selection node.
   //
   TIntermSelection* node = new TIntermSelection(cond, trueBlock, falseBlock, trueBlock->getType());
   node->setLine(line);

   return node;
}

//
// Constant terminal nodes.  Has a union that contains bool, float or int constants
//
// Returns the constant union node created.
//

TIntermConstantUnion* TIntermediate::addConstantUnion(constUnion* unionArrayPointer, const TType& t, TSourceLoc line)
{
   TIntermConstantUnion* node = new TIntermConstantUnion(unionArrayPointer, t);
   node->setLine(line);

   return node;
}

TIntermTyped* TIntermediate::addSwizzle(TVectorFields& fields, TSourceLoc line)
{

   TIntermAggregate* node = new TIntermAggregate(EOpSequence);

   node->setLine(line);
   TIntermConstantUnion* constIntNode;
   TIntermSequence &sequenceVector = node->getSequence();
   constUnion* unionArray;

   for (int i = 0; i < fields.num; i++)
   {
      unionArray = new constUnion[1];
      unionArray->setIConst(fields.offsets[i]);
      constIntNode = addConstantUnion(unionArray, TType(EbtInt, EvqConst), line);
      sequenceVector.push_back(constIntNode);
   }

   return node;
}

//
// Create loop nodes.
//
TIntermNode* TIntermediate::addLoop(TIntermNode* body, TIntermTyped* test, TIntermTyped* terminal, bool testFirst, TSourceLoc line)
{
   TIntermNode* node = new TIntermLoop(body, test, terminal, testFirst);
   node->setLine(line);

   return node;
}

//
// Add branches.
//
TIntermBranch* TIntermediate::addBranch(TOperator branchOp, TSourceLoc line)
{
   return addBranch(branchOp, 0, line);
}

TIntermBranch* TIntermediate::addBranch(TOperator branchOp, TIntermTyped* expression, TSourceLoc line)
{
   TIntermBranch* node = new TIntermBranch(branchOp, expression);
   node->setLine(line);

   return node;
}

//
// This is to be executed once the final root is put on top by the parsing
// process.
//
bool TIntermediate::postProcess(TIntermNode* root, EShLanguage language)
{
   if (root == 0)
      return true;

   //
   // First, finish off the top level sequence, if any
   //
   TIntermAggregate* aggRoot = root->getAsAggregate();
   if (aggRoot && aggRoot->getOp() == EOpNull)
      aggRoot->setOperator(EOpSequence);

   return true;
}

//
// This deletes the tree.
//
void TIntermediate::remove(TIntermNode* root)
{
   if (root)
      RemoveAllTreeNodes(root);
}

////////////////////////////////////////////////////////////////
//
// Member functions of the nodes used for building the tree.
//
////////////////////////////////////////////////////////////////

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
   case EOpConstructMat2:
   case EOpConstructMat3:
   case EOpConstructMat4:
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

//
// Establishes the type of the resultant operation, as well as
// makes the operator the correct one for the operands.
//
// Returns false if operator can't work on operands.
//
bool TIntermBinary::promote(TInfoSink& infoSink)
{
   int size = left->getNominalSize();
   if (right->getNominalSize() < size)
      size = right->getNominalSize();

   if (size == 1)
   {
      size = left->getNominalSize();
      if (right->getNominalSize() > size)
         size = right->getNominalSize();
   }

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
   setType(TType(type, EvqTemporary, left->getNominalSize(), left->isMatrix()));

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
         setType(TType(EbtBool));
         break;

         //
         // Set array information.
         //
      case EOpAssign:
      case EOpInitialize:
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
   if (size == 1)
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
         setType(TType(EbtBool));
         break;

         //
         // And and Or operate on conditionals
         //
      case EOpLogicalAnd:
      case EOpLogicalOr:
         if (left->getBasicType() != EbtBool || right->getBasicType() != EbtBool)
            return false;
         setType(TType(EbtBool));
         break;

         //
         // Check for integer only operands.
         //
      case EOpMod:
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

   //determine if this is an assignment
   bool assignment = ( op >= EOpAssign && op <= EOpRightShiftAssign) ? true : false;

   //
   // Are the sizes compatible?
   //
   if ( left->getNominalSize() != size &&  left->getNominalSize() != 1 ||
        right->getNominalSize() != size && right->getNominalSize() != 1)
   {
      //Insert a constructor on the larger type to make the sizes match

      if ( left->getNominalSize() > right->getNominalSize() )
      {

         if (assignment)
            return false; //can't promote the destination

         //down convert left to match right
         TOperator convert = EOpNull;
         if (left->getTypePointer()->isMatrix())
         {
            switch (right->getNominalSize())
            {
            case 2: convert = EOpConstructMat2FromMat; break;
            case 3: convert = EOpConstructMat3FromMat; break;
            case 4: convert =  EOpConstructMat4; break; //should never need to down convert to mat4
            }
         }
         else if (left->getTypePointer()->isVector())
         {
            switch (left->getTypePointer()->getBasicType())
            {
            case EbtBool:  convert = TOperator( EOpConstructBVec2 + right->getNominalSize() - 2); break;
            case EbtInt:   convert = TOperator( EOpConstructIVec2 + right->getNominalSize() - 2); break;
            case EbtFloat: convert = TOperator( EOpConstructVec2 + right->getNominalSize() - 2); break;
            }
         }
         else
         {
            assert(0); //size 1 case should have been handled
         }
         TIntermAggregate *node = new TIntermAggregate(convert);
         node->setLine(left->getLine());
         node->setType(TType(left->getBasicType(), EvqTemporary, right->getNominalSize(), left->isMatrix()));
         node->getSequence().push_back(left);
         left = node;
         //now reset this node's type
         setType(TType(left->getBasicType(), EvqTemporary, right->getNominalSize(), left->isMatrix()));
      }
      else
      {
         //down convert right to match left
         TOperator convert = EOpNull;
         if (right->getTypePointer()->isMatrix())
         {
            switch (left->getNominalSize())
            {
            case 2: convert = EOpConstructMat2FromMat; break;
            case 3: convert = EOpConstructMat3FromMat; break;
            case 4: convert =  EOpConstructMat4; break; //should never need to down convert to mat4
            }
         }
         else if (right->getTypePointer()->isVector())
         {
            switch (right->getTypePointer()->getBasicType())
            {
            case EbtBool:  convert = TOperator( EOpConstructBVec2 + left->getNominalSize() - 2); break;
            case EbtInt:   convert = TOperator( EOpConstructIVec2 + left->getNominalSize() - 2); break;
            case EbtFloat: convert = TOperator( EOpConstructVec2 + left->getNominalSize() - 2); break;
            }
         }
         else
         {
            assert(0); //size 1 case should have been handled
         }
         TIntermAggregate *node = new TIntermAggregate(convert);
         node->setLine(right->getLine());
         node->setType(TType(right->getBasicType(), EvqTemporary, left->getNominalSize(), right->isMatrix()));
         node->getSequence().push_back(right);
         right = node;
      }
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
            setType(TType(type, EvqTemporary, size, true));
         }
      }
      else if (left->isMatrix() && !right->isMatrix())
      {
         if (right->isVector())
         {
            op = EOpMatrixTimesVector;
            setType(TType(type, EvqTemporary, size, false));
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
            setType(TType(type, EvqTemporary, size, false));
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
            setType(TType(type, EvqTemporary, size, false));
         }
      }
      else
      {
         infoSink.info.message(EPrefixInternalError, "Missing elses", getLine());
         return false;
      }
      break;

   case EOpAssign:
   case EOpInitialize:
      if (left->getNominalSize() != right->getNominalSize())
      {
         //right needs to be forced to match left
         TOperator convert = EOpNull;

         if (left->isMatrix() )
         {
            //TODO: These might need to be changed to smears
            switch (left->getNominalSize())
            {
            case 2: convert = EOpConstructMat2; break;
            case 3: convert = EOpConstructMat3; break;
            case 4: convert =  EOpConstructMat4; break; 
            }
         }
         else if (left->isVector() )
         {
            switch (right->getTypePointer()->getBasicType())
            {
            case EbtBool:  convert = TOperator( EOpConstructBVec2 + left->getNominalSize() - 2); break;
            case EbtInt:   convert = TOperator( EOpConstructIVec2 + left->getNominalSize() - 2); break;
            case EbtFloat: convert = TOperator( EOpConstructVec2 + left->getNominalSize() - 2); break;
            }
         }
         else
         {
            switch (right->getTypePointer()->getBasicType())
            {
            case EbtBool:  convert = EOpConstructBool; break;
            case EbtInt:   convert = EOpConstructInt; break;
            case EbtFloat: convert = EOpConstructFloat; break;
            }
         }

         assert( convert != EOpNull);
         TIntermAggregate *node = new TIntermAggregate(convert);
         node->setLine(right->getLine());
         node->setType(TType(left->getBasicType(), right->getQualifier() == EvqConst ? EvqConst : EvqTemporary, left->getNominalSize(), left->isMatrix()));
         node->getSequence().push_back(right);
         right = node;
         size = right->getNominalSize();
      }
      // fall through
   case EOpAdd:
   case EOpSub:
   case EOpDiv:
   case EOpMod:
   case EOpAddAssign:
   case EOpSubAssign:
   case EOpDivAssign:
   case EOpModAssign:
      if (left->isMatrix() && right->isVector() ||
          left->isVector() && right->isMatrix() ||
          left->getBasicType() != right->getBasicType())
         return false;
      setType(TType(type, EvqTemporary, size, left->isMatrix() || right->isMatrix()));
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
      setType(TType(EbtBool, EvqTemporary, size, false));
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

bool CompareStruct(const TType& leftNodeType, constUnion* rightUnionArray, constUnion* leftUnionArray)
{
   TTypeList* fields = leftNodeType.getStruct();

   size_t structSize = fields->size();
   int index = 0;

   for (size_t j = 0; j < structSize; j++)
   {
      int size = (*fields)[j].type->getObjectSize();
      for (int i = 0; i < size; i++)
      {
         if ((*fields)[j].type->getBasicType() == EbtStruct)
         {
            if (!CompareStructure(*(*fields)[j].type, &rightUnionArray[index], &leftUnionArray[index]))
               return false;
         }
         else
         {
            if (leftUnionArray[index] != rightUnionArray[index])
               return false;
            index++;
         }    

      }
   }
   return true;
} 

bool CompareStructure(const TType& leftNodeType, constUnion* rightUnionArray, constUnion* leftUnionArray)
{
   if (leftNodeType.isArray())
   {
      TType typeWithoutArrayness = leftNodeType;
      typeWithoutArrayness.clearArrayness();

      int arraySize = leftNodeType.getArraySize();

      for (int i = 0; i < arraySize; ++i)
      {
         int offset = typeWithoutArrayness.getObjectSize() * i;
         if (!CompareStruct(typeWithoutArrayness, &rightUnionArray[offset], &leftUnionArray[offset]))
            return false;
      }
   }
   else
      return CompareStruct(leftNodeType, rightUnionArray, leftUnionArray);    

   return true;
} 

//
// The fold functions see if an operation on a constant can be done in place,
// without generating run-time code.
//
// Returns the node to keep using, which may or may not be the node passed in.
//

TIntermTyped* TIntermConstantUnion::fold(TOperator op, TIntermTyped* constantNode, TInfoSink& infoSink)
{
   constUnion *unionArray = getUnionArrayPointer(); 
   int objectSize = getType().getObjectSize();

   if (constantNode)
   {  // binary operations
      TIntermConstantUnion *node = constantNode->getAsConstantUnion();
      constUnion *rightUnionArray = node->getUnionArrayPointer();
      TType returnType = getType();

      // for a case like float f = 1.2 + vec4(2,3,4,5);
      if (constantNode->getType().getObjectSize() == 1 && objectSize > 1)
      {
         rightUnionArray = new constUnion[objectSize];
         for (int i = 0; i < objectSize; ++i)
            rightUnionArray[i] = *node->getUnionArrayPointer(); 
         returnType = getType();
      }
      else if (constantNode->getType().getObjectSize() > 1 && objectSize == 1)
      {
         // for a case like float f = vec4(2,3,4,5) + 1.2;
         unionArray = new constUnion[constantNode->getType().getObjectSize()];
         for (int i = 0; i < constantNode->getType().getObjectSize(); ++i)
            unionArray[i] = *getUnionArrayPointer(); 
         returnType = node->getType();
         objectSize = constantNode->getType().getObjectSize();
      }

      constUnion* tempConstArray = 0;
      TIntermConstantUnion *tempNode;
      int index = 0;
      bool boolNodeFlag = false;
      switch (op)
      {
      case EOpAdd: 
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] + rightUnionArray[i];
         }
         break;
      case EOpSub: 
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] - rightUnionArray[i];
         }
         break;

      case EOpMul:
      case EOpVectorTimesScalar:
      case EOpMatrixTimesScalar: 
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] * rightUnionArray[i];
         }
         break;
      case EOpMatrixTimesMatrix:                
         if (getType().getBasicType() != EbtFloat || node->getBasicType() != EbtFloat)
         {
            infoSink.info.message(EPrefixInternalError, "Constant Folding cannot be done for matrix multiply", getLine());
            return 0;
         }
         {// support MSVC++6.0
            int size = getNominalSize();
            tempConstArray = new constUnion[size*size];
            for (int row = 0; row < size; row++)
            {
               for (int column = 0; column < size; column++)
               {
                  tempConstArray[size * column + row].setFConst(0.0f);
                  for (int i = 0; i < size; i++)
                  {
                     tempConstArray[size * column + row].setFConst(tempConstArray[size * column + row].getFConst() + unionArray[i * size + row].getFConst() * (rightUnionArray[column * size + i].getFConst())); 
                  }
               }
            }
         }
         break;
      case EOpDiv: 
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
            {
               switch (getType().getBasicType())
               {
               case EbtFloat: 
                  if (rightUnionArray[i] == 0.0f)
                  {
                     infoSink.info.message(EPrefixWarning, "Divide by zero error during constant folding", getLine());
                     tempConstArray[i].setFConst(FLT_MAX);
                  }
                  else
                     tempConstArray[i].setFConst(unionArray[i].getFConst() / rightUnionArray[i].getFConst());
                  break;

               case EbtInt:   
                  if (rightUnionArray[i] == 0)
                  {
                     infoSink.info.message(EPrefixWarning, "Divide by zero error during constant folding", getLine());
                     tempConstArray[i].setIConst(INT_MAX);
                  }
                  else
                     tempConstArray[i].setIConst(unionArray[i].getIConst() / rightUnionArray[i].getIConst());
                  break;            
               default: 
                  infoSink.info.message(EPrefixInternalError, "Constant folding cannot be done for \"/\"", getLine());
                  return 0;
               }
            }
         }
         break;

      case EOpMatrixTimesVector: 
         if (node->getBasicType() != EbtFloat)
         {
            infoSink.info.message(EPrefixInternalError, "Constant Folding cannot be done for matrix times vector", getLine());
            return 0;
         }
         tempConstArray = new constUnion[getNominalSize()];

         {// support MSVC++6.0                    
            for (int size = getNominalSize(), i = 0; i < size; i++)
            {
               tempConstArray[i].setFConst(0.0f);
               for (int j = 0; j < size; j++)
               {
                  tempConstArray[i].setFConst(tempConstArray[i].getFConst() + ((unionArray[j*size + i].getFConst()) * rightUnionArray[j].getFConst()));
               }
            }
         }

         tempNode = new TIntermConstantUnion(tempConstArray, node->getType());
         tempNode->setLine(getLine());

         return tempNode;                

      case EOpVectorTimesMatrix:
         if (getType().getBasicType() != EbtFloat)
         {
            infoSink.info.message(EPrefixInternalError, "Constant Folding cannot be done for vector times matrix", getLine());
            return 0;
         }

         tempConstArray = new constUnion[getNominalSize()];
         {// support MSVC++6.0
            for (int size = getNominalSize(), i = 0; i < size; i++)
            {
               tempConstArray[i].setFConst(0.0f);
               for (int j = 0; j < size; j++)
               {
                  tempConstArray[i].setFConst(tempConstArray[i].getFConst() + ((unionArray[j].getFConst()) * rightUnionArray[i*size + j].getFConst()));
               }
            }
         }
         break;

      case EOpMod:
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] % rightUnionArray[i];
         }
         break;

      case EOpRightShift:
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] >> rightUnionArray[i];
         }
         break;

      case EOpLeftShift:
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] << rightUnionArray[i];
         }
         break;

      case EOpAnd:
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] & rightUnionArray[i];
         }
         break;
      case EOpInclusiveOr:
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] | rightUnionArray[i];
         }
         break;
      case EOpExclusiveOr:
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] ^ rightUnionArray[i];
         }
         break;

      case EOpLogicalAnd: // this code is written for possible future use, will not get executed currently
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] && rightUnionArray[i];
         }
         break;

      case EOpLogicalOr: // this code is written for possible future use, will not get executed currently
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               tempConstArray[i] = unionArray[i] || rightUnionArray[i];
         }
         break;

      case EOpLogicalXor:  
         tempConstArray = new constUnion[objectSize];
         {// support MSVC++6.0
            for (int i = 0; i < objectSize; i++)
               switch (getType().getBasicType())
               {
               case EbtBool: tempConstArray[i].setBConst((unionArray[i] == rightUnionArray[i]) ? false : true); break;
               default: assert(false && "Default missing");
               }
         }
         break;

      case EOpLessThan:         
         assert(objectSize == 1);
         tempConstArray = new constUnion[1];
         tempConstArray->setBConst(*unionArray < *rightUnionArray);
         returnType = TType(EbtBool, EvqConst);
         break;
      case EOpGreaterThan:      
         assert(objectSize == 1);
         tempConstArray = new constUnion[1];
         tempConstArray->setBConst(*unionArray > *rightUnionArray);
         returnType = TType(EbtBool, EvqConst);
         break;
      case EOpLessThanEqual:
         {
            assert(objectSize == 1);
            constUnion constant;
            constant.setBConst(*unionArray > *rightUnionArray);
            tempConstArray = new constUnion[1];
            tempConstArray->setBConst(!constant.getBConst());
            returnType = TType(EbtBool, EvqConst);
            break;
         }
      case EOpGreaterThanEqual: 
         {
            assert(objectSize == 1);
            constUnion constant;
            constant.setBConst(*unionArray < *rightUnionArray);
            tempConstArray = new constUnion[1];
            tempConstArray->setBConst(!constant.getBConst());
            returnType = TType(EbtBool, EvqConst);
            break;
         }

      case EOpEqual: 
         if (getType().getBasicType() == EbtStruct)
         {
            if (!CompareStructure(node->getType(), node->getUnionArrayPointer(), unionArray))
               boolNodeFlag = true;
         }
         else
         {
            for (int i = 0; i < objectSize; i++)
            {
               if (unionArray[i] != rightUnionArray[i])
               {
                  boolNodeFlag = true;
                  break;  // break out of for loop
               }
            }
         }

         tempConstArray = new constUnion[1];
         if (!boolNodeFlag)
         {
            tempConstArray->setBConst(true);
         }
         else
         {
            tempConstArray->setBConst(false);
         }

         tempNode = new TIntermConstantUnion(tempConstArray, TType(EbtBool, EvqConst));
         tempNode->setLine(getLine());

         return tempNode;         

      case EOpNotEqual: 
         if (getType().getBasicType() == EbtStruct)
         {
            if (CompareStructure(node->getType(), node->getUnionArrayPointer(), unionArray))
               boolNodeFlag = true;
         }
         else
         {
            for (int i = 0; i < objectSize; i++)
            {
               if (unionArray[i] == rightUnionArray[i])
               {
                  boolNodeFlag = true;
                  break;  // break out of for loop
               }
            }
         }

         tempConstArray = new constUnion[1];
         if (!boolNodeFlag)
         {
            tempConstArray->setBConst(true);
         }
         else
         {
            tempConstArray->setBConst(false);
         }

         tempNode = new TIntermConstantUnion(tempConstArray, TType(EbtBool, EvqConst));
         tempNode->setLine(getLine());

         return tempNode;         

      default: 
         infoSink.info.message(EPrefixInternalError, "Invalid operator for constant folding", getLine());
         return 0;
      }
      tempNode = new TIntermConstantUnion(tempConstArray, returnType);
      tempNode->setLine(getLine());

      return tempNode;                
   }
   else
   {
      //
      // Do unary operations
      //
      TIntermConstantUnion *newNode = 0;
      constUnion* tempConstArray = new constUnion[objectSize];
      for (int i = 0; i < objectSize; i++)
      {
         switch (op)
         {
         case EOpNegative:                                       
            switch (getType().getBasicType())
            {
            case EbtFloat: tempConstArray[i].setFConst(-unionArray[i].getFConst()); break;
            case EbtInt:   tempConstArray[i].setIConst(-unionArray[i].getIConst()); break;
            default: 
               infoSink.info.message(EPrefixInternalError, "Unary operation not folded into constant", getLine());
               return 0;
            }
            break;
         case EOpLogicalNot: // this code is written for possible future use, will not get executed currently                                      
            switch (getType().getBasicType())
            {
            case EbtBool:  tempConstArray[i].setBConst(!unionArray[i].getBConst()); break;
            default: 
               infoSink.info.message(EPrefixInternalError, "Unary operation not folded into constant", getLine());
               return 0;
            }
            break;
         default: 
            return 0;
         }
      }
      newNode = new TIntermConstantUnion(tempConstArray, getType());
      newNode->setLine(getLine());
      return newNode;     
   }

   return this;
}

TIntermTyped* TIntermediate::promoteConstantUnion(TBasicType promoteTo, TIntermConstantUnion* node) 
{
   constUnion *rightUnionArray = node->getUnionArrayPointer();
   int size = node->getType().getObjectSize();

   constUnion *leftUnionArray = new constUnion[size];

   for (int i=0; i < size; i++)
   {

      switch (promoteTo)
      {
      case EbtFloat:
         switch (node->getType().getBasicType())
         {
         case EbtInt:
            leftUnionArray[i].setFConst(static_cast<float>(rightUnionArray[i].getIConst()));
            break;
         case EbtBool:
            leftUnionArray[i].setFConst(static_cast<float>(rightUnionArray[i].getBConst()));
            break;
         case EbtFloat:
            leftUnionArray[i] = rightUnionArray[i];
            break;
         default: 
            infoSink.info.message(EPrefixInternalError, "Cannot promote", node->getLine());
            return 0;
         }                
         break;
      case EbtInt:
         switch (node->getType().getBasicType())
         {
         case EbtInt:
            leftUnionArray[i] = rightUnionArray[i];
            break;
         case EbtBool:
            leftUnionArray[i].setIConst(static_cast<int>(rightUnionArray[i].getBConst()));
            break;
         case EbtFloat:
            leftUnionArray[i].setIConst(static_cast<int>(rightUnionArray[i].getFConst()));
            break;
         default: 
            infoSink.info.message(EPrefixInternalError, "Cannot promote", node->getLine());
            return 0;
         }                
         break;
      case EbtBool:
         switch (node->getType().getBasicType())
         {
         case EbtInt:
            leftUnionArray[i].setBConst(rightUnionArray[i].getIConst() != 0);
            break;
         case EbtBool:
            leftUnionArray[i] = rightUnionArray[i];
            break;
         case EbtFloat:
            leftUnionArray[i].setBConst(rightUnionArray[i].getFConst() != 0.0f);
            break;
         default: 
            infoSink.info.message(EPrefixInternalError, "Cannot promote", node->getLine());
            return 0;
         }                

         break;
      default:
         infoSink.info.message(EPrefixInternalError, "Incorrect data type found", node->getLine());
         return 0;
      }

   }

   const TType& t = node->getType();

   return addConstantUnion(leftUnionArray, TType(promoteTo, t.getQualifier(), t.getNominalSize(), t.isMatrix(), t.isArray()), node->getLine());
}

void TIntermAggregate::addToPragmaTable(const TPragmaTable& pTable)
{
   assert(!pragmaTable);
   pragmaTable = new TPragmaTable();
   *pragmaTable = pTable;
}


