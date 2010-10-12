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
// Definition of the in-memory high-level intermediate representation
// of shaders.  This is a tree that parser creates.
//
// Nodes in the tree are defined as a hierarchy of classes derived from 
// TIntermNode. Each is a node in a tree.  There is no preset branching factor;
// each node can have it's own type of list of children.
//

#ifndef __INTERMEDIATE_H
#define __INTERMEDIATE_H

#include "../Include/Common.h"
#include "../Include/Types.h"
#include "../Include/ConstantUnion.h"

//
// Operators used by the high-level (parse tree) representation.
//
enum TOperator
{
   EOpNull,            // if in a node, should only mean a node is still being built
   EOpSequence,        // denotes a list of statements, or parameters, etc.
   EOpFunctionCall,    
   EOpFunction,        // For function definition
   EOpParameters,      // an aggregate listing the parameters to a function

   //
   // Unary operators
   //

   EOpNegative,
   EOpLogicalNot,
   EOpVectorLogicalNot,
   EOpBitwiseNot,

   EOpPostIncrement,
   EOpPostDecrement,
   EOpPreIncrement,
   EOpPreDecrement,

   EOpConvIntToBool,
   EOpConvFloatToBool,
   EOpConvBoolToFloat,
   EOpConvIntToFloat,
   EOpConvFloatToInt,
   EOpConvBoolToInt,

   //
   // binary operations
   //

   EOpAdd,
   EOpSub,
   EOpMul,
   EOpDiv,
   EOpMod,
   EOpRightShift,
   EOpLeftShift,
   EOpAnd,
   EOpInclusiveOr,
   EOpExclusiveOr,
   EOpEqual,
   EOpNotEqual,
   EOpVectorEqual,
   EOpVectorNotEqual,
   EOpLessThan,
   EOpGreaterThan,
   EOpLessThanEqual,
   EOpGreaterThanEqual,
   EOpComma,

   EOpVectorTimesScalar,
   EOpVectorTimesMatrix,
   EOpMatrixTimesVector,
   EOpMatrixTimesScalar,

   EOpLogicalOr,
   EOpLogicalXor,
   EOpLogicalAnd,

   EOpIndexDirect,
   EOpIndexIndirect,
   EOpIndexDirectStruct,

   EOpVectorSwizzle,
   EOpMatrixSwizzle,

   //
   // Built-in functions potentially mapped to operators
   //

   EOpRadians,
   EOpDegrees,
   EOpSin,
   EOpCos,
   EOpTan,
   EOpAsin,
   EOpAcos,
   EOpAtan,
   EOpAtan2,
   EOpSinCos,

   EOpPow,
   EOpExp,
   EOpLog,
   EOpExp2,
   EOpLog2,
   EOpLog10,
   EOpSqrt,
   EOpInverseSqrt,

   EOpAbs,
   EOpSign,
   EOpFloor,
   EOpCeil,
   EOpFract,
   EOpMin,
   EOpMax,
   EOpClamp,
   EOpMix,
   EOpStep,
   EOpSmoothStep,

   EOpLength,
   EOpDistance,
   EOpDot,
   EOpCross,
   EOpNormalize,
   EOpFaceForward,
   EOpReflect,
   EOpRefract,

   EOpDPdx,            // Fragment only
   EOpDPdy,            // Fragment only
   EOpFwidth,          // Fragment only
   EOpFclip,		   // Framgent only

   //Added for HLSL support
   EOpTex1D,
   EOpTex1DProj,
   EOpTex1DLod,
   EOpTex1DBias,
   EOpTex1DGrad,
   EOpTex2D,
   EOpTex2DProj,
   EOpTex2DLod,
   EOpTex2DBias,
   EOpTex2DGrad,
   EOpTex3D,
   EOpTex3DProj,
   EOpTex3DLod,
   EOpTex3DBias,
   EOpTex3DGrad,
   EOpTexCube,
   EOpTexCubeProj,
   EOpTexCubeLod,
   EOpTexCubeBias,
   EOpTexCubeGrad,
   EOpTexRect,
   EOpTexRectProj,
	
   EOpTranspose,
   EOpDeterminant,
   EOpSaturate,
   EOpModf,
   EOpLdexp,

   EOpMatrixTimesMatrix,

   EOpAny,
   EOpAll,

   EOpItof,         // pack/unpack only
   EOpFtoi,         // pack/unpack only    
   EOpSkipPixels,   // pack/unpack only
   EOpReadInput,    // unpack only
   EOpWritePixel,   // unpack only
   EOpBitmapLsb,    // unpack only
   EOpBitmapMsb,    // unpack only
   EOpWriteOutput,  // pack only
   EOpReadPixel,    // pack only

   //
   // Branch
   //

   EOpKill,            // Fragment only
   EOpReturn,
   EOpBreak,
   EOpContinue,

   //
   // Constructors
   //

   EOpConstructInt,
   EOpConstructBool,
   EOpConstructFloat,
   EOpConstructVec2,
   EOpConstructVec3,
   EOpConstructVec4,
   EOpConstructBVec2,
   EOpConstructBVec3,
   EOpConstructBVec4,
   EOpConstructIVec2,
   EOpConstructIVec3,
   EOpConstructIVec4,
   EOpConstructMat2,
   EOpConstructMat3,
   EOpConstructMat4,
   EOpConstructStruct,

   //HLSL matrix/matrix constructors
   EOpConstructMat2FromMat,
   EOpConstructMat3FromMat,

   //
   // moves
   //

   EOpAssign,
   EOpInitialize,  //Special Assign only happening at declaration
   EOpAddAssign,
   EOpSubAssign,
   EOpMulAssign,
   EOpVectorTimesMatrixAssign,
   EOpVectorTimesScalarAssign,
   EOpMatrixTimesScalarAssign,
   EOpMatrixTimesMatrixAssign,
   EOpDivAssign,
   EOpModAssign,
   EOpAndAssign,
   EOpInclusiveOrAssign,
   EOpExclusiveOrAssign,
   EOpLeftShiftAssign,
   EOpRightShiftAssign,

   //
   // Array operators
   //

   EOpArrayLength,

   //
   // Special HLSL functions
   //
   EOpD3DCOLORtoUBYTE4,
	
	//
	// Ternary selection on vector
	//
	EOpVecTernarySel,
};

class TIntermTraverser;
class TIntermAggregate;
class TIntermBinary;
class TIntermConstantUnion;
class TIntermSelection;
class TIntermTyped;
class TIntermSymbol;
class TInfoSink;

//
// Base class for the tree nodes
//
class TIntermNode
{
public:
   POOL_ALLOCATOR_NEW_DELETE(GlobalPoolAllocator)

   TIntermNode() : line(0)
   {
   }

   TSourceLoc getLine() const { return line; }
   void setLine(TSourceLoc l) { line = l; }

   virtual void traverse(TIntermTraverser*) = 0;

   virtual TIntermTyped*     getAsTyped() { return 0; }
   virtual TIntermConstantUnion*     getAsConstantUnion() { return 0; }
   virtual TIntermAggregate* getAsAggregate() { return 0; }
   virtual TIntermBinary*    getAsBinaryNode() { return 0; }
   virtual TIntermSelection* getAsSelectionNode() { return 0; }
   virtual TIntermSymbol*    getAsSymbolNode() { return 0; }
   virtual ~TIntermNode() { }

protected:
   TSourceLoc line;
};

//
// This is just to help yacc.
//
struct TIntermNodePair
{
   TIntermNode* node1;
   TIntermNode* node2;
};

class TIntermSymbol;
class TIntermBinary;

//
// Intermediate class for nodes that have a type.
//
class TIntermTyped : public TIntermNode
{
public:
   TIntermTyped(const TType& t) : type(t)
   {
   }

   virtual TIntermTyped* getAsTyped() { return this; }

   void setType(const TType& t) { type = t; }
   TType getType() const { return type; }
   TType* getTypePointer() { return &type; }

   TBasicType getBasicType() const { return type.getBasicType(); }
   TQualifier getQualifier() const { return type.getQualifier(); }
   int getNominalSize() const { return type.getNominalSize(); }
   int getSize() const { return type.getInstanceSize(); }
   bool isMatrix() const { return type.isMatrix(); }
   bool isArray()  const { return type.isArray(); }
   bool isVector() const { return type.isVector(); }
   const char* getBasicString() const { return type.getBasicString(); }
   const char* getQualifierString() const { return type.getQualifierString(); }
   TString getCompleteString() const { return type.getCompleteString(); }

protected:
   TType type;
};

//
// Handle for, do-while, and while loops.
//
class TIntermLoop : public TIntermNode
{
public:
   TIntermLoop(TIntermNode* aBody, TIntermTyped* aTest, TIntermTyped* aTerminal, bool testFirst) : 
   body(aBody),
   test(aTest),
   terminal(aTerminal),
   first(testFirst)
   {
   }
   virtual void traverse(TIntermTraverser*);
   TIntermNode*  getBody() { return body; }
   TIntermTyped* getTest() { return test; }
   TIntermTyped* getTerminal() { return terminal; }
   bool testFirst() { return first; }
protected:
   TIntermNode* body;       // code to loop over
   TIntermTyped* test;      // exit condition associated with loop, could be 0 for 'for' loops
   TIntermTyped* terminal;  // exists for for-loops
   bool first;              // true for while and for, not for do-while
};

//
// Handle break, continue, return, and kill.
//
class TIntermBranch : public TIntermNode
{
public:
   TIntermBranch(TOperator op, TIntermTyped* e) :
   flowOp(op),
   expression(e)
   {
   }
   virtual void traverse(TIntermTraverser*);

   TOperator getFlowOp() { return flowOp; }
   TIntermTyped* getExpression() { return expression; }
protected:
   TOperator flowOp;
   TIntermTyped* expression;  // non-zero except for "return exp;" statements
};

//
// Nodes that correspond to symbols or constants in the source code.
//
class TIntermSymbol : public TIntermTyped
{
public:
   // if symbol is initialized as symbol(sym), the memory comes from the poolallocator of sym. If sym comes from
   // per process globalpoolallocator, then it causes increased memory usage per compile
   // it is essential to use "symbol = sym" to assign to symbol
   TIntermSymbol(int i, const TString& sym, const TType& t) : 
   TIntermTyped(t), id(i), info(0)
   {
      symbol = sym;
   } 
   TIntermSymbol(int i, const TString& sym, const TTypeInfo *inf, const TType& t) : 
   TIntermTyped(t), id(i), info(inf)
   {
      symbol = sym;
   } 

   int getId() const { return id; }
   const TString& getSymbol() const { return symbol; }

   virtual const TTypeInfo* getInfo() const
   {
      return info;
   }
   virtual void traverse(TIntermTraverser*);
   virtual TIntermSymbol* getAsSymbolNode()
   {
      return this;
   }
protected:
   int id;
   TString symbol;
   const TTypeInfo *info;
};

class TIntermConstantUnion : public TIntermTyped
{
public:
   TIntermConstantUnion(constUnion *unionPointer, const TType& t) : TIntermTyped(t), unionArrayPointer(unionPointer)
   {
   }
   constUnion* getUnionArrayPointer() const
   {
      return unionArrayPointer;
   }
   void setUnionArrayPointer(constUnion *c)
   {
      unionArrayPointer = c;
   }
   virtual TIntermConstantUnion* getAsConstantUnion()
   {
      return this;
   }
   virtual void traverse(TIntermTraverser* );
   virtual TIntermTyped* fold(TOperator, TIntermTyped*, TInfoSink&);
protected:
   constUnion *unionArrayPointer;
};

//
// Intermediate class for node types that hold operators.
//
class TIntermOperator : public TIntermTyped
{
public:
   TOperator getOp()
   {
      return op;
   }
   bool modifiesState() const;
   bool isConstructor() const;
   virtual bool promote(TInfoSink&)
   {
      return true;
   }
protected:
   TIntermOperator(TOperator o) : TIntermTyped(TType(EbtFloat)), op(o)
   {
   }
   TIntermOperator(TOperator o, TType& t) : TIntermTyped(t), op(o)
   {
   }   
   TOperator op;
};

//
// Nodes for all the basic binary math operators.
//
class TIntermBinary : public TIntermOperator
{
public:
   TIntermBinary(TOperator o) : TIntermOperator(o)
   {
   }
   virtual void traverse(TIntermTraverser*);

   void setLeft(TIntermTyped* n) { left = n; }
   void setRight(TIntermTyped* n) { right = n; }
   TIntermTyped* getLeft() const { return left; }
   TIntermTyped* getRight() const { return right; }

   virtual TIntermBinary* getAsBinaryNode()
   {
      return this;
   }
   virtual bool promote(TInfoSink&);
protected:
   TIntermTyped* left;
   TIntermTyped* right;
};

//
// Nodes for unary math operators.
//
class TIntermUnary : public TIntermOperator
{
public:
   TIntermUnary(TOperator o, TType& t) : TIntermOperator(o, t), operand(0)
   {
   }
   TIntermUnary(TOperator o) : TIntermOperator(o), operand(0)
   {
   }
   virtual void traverse(TIntermTraverser*);

   void setOperand(TIntermTyped* o) { operand = o; }
   TIntermTyped* getOperand() { return operand; }

   virtual bool promote(TInfoSink&);
protected:
   TIntermTyped* operand;
};

typedef TVector<TIntermNode*> TIntermSequence;

//
// Nodes that operate on an arbitrary sized set of children.
//
class TIntermAggregate : public TIntermOperator
{
public:
   TIntermAggregate() : TIntermOperator(EOpNull)
   {
   }
   TIntermAggregate(TOperator o) : TIntermOperator(o)
   {
   }
   ~TIntermAggregate() { }

   virtual TIntermAggregate* getAsAggregate()
   {
      return this;
   }

   void setOperator(TOperator o) { op = o; }
   TIntermSequence& getSequence() { return sequence; }
   void setName(const TString& n) { name = n; }
   void setPlainName(const TString& n) { plainName = n; }
   void setSemantic(const TString& s) { semantic = s; }
   const TString& getName() const { return name; }
   const TString& getPlainName() const { return plainName; }
   const TString& getSemantic() const { return semantic; }

   virtual void traverse(TIntermTraverser*);

protected:
   TIntermAggregate(const TIntermAggregate&); // disallow copy constructor
   TIntermAggregate& operator=(const TIntermAggregate&); // disallow assignment operator
   TIntermSequence sequence;
   TString name;
   TString plainName;
   TString semantic;
};

//
// For if tests.  Simplified since there is no switch statement.
//
class TIntermSelection : public TIntermTyped
{
public:
   TIntermSelection(TIntermTyped* cond, TIntermNode* trueB, TIntermNode* falseB) :
   TIntermTyped(TType(EbtVoid)), condition(cond), trueBlock(trueB), falseBlock(falseB)
   {
   }
   TIntermSelection(TIntermTyped* cond, TIntermNode* trueB, TIntermNode* falseB, const TType& type) :
   TIntermTyped(type), condition(cond), trueBlock(trueB), falseBlock(falseB)
   {
   }
   virtual void traverse(TIntermTraverser*);

   TIntermNode* getCondition() const { return condition; }
   TIntermNode* getTrueBlock() const { return trueBlock; }
   TIntermNode* getFalseBlock() const { return falseBlock; }
   TIntermSelection* getAsSelectionNode() { return this; }

	bool promoteTernary(TInfoSink&);
protected:
   TIntermTyped* condition;
   TIntermNode* trueBlock;
   TIntermNode* falseBlock;
};

//
// For traversing the tree.  User should derive from this, 
// put their traversal specific data in it, and then pass
// it to a Traverse method.
//
// When using this, just fill in the methods for nodes you want visited.
// Return false from a pre-visit to skip visiting that node's subtree.
//
class TIntermTraverser
{
public:
   POOL_ALLOCATOR_NEW_DELETE(GlobalPoolAllocator)

   TIntermTraverser() : 
      visitSymbol(0), 
      visitConstantUnion(0),
      visitBinary(0),
      visitUnary(0),
      visitSelection(0),
      visitAggregate(0),
      visitLoop(0),
      visitBranch(0),
      depth(0),
      preVisit(true),
      postVisit(false)
   {
   }

   void (*visitSymbol)(TIntermSymbol*, TIntermTraverser*);
   void (*visitConstantUnion)(TIntermConstantUnion*, TIntermTraverser*);
   bool (*visitBinary)(bool preVisit, TIntermBinary*, TIntermTraverser*);
   bool (*visitUnary)(bool preVisit, TIntermUnary*, TIntermTraverser*);
   bool (*visitSelection)(bool preVisit, TIntermSelection*, TIntermTraverser*);
   bool (*visitAggregate)(bool preVisit, TIntermAggregate*, TIntermTraverser*);
   bool (*visitLoop)(bool preVisit, TIntermLoop*, TIntermTraverser*);
   bool (*visitBranch)(bool preVisit, TIntermBranch*,  TIntermTraverser*);

   int  depth;
   bool preVisit;
   bool postVisit;
};

#endif // __INTERMEDIATE_H

