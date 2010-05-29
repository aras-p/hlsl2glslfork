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
// Implementation of TGlslOutputTraverser
//=================================================================================================================================

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================
#include "glslOutput.h"

//=================================================================================================================================
//
//          Helper Functions Block
//
//=================================================================================================================================

//=========================================================================================================
/// Get the number of elements in a GLSL symbol type
/// \param t
///    GLSL symbol type
/// \return
///    The number of elements in the GLSL symbol type, 0 if unknown type
//=========================================================================================================
int getElements( EGlslSymbolType t )
{
   switch (t)
   {
   case EgstBool:
   case EgstInt:
   case EgstFloat:
   case EgstStruct:
      return 1;
   case EgstBool2:
   case EgstInt2:
   case EgstFloat2:
      return 2;
   case EgstBool3:
   case EgstInt3:
   case EgstFloat3:
      return 3;
   case EgstBool4:
   case EgstInt4:
   case EgstFloat4:
   case EgstFloat2x2:
      return 4;
   case EgstFloat3x3:
      return 9;
   case EgstFloat4x4:
      return 16;
   }

   return 0;
}

//=========================================================================================================
/// Helper function to output a constructor of all constants
/// \param out
///    Output string buffer
/// \param t
///    GLSL symbol type to output
/// \param c
///    Constant union holding values to pass to constructor
/// \param str
///    If a structure, contains the GLSL structure pointer (default is NULL)
/// \return
///    The number of elements in the GLSL symbol type, 0 if unknown type
//=========================================================================================================
void writeConstantConstructor( std::stringstream& out, EGlslSymbolType t, constUnion *c, GlslStruct *str = 0 )
{
   int elementCount = getElements(t);
   bool construct = elementCount > 1 || str != 0;

   if (construct)
   {
      writeType( out, t, str);
      out << "( ";
   }

   if ( str == 0)
   {
      // simple type
      for (int ii = 0; ii<elementCount; ii++, c++)
      {
         if (construct && ii > 0)
         {
            out << ", ";
         }

         switch (c->getType())
         {
         case EbtBool:
            out << (c->getBConst() ? "true" : "false");
            break;
         case EbtInt:
            out << c->getIConst();
            break;
         case EbtFloat:
            out << c->getFConst();
            break;
         default:
            assert(0);
         }
      }
   }
   else
   {
      // compound type
      for (int ii = 0; ii<str->memberCount(); ii++)
      {
         const GlslStruct::member &m = str->getMember(ii);

         if (construct && ii > 0)
            out << ", ";

         writeConstantConstructor( out, m.type, c);
      }
   }

   if (construct)
   {
      out << ")";
   }
}

//=========================================================================================================
/// Helper function to output a comparison operator
/// \param compareOp
///    Scalar comparison op (e.g. "==", "!=",..)
/// \param compareCall  
///    Vector comparison call (e.g. "equal", "notEqual",..)
/// \param node 
///    Node to output
/// \param goit 
///    GLSL traverser
//=========================================================================================================
void writeComparison( const TString &compareOp, const TString &compareCall, TIntermBinary *node, TGlslOutputTraverser* goit ) 
{
   GlslFunction *current = goit->current;    
   std::stringstream& out = current->getActiveOutput();
   bool bUseCompareCall = false;

   // Determine whether we need the vector or scalar comparison function
   if ( ( node->getLeft() && node->getLeft()->getNominalSize() > 1 ) ||
        ( node->getRight() && node->getRight()->getNominalSize() > 1 ) )
   {
      bUseCompareCall = true;
   }

   current->beginStatement ();

   // Output vector comparison
   if ( bUseCompareCall )
   {
      out << compareCall << "( ";

      if (node->getLeft())
      {
         // If it is a float, need to smear to the size of the right hand side
         if ( node->getLeft()->getNominalSize() == 1 )
         {
            out << "vec" <<  node->getRight()->getNominalSize() << "( ";

            node->getLeft()->traverse(goit);

            out << " )";                
         }
         else
         {
            node->getLeft()->traverse(goit);
         }         
      }
      out << ", ";

      if (node->getRight())
      {
         // If it is a float, need to smear to the size of the left hand side
         if ( node->getRight()->getNominalSize() == 1 )
         {
            out << "vec" <<  node->getLeft()->getNominalSize() << "( ";

            node->getRight()->traverse(goit);

            out << " )";             
         }
         else
         {
            node->getRight()->traverse(goit);
         }         
      }
      out << ")";
   }
   // Output scalar comparison
   else
   {
      out << "(";

      if (node->getLeft())
         node->getLeft()->traverse(goit);
      out << " " << compareOp << " ";
      if (node->getRight())
         node->getRight()->traverse(goit);

      out << ")";
   }
}

//=========================================================================================================
/// Helper function to output a function call
/// \param name
///    Name of function to call
/// \param node  
///    Aggregate node of function parameters
/// \param goit 
///    GLSL traverser
/// \param bGenMatrix
///    If true, this function will generate a call to the xlat_lib matrix version of the function if
///    the node is a matrix.
//=========================================================================================================
void writeFuncCall( const TString &name, TIntermAggregate *node, TGlslOutputTraverser* goit, bool bGenMatrix = false )
{
   TIntermSequence::iterator sit;
   TIntermSequence &sequence = node->getSequence(); 
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();

   current->beginStatement();
   
   if ( bGenMatrix )
   {
      if ( node->isMatrix () )
      {
         out << "xlat_lib_";
         current->addLibFunction ( node->getOp() );
      }
   }      

   out << name << "( ";

	for (sit = sequence.begin(); sit != sequence.end(); ++sit)
	{
		if (sit !=sequence.begin())
			out << ", ";
		(*sit)->traverse(goit);
	}
	
   out << ")";
}

//=========================================================================================================
/// Helper function to output a imaru built-in function call.  If it is a matrix, generates the appropriate 
/// xlat_lib function
/// \param name
///    Name of function to call
/// \param node  
///    Unary node to traverse
/// \param opStr
///    Operator string name (e.g. "radians")
/// \param funcStyle
///    Reference to function style, will be set to true
/// \param pre
///    Reference to prefix, will be set to true
/// \param goit 
///    GLSL traverser
//=========================================================================================================
void setupUnaryBuiltInFuncCall( const TString &name, TIntermUnary *node, TString &opStr, bool &funcStyle, bool &prefix,
                                TGlslOutputTraverser* goit )
{
   GlslFunction *current = goit->current;   

   funcStyle = true;
   prefix = true;
   if ( node->isMatrix() )
   {
      current->addLibFunction( node->getOp() );
      opStr = "xlat_lib_" + name;
   }
   else
   {
      opStr = name;
   }   
}

//=========================================================================================================
/// Helper function to output a texture call
/// \param name
///    Name of function to call
/// \param node  
///    Aggregate node of function parameters
/// \param goit 
///    GLSL traverser
//=========================================================================================================
void writeTex( const TString &name, TIntermAggregate *node, TGlslOutputTraverser* goit )
{
   writeFuncCall( name, node, goit);
}



//=================================================================================================================================
//
//          Constructor(s) / Destructor(s) Block 
//
//=================================================================================================================================

//=========================================================================================================
/// Constructor
//=========================================================================================================
TGlslOutputTraverser::TGlslOutputTraverser(TInfoSink& i, std::vector<GlslFunction*> &funcList, std::vector<GlslStruct*> &sList ) :
      infoSink(i),
      generatingCode(true),
      functionList(funcList),
      structList(sList)
{
   visitSymbol = traverseSymbol;
   visitConstantUnion = traverseConstantUnion;
   visitBinary = traverseBinary;
   visitUnary = traverseUnary;
   visitSelection = traverseSelection;
   visitAggregate = traverseAggregate;
   visitLoop = traverseLoop;
   visitBranch = traverseBranch;
   global = new GlslFunction( "__global__", "__global__", EgstVoid, "");
   functionList.push_back(global);
   current = global;
}

//=================================================================================================================================
//
//          Private methods block
//
//=================================================================================================================================

//=========================================================================================================
/// Traverse a symbol node, add the name to the output buffer
/// \param node
///   Symbol node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
void TGlslOutputTraverser::traverseSymbol(TIntermSymbol *node, TIntermTraverser *it)
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();

   current->beginStatement();

   if ( ! current->hasSymbol( node->getId()))
   {

      //check to see if it is a global we can share
      if ( goit->global->hasSymbol( node->getId()))
      {
         current->addSymbol( &goit->global->getSymbol( node->getId()));
      }
      else
      {
         int array = node->getTypePointer()->isArray() ? node->getTypePointer()->getArraySize() : 0;
         const char* semantic = "";
         if (node->getInfo())
            semantic = node->getInfo()->getSemantic().c_str();
         GlslSymbol * sym = new GlslSymbol( node->getSymbol().c_str(), semantic, node->getId(),
                                            translateType(node->getTypePointer()), translateQualifier(node->getQualifier()), array);
         current->addSymbol(sym);
         if (sym->getType() == EgstStruct)
         {
            GlslStruct *s = goit->createStructFromType( node->getTypePointer());
            sym->setStruct(s);
         }
      }
   }


   out << current->getSymbol( node->getId()).getName();
}

//=========================================================================================================
/// Traverse a symbol node which is a function parameter, add the name to the output buffer
/// \param node
///   Parameter symbol node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
void TGlslOutputTraverser::traverseParameterSymbol(TIntermSymbol *node, TIntermTraverser *it)
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;

   int array = node->getTypePointer()->isArray() ? node->getTypePointer()->getArraySize() : 0;
   const char* semantic = "";
   if (node->getInfo())
      semantic = node->getInfo()->getSemantic().c_str();
   GlslSymbol * sym = new GlslSymbol( node->getSymbol().c_str(), semantic, node->getId(),
                                      translateType(node->getTypePointer()), translateQualifier(node->getQualifier()), array);
   current->addParameter(sym);

   if (sym->getType() == EgstStruct)
   {
      GlslStruct *s = goit->createStructFromType( node->getTypePointer());
      sym->setStruct(s);
   }
}


//=========================================================================================================
/// Traverse a node in the tree representing a constant
/// \param node
///   Parameter constant node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
void TGlslOutputTraverser::traverseConstantUnion( TIntermConstantUnion *node, TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   EGlslSymbolType type = translateType( node->getTypePointer());
   GlslStruct *str = 0;

   constUnion *c = node->getUnionArrayPointer();

   current->beginStatement();

   if (type == EgstStruct)
   {
      str = goit->createStructFromType( node->getTypePointer());
   }

   writeConstantConstructor( out, type, c, str);
}

//=========================================================================================================
/// Traverse a node in the tree representing a constant intended for immediate use, such as a direct 
/// reference.
/// \param node
///   Parameter constant node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
void TGlslOutputTraverser::traverseImmediateConstant( TIntermConstantUnion *node, TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);

   constUnion *c = node->getUnionArrayPointer();

   // These are all expected to be length 1
   assert( node->getSize() == 1);

   // Autotype the result
   switch (c[0].getType())
   {
   case EbtBool:
      goit->indexList.push_back( c[0].getBConst() ? 1 : 0);
      break;
   case EbtInt:
      goit->indexList.push_back( c[0].getIConst());
      break;
   case EbtFloat:
      goit->indexList.push_back( int(c[0].getFConst()));
      break;
   default:
      assert(0); 
      goit->indexList.push_back(0);
   }
}

//=========================================================================================================
/// Traverse a binary node generating the output GLSL code
/// \param preVisit
///   Unused parameter, used by base class version
/// \param node
///   Binary node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
bool TGlslOutputTraverser::traverseBinary( bool preVisit, TIntermBinary *node, TIntermTraverser *it )
{
   TString op = "??";
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   bool infix = true;
   bool assign = false;
   bool needsParens = true;

   switch (node->getOp())
   {
   case EOpAssign:                   op = "=";   infix = true; needsParens = false; break;

   case EOpInitialize:
      if (goit->parseInitializer(node))
         return false;

      // Check to see if we have an array initializer list
      if ( node->getLeft() && node->getRight() )
      {
         TIntermSymbol *symNode = node->getLeft()->getAsSymbolNode();
         TIntermAggregate *aggNode = node->getRight()->getAsAggregate();

         // Left hand size is a symbol, right hand side is an aggregate.  This
         // is an initializer sequence for an array.
         if ( symNode && symNode->isArray() && aggNode )
         {
            // Get the number of elements in the array.  TParseContext::constructArray should
            // have already made sure that the correct number of elements are present in
            // the initializer sequence
            int nElements = symNode->getTypePointer()->getArraySize();

            // Loop over all elements
            for ( int i = 0; i < nElements; i++ )
            {
               // Initialize each element
               node->getLeft()->traverse(goit);
               out << "[" << i << "] = ";
               aggNode->getSequence()[i]->traverse(goit);                      

               current->endStatement();
               if ( i != nElements - 1 )
                  current->beginStatement();
            }
            return false;
         }
      }

      // Fallthrough and process as a normal assignment
      current->beginStatement();
      op = "=";
      infix = true;
      needsParens = false;
      break;

   case EOpAddAssign:                op = "+=";  infix = true; needsParens = false; break;
   case EOpSubAssign:                op = "-=";  infix = true; needsParens = false; break;
   case EOpMulAssign:                op = "*=";  infix = true; needsParens = false; break;
   case EOpVectorTimesMatrixAssign:  op = "*=";  infix = true; needsParens = false; break;
   case EOpVectorTimesScalarAssign:  op = "*=";  infix = true; needsParens = false; break;
   case EOpMatrixTimesScalarAssign:  op = "*=";  infix = true; needsParens = false; break;
   case EOpMatrixTimesMatrixAssign:  op = "matrixCompMult";  infix = false; assign = true; break;
   case EOpDivAssign:                op = "/=";  infix = true; needsParens = false; break;
   case EOpModAssign:                op = "%=";  infix = true; needsParens = false; break;
   case EOpAndAssign:                op = "&=";  infix = true; needsParens = false; break;
   case EOpInclusiveOrAssign:        op = "|=";  infix = true; needsParens = false; break;
   case EOpExclusiveOrAssign:        op = "^=";  infix = true; needsParens = false; break;
   case EOpLeftShiftAssign:          op = "<<="; infix = true; needsParens = false; break;
   case EOpRightShiftAssign:         op = "??="; infix = true; needsParens = false; break;

   case EOpIndexDirect:
      {
         TIntermTyped *left = node->getLeft();
         TIntermTyped *right = node->getRight();
         assert( left && right);

         current->beginStatement();

         left->traverse(goit);

         // Special code for handling a vector component select (this improves readability)
         if (left->isVector() && !left->isArray() && right->getAsConstantUnion())
         {
            char swiz[] = "xyzw";
            goit->visitConstantUnion = TGlslOutputTraverser::traverseImmediateConstant;
            goit->generatingCode = false;
            right->traverse(goit);
            assert( goit->indexList.size() == 1);
            assert( goit->indexList[0] < 4);
            out << "." << swiz[goit->indexList[0]];
            out << " ";
            goit->indexList.clear();
            goit->visitConstantUnion = TGlslOutputTraverser::traverseConstantUnion;
            goit->generatingCode = true;
         }
         else
         {
            out << "[ ";
            right->traverse(goit);
            out << " ]";
         }
         return false;
      }
   case EOpIndexIndirect:
      current->beginStatement();

      if (node->getLeft())
         node->getLeft()->traverse(goit);
      out << "[ ";
      if (node->getRight())
         node->getRight()->traverse(goit);
      out << " ]";
      return false;

   case EOpIndexDirectStruct:
      {
         current->beginStatement();
         GlslStruct *s = goit->createStructFromType(node->getLeft()->getTypePointer());
         if (node->getLeft())
            node->getLeft()->traverse(goit);

         // The right child is always an offset into the struct, switch to get an
         // immediate constant, and put it back afterwords
         goit->visitConstantUnion = TGlslOutputTraverser::traverseImmediateConstant;
         goit->generatingCode = false;

         if (node->getRight())
         {
            node->getRight()->traverse(goit);
            assert( goit->indexList.size() == 1);
            assert( goit->indexList[0] < s->memberCount());
            out << "." << s->getMember(goit->indexList[0]).name;

         }

         goit->indexList.clear();
         goit->visitConstantUnion = TGlslOutputTraverser::traverseConstantUnion;
         goit->generatingCode = true;
      }
      return false;

   case EOpVectorSwizzle:
      current->beginStatement();
      if (node->getLeft())
         node->getLeft()->traverse(goit);
      goit->visitConstantUnion = TGlslOutputTraverser::traverseImmediateConstant;
      goit->generatingCode = false;
      if (node->getRight())
      {
         node->getRight()->traverse(goit);
         assert( goit->indexList.size() <= 4);
         out << ".";
         const char fields[] = "xyzw";
         for (int ii = 0; ii < (int)goit->indexList.size(); ii++)
         {
            int val = goit->indexList[ii];
            assert( val >= 0);
            assert( val < 4);
            out << fields[val];
         }
      }
      out << " ";
      goit->indexList.clear();
      goit->visitConstantUnion = TGlslOutputTraverser::traverseConstantUnion;
      goit->generatingCode = true;
      return false;

   case EOpMatrixSwizzle:
      current->beginStatement();
      // This presently only works for swizzles as rhs operators
      goit->visitConstantUnion = TGlslOutputTraverser::traverseImmediateConstant;
      goit->generatingCode = false;
      if (node->getRight())
      {
         node->getRight()->traverse(goit);
         assert( goit->indexList.size() <= 4);
         assert( goit->indexList.size() > 0);
         int collumn[4], row[4];
         for (int ii = 0; ii < (int)goit->indexList.size(); ii++)
         {
            int val = goit->indexList[ii];
            collumn[ii] = val/4;
            row[ii] = val%4;
         }
         bool sameCollumn = true;
         for (int ii = 1; ii < (int)goit->indexList.size(); ii++)
         {
            sameCollumn &= collumn[ii] == collumn[ii-1];
         }
         if (sameCollumn)
         {
            //select column, then swizzle row
            if (node->getLeft())
               node->getLeft()->traverse(goit);
            out << "[" << collumn[0] << "].";
            const char fields[] = "xyzw";
            for (int ii = 0; ii < (int)goit->indexList.size(); ii++)
            {
               out << fields[row[ii]];
            }
         }
         else
         {
            // Insert constructor, and dereference individually

            // Might need to account for different types here 
            assert( (int)goit->indexList.size() != 1); //should have hit same collumn case
            out << "vec" << (int)goit->indexList.size() << "( ";
            const char fields[] = "xyzw";
            if (node->getLeft())
               node->getLeft()->traverse(goit);
            out << "[" << collumn[0] << "].";
            out << fields[row[0]];
            for (int ii = 1; ii < (int)goit->indexList.size(); ii++)
            {
               out << ", ";
               if (node->getLeft())
                  node->getLeft()->traverse(goit);
               out << "[" << collumn[ii] << "].";
               out << fields[row[ii]];
            }
            out << ")";
         }
      }
      goit->indexList.clear();
      goit->visitConstantUnion = TGlslOutputTraverser::traverseConstantUnion;
      goit->generatingCode = true;
      return false;

   case EOpAdd:    op = "+"; infix = true; break;
   case EOpSub:    op = "-"; infix = true; break;
   case EOpMul:    op = "*"; infix = true; break;
   case EOpDiv:    op = "/"; infix = true; break;
   case EOpMod:    op = "%"; infix = true; break;
   case EOpRightShift:  op = "<<"; infix = true; break;
   case EOpLeftShift:   op = ">>"; infix = true; break;
   case EOpAnd:         op = "&"; infix = true; break;
   case EOpInclusiveOr: op = "|"; infix = true; break;
   case EOpExclusiveOr: op = "^"; infix = true; break;
   case EOpEqual:       
      writeComparison ( "==", "equal", node, goit );
      return false;        

   case EOpNotEqual:        
      writeComparison ( "!=", "notEqual", node, goit );
      return false;               

   case EOpLessThan: 
      writeComparison ( "<", "lessThan", node, goit );
      return false;               

   case EOpGreaterThan:
      writeComparison ( ">", "greaterThan", node, goit );
      return false;               

   case EOpLessThanEqual:    
      writeComparison ( "<=", "lessThanEqual", node, goit );
      return false;               


   case EOpGreaterThanEqual: 
      writeComparison ( ">=", "greaterThanEqual", node, goit );
      return false;               


   case EOpVectorTimesScalar: op = "*"; infix = true; break;
   case EOpVectorTimesMatrix: op = "*"; infix = true; break;
   case EOpMatrixTimesVector: op = "*"; infix = true; break;
   case EOpMatrixTimesScalar: op = "*"; infix = true; break;
   case EOpMatrixTimesMatrix: op = "matrixCompMult"; infix = false; assign = false; break;

   case EOpLogicalOr:  op = "||"; infix = true; break;
   case EOpLogicalXor: op = "^^"; infix = true; break;
   case EOpLogicalAnd: op = "&&"; infix = true; break;
   default: assert(0);
   }

   current->beginStatement();

   if (infix)
   {
      if (needsParens)
         out << "(";

      if (node->getLeft())
         node->getLeft()->traverse(goit);
      out << " " << op << " ";
      if (node->getRight())
         node->getRight()->traverse(goit);

      if (needsParens)
         out << ")";
   }
   else
   {
      if (assign)
      {
         // Need to traverse the left child twice to allow for the assign and the op
         // This is OK, because we know it is an lvalue
         if (node->getLeft())
            node->getLeft()->traverse(goit);

         out << " = " << op << "( ";

         if (node->getLeft())
            node->getLeft()->traverse(goit);
         out << ", ";
         if (node->getRight())
            node->getRight()->traverse(goit);

         out << ")";
      }
      else
      {
         out << op << "( ";

         if (node->getLeft())
            node->getLeft()->traverse(goit);
         out << ", ";
         if (node->getRight())
            node->getRight()->traverse(goit);

         out << ")";
      }
   }

   return false;
}

//=========================================================================================================
/// Traverse a unary node generating the output GLSL code
/// \param preVisit
///   Unused parameter, used by base class version
/// \param node
///   Unary node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
bool TGlslOutputTraverser::traverseUnary( bool preVisit, TIntermUnary *node, TIntermTraverser *it )
{
   TString op("??");
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   bool funcStyle = false;
   bool prefix = true;
   char zero[] = "0";

   current->beginStatement();

   switch (node->getOp())
   {
   case EOpNegative:       op = "-";  funcStyle = false; prefix = true; break;
   case EOpVectorLogicalNot:
   case EOpLogicalNot:     op = "!";  funcStyle = false; prefix = true; break;
   case EOpBitwiseNot:     op = "-";  funcStyle = false; prefix = true; break;

   case EOpPostIncrement:  op = "++"; funcStyle = false; prefix = false; break;
   case EOpPostDecrement:  op = "--"; funcStyle = false; prefix = false; break;
   case EOpPreIncrement:   op = "++"; funcStyle = false; prefix = true; break;
   case EOpPreDecrement:   op = "--"; funcStyle = false; prefix = true; break;

   case EOpConvIntToBool:
   case EOpConvFloatToBool:
      op = "bool";
      if ( node->getTypePointer()->getNominalSize() > 1)
      {
         zero[0] += node->getTypePointer()->getNominalSize();
         op = TString("bvec") + zero; 
      }
      funcStyle = true;
      prefix = true;
      break;

   case EOpConvBoolToFloat:
   case EOpConvIntToFloat:
      op = "float";
      if ( node->getTypePointer()->getNominalSize() > 1)
      {
         zero[0] += node->getTypePointer()->getNominalSize();
         op = TString("vec") + zero; 
      }
      funcStyle = true;
      prefix = true;
      break;

   case EOpConvFloatToInt: 
   case EOpConvBoolToInt:
      op = "int";
      if ( node->getTypePointer()->getNominalSize() > 1)
      {
         zero[0] += node->getTypePointer()->getNominalSize();
         op = TString("ivec") + zero; 
      }
      funcStyle = true;
      prefix = true;
      break;

   case EOpRadians:        setupUnaryBuiltInFuncCall ( "radians", node, op, funcStyle, prefix, goit );  break;
   case EOpDegrees:        setupUnaryBuiltInFuncCall ( "degrees", node, op, funcStyle, prefix, goit ); break;
   case EOpSin:            setupUnaryBuiltInFuncCall ( "sin", node, op, funcStyle, prefix, goit ); break;
   case EOpCos:            setupUnaryBuiltInFuncCall ( "cos", node, op, funcStyle, prefix, goit ); break;
   case EOpTan:            setupUnaryBuiltInFuncCall ( "tan", node, op, funcStyle, prefix, goit ); break;
   case EOpAsin:           setupUnaryBuiltInFuncCall ( "asin", node, op, funcStyle, prefix, goit ); break;
   case EOpAcos:           setupUnaryBuiltInFuncCall ( "acos", node, op, funcStyle, prefix, goit ); break;
   case EOpAtan:           setupUnaryBuiltInFuncCall ( "atan", node, op, funcStyle, prefix, goit ); break;
   
   case EOpExp:            setupUnaryBuiltInFuncCall ( "exp", node, op, funcStyle, prefix, goit ); break;
   case EOpLog:            setupUnaryBuiltInFuncCall ( "log", node, op, funcStyle, prefix, goit ); break;
   case EOpExp2:           setupUnaryBuiltInFuncCall ( "exp2", node, op, funcStyle, prefix, goit ); break;
   case EOpLog2:           setupUnaryBuiltInFuncCall ( "log2", node, op, funcStyle, prefix, goit ); break;
   case EOpSqrt:           setupUnaryBuiltInFuncCall ( "sqrt", node, op, funcStyle, prefix, goit ); break;
   case EOpInverseSqrt:    setupUnaryBuiltInFuncCall ( "inversesqrt", node, op, funcStyle, prefix, goit ); break;

   case EOpAbs:            setupUnaryBuiltInFuncCall ( "abs", node, op, funcStyle, prefix, goit ); break;
   case EOpSign:           setupUnaryBuiltInFuncCall ( "sign", node, op, funcStyle, prefix, goit ); break;
   case EOpFloor:          setupUnaryBuiltInFuncCall ( "floor", node, op, funcStyle, prefix, goit ); break;
   case EOpCeil:           setupUnaryBuiltInFuncCall ( "ceil", node, op, funcStyle, prefix, goit ); break;
   case EOpFract:          setupUnaryBuiltInFuncCall ( "fract", node, op, funcStyle, prefix, goit ); break;

   case EOpLength:         op = "length";  funcStyle = true; prefix = true; break;
   case EOpNormalize:      op = "normalize";  funcStyle = true; prefix = true; break;
   case EOpDPdx:           setupUnaryBuiltInFuncCall ( "dFdx", node, op, funcStyle, prefix, goit ); break;
   case EOpDPdy:           setupUnaryBuiltInFuncCall ( "dFdy", node, op, funcStyle, prefix, goit ); break;
   case EOpFwidth:         setupUnaryBuiltInFuncCall ( "fwidth", node, op, funcStyle, prefix, goit ); break;
   case EOpFclip:		   
	  current->addLibFunction(EOpFclip);
      op = "xlat_lib_clip";
      funcStyle = true;
      prefix = true;
      break;    

   case EOpAny:            op = "any";  funcStyle = true; prefix = true; break;
   case EOpAll:            op = "all";  funcStyle = true; prefix = true; break;

      //these are HLSL specific and they map to the lib functions
   case EOpSaturate:
      current->addLibFunction(EOpSaturate);
      op = "xlat_lib_saturate";
      funcStyle = true;
      prefix = true;
      break;    

   case EOpTranspose:
      current->addLibFunction(EOpTranspose);
      op = "xlat_lib_transpose";
      funcStyle = true;
      prefix = true;
      break;

   case EOpDeterminant:
      current->addLibFunction(EOpDeterminant);
      op = "xlat_lib_determinant";
      funcStyle = true;
      prefix = true;
      break;

   case EOpLog10:        
      current->addLibFunction(EOpLog10);
      op = "xlat_lib_log10";
      funcStyle = true;
      prefix = true;
      break;       

   case EOpD3DCOLORtoUBYTE4:
      current->addLibFunction(EOpD3DCOLORtoUBYTE4);
      op = "xlat_lib_D3DCOLORtoUBYTE4";
      funcStyle = true;
      prefix = true;
      break;

   default:
      assert(0);
   }

   if (funcStyle)
      out << op << "( ";
   else
   {
      out << "( ";
      if (prefix)
         out << op;
   }

   node->getOperand()->traverse(goit);

   if (! funcStyle && !prefix)
      out << op;

   out << " )";

   return false;
}

//=========================================================================================================
/// Traverse a selection generating the output GLSL code
/// \param preVisit
///   Unused parameter, used by base class version
/// \param node
///   Selection node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
bool TGlslOutputTraverser::traverseSelection( bool preVisit, TIntermSelection *node, TIntermTraverser *it )
{
	TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
	GlslFunction *current = goit->current;
	std::stringstream& out = current->getActiveOutput();

	current->beginStatement();

	if (node->getBasicType() == EbtVoid)
	{
		// if/else selection
		out << "if ( ";
		node->getCondition()->traverse(goit);
		out << " )";
		current->beginBlock();
		node->getTrueBlock()->traverse(goit);
		current->endBlock();
		if (node->getFalseBlock())
		{
			current->indent();
			out << "else";
			current->beginBlock();
			node->getFalseBlock()->traverse(goit);
			current->endBlock();
		}
	}
	else if (node->isVector() && node->getCondition()->getAsTyped()->isVector())
	{
		// ?: selection on vectors, e.g. bvec4 ? vec4 : vec4
		// emulate HLSL's component-wise selection here
		current->addLibFunction(EOpVecTernarySel);
		out << "xlat_lib_vecTSel (";
		node->getCondition()->traverse(goit);
		out << ", ";
		node->getTrueBlock()->traverse(goit);
		out << ", ";
		if (node->getFalseBlock())
		{
			node->getFalseBlock()->traverse(goit);
		}
		else
			assert(0);
		out << ")";
	}
	else
	{
		// simple ?: selection
		out << "( ";
		node->getCondition()->traverse(goit);
		out << " ) ? ( ";
		node->getTrueBlock()->traverse(goit);
		out << " ) : ( ";
		if (node->getFalseBlock())
		{
			node->getFalseBlock()->traverse(goit);
		}
		else
			assert(0);
		out << " )";
	}

	return false;
}

//=========================================================================================================
/// Traverse an aggregate node generating the output GLSL code
/// \param preVisit
///   Unused parameter, used by base class version
/// \param node
///   Aggregate node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
bool TGlslOutputTraverser::traverseAggregate( bool preVisit, TIntermAggregate *node, TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();
   int argCount = (int) node->getSequence().size();

   if (node->getOp() == EOpNull)
   {
      goit->infoSink.info << "node is still EOpNull!\n";
      return true;
   }


   switch (node->getOp())
   {
   case EOpSequence:
      if (goit->generatingCode )
      {
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
		 for (sit = sequence.begin(); sit != sequence.end(); ++sit)
		 {
		   //current->indent();
		   (*sit)->traverse(it);
		   //out << ";\n";
		   current->endStatement();
		 }
      }
      else
      {
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
		  for (sit = sequence.begin(); sit != sequence.end(); ++sit)
		  {
		    (*sit)->traverse(it);
		  }
      }

      return false;

   case EOpFunction:
      {
         GlslFunction *func = new GlslFunction( node->getPlainName().c_str(), node->getName().c_str(),
                                                translateType(node->getTypePointer()), node->getSemantic().c_str()); 
         if (func->getReturnType() == EgstStruct)
         {
            GlslStruct *s = goit->createStructFromType( node->getTypePointer());
            func->setStruct(s);
         }
         goit->functionList.push_back( func);
         goit->current = func;
         goit->current->beginBlock( false);
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
		 for (sit = sequence.begin(); sit != sequence.end(); ++sit)
		 {
		   (*sit)->traverse(it);
		 }
         goit->current->endBlock();
         goit->current = goit->global;
         return false;
      }

   case EOpParameters:
      it->visitSymbol = traverseParameterSymbol;
      {
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
		 for (sit = sequence.begin(); sit != sequence.end(); ++sit)
           (*sit)->traverse(it);
      }
      it->visitSymbol = traverseSymbol;
      return false;

   case EOpConstructFloat: writeFuncCall( "float", node, goit); return false;
   case EOpConstructVec2:  writeFuncCall( "vec2", node, goit); return false;
   case EOpConstructVec3:  writeFuncCall( "vec3", node, goit); return false;
   case EOpConstructVec4:  writeFuncCall( "vec4", node, goit); return false;
   case EOpConstructBool:  writeFuncCall( "bool", node, goit); return false;
   case EOpConstructBVec2: writeFuncCall( "bvec2", node, goit); return false;
   case EOpConstructBVec3: writeFuncCall( "bvec3", node, goit); return false;
   case EOpConstructBVec4: writeFuncCall( "bvec4", node, goit); return false;
   case EOpConstructInt:   writeFuncCall( "int", node, goit); return false;
   case EOpConstructIVec2: writeFuncCall( "ivec2", node, goit); return false;
   case EOpConstructIVec3: writeFuncCall( "ivec3", node, goit); return false;
   case EOpConstructIVec4: writeFuncCall( "ivec4", node, goit); return false;
   case EOpConstructMat2:  writeFuncCall( "mat2", node, goit); return false;
   case EOpConstructMat3:  writeFuncCall( "mat3", node, goit); return false;
   case EOpConstructMat4:  writeFuncCall( "mat4", node, goit); return false;
   case EOpConstructStruct:  writeFuncCall( "struct", node, goit); return false;


   case EOpConstructMat2FromMat:
      current->addLibFunction(EOpConstructMat2FromMat);
      writeFuncCall( "xlat_lib_constructMat2", node, goit);
      return false;

   case EOpConstructMat3FromMat:
      current->addLibFunction(EOpConstructMat3FromMat);
      writeFuncCall( "xlat_lib_constructMat3", node, goit);
      return false;

   case EOpComma:
      {
         TIntermSequence::iterator sit;
         TIntermSequence &sequence = node->getSequence(); 
         for (sit = sequence.begin(); sit != sequence.end(); ++sit)
         {
            (*sit)->traverse(it);
            if ( sit+1 != sequence.end())
               out << ", ";
         }
      }
      return false;

   case EOpFunctionCall:
      current->addCalledFunction(node->getName().c_str());
      writeFuncCall( node->getPlainName(), node, goit);
      return false; 

   case EOpLessThan:         writeFuncCall( "lessThan", node, goit); return false;
   case EOpGreaterThan:      writeFuncCall( "greaterThan", node, goit); return false;
   case EOpLessThanEqual:    writeFuncCall( "lessThanEqual", node, goit); return false;
   case EOpGreaterThanEqual: writeFuncCall( "greaterThanEqual", node, goit); return false;
   case EOpVectorEqual:      writeFuncCall( "equal", node, goit); return false;
   case EOpVectorNotEqual:   writeFuncCall( "notEqual", node, goit); return false;

   case EOpMod:           writeFuncCall( "mod", node, goit); return false;
   case EOpPow:           writeFuncCall( "pow", node, goit, true); return false;

   case EOpAtan2:         writeFuncCall( "atan", node, goit, true); return false;

   case EOpMin:           writeFuncCall( "min", node, goit, true); return false;
   case EOpMax:           writeFuncCall( "max", node, goit, true); return false;
   case EOpClamp:         writeFuncCall( "clamp", node, goit, true); return false;
   case EOpMix:           writeFuncCall( "mix", node, goit, true); return false;
   case EOpStep:          writeFuncCall( "step", node, goit, true); return false;
   case EOpSmoothStep:    writeFuncCall( "smoothstep", node, goit, true); return false;

   case EOpDistance:      writeFuncCall( "distance", node, goit); return false;
   case EOpDot:           writeFuncCall( "dot", node, goit); return false;
   case EOpCross:         writeFuncCall( "cross", node, goit); return false;
   case EOpFaceForward:   writeFuncCall( "faceforward", node, goit); return false;
   case EOpReflect:       writeFuncCall( "reflect", node, goit); return false;
   case EOpRefract:       writeFuncCall( "refract", node, goit); return false;
   case EOpMul:
      {
         //This should always have two arguments
         assert(node->getSequence().size() == 2);
         current->beginStatement();                     

         out << "( ";
         node->getSequence()[0]->traverse(goit);
         out << " * ";
         node->getSequence()[1]->traverse(goit);
         out << " )";

         return false;
      }

      //HLSL texture functions
   case EOpTex1D:
      if (argCount == 2)
         writeTex( "texture1D", node, goit);
      else
      {
         current->addLibFunction(EOpTex1DGrad);
         writeTex( "xlat_lib_tex1Dgrad", node, goit);
      }
      return false;

   case EOpTex1DProj:     
      writeTex( "texture1DProj", node, goit); 
      return false;

   case EOpTex1DLod:
      current->addLibFunction(EOpTex1DLod);
      writeTex( "xlat_lib_tex1Dlod", node, goit); 
      return false;

   case EOpTex1DBias:
      current->addLibFunction(EOpTex1DBias);
      writeTex( "xlat_lib_tex1Dbias", node, goit); 
      return false;

   case EOpTex1DGrad:     
      current->addLibFunction(EOpTex1DGrad);
      writeTex( "xlat_lib_tex1Dgrad", node, goit); 
      return false;

   case EOpTex2D:
      if (argCount == 2)
         writeTex( "texture2D", node, goit);
      else
      {
         current->addLibFunction(EOpTex2DGrad);
         writeTex( "xlat_lib_tex2Dgrad", node, goit);
      }
      return false;

   case EOpTex2DProj:     
      writeTex( "texture2DProj", node, goit); 
      return false;

   case EOpTex2DLod:      
      current->addLibFunction(EOpTex2DLod);
      writeTex( "xlat_lib_tex2Dlod", node, goit); 
      return false;

   case EOpTex2DBias:  
      current->addLibFunction(EOpTex2DBias);
      writeTex( "xlat_lib_tex2Dbias", node, goit); 
      return false;

   case EOpTex2DGrad:  
      current->addLibFunction(EOpTex2DGrad);
      writeTex( "xlat_lib_tex2Dgrad", node, goit); 
      return false;

   case EOpTex3D:
      if (argCount == 2)
         writeTex( "texture3D", node, goit);
      else
      {
         current->addLibFunction(EOpTex3DGrad);
         writeTex( "xlat_lib_tex3Dgrad", node, goit);            
      }
      return false;

   case EOpTex3DProj:    
      writeTex( "texture3DProj", node, goit); 
      return false;

   case EOpTex3DLod:     
      current->addLibFunction(EOpTex3DLod);
      writeTex( "xlat_lib_tex3Dlod", node, goit); 
      return false;

   case EOpTex3DBias:     
      current->addLibFunction(EOpTex3DBias);
      writeTex( "xlat_lib_tex3Dbias", node, goit); 
      return false;

   case EOpTex3DGrad:    
      current->addLibFunction(EOpTex3DGrad);
      writeTex( "xlat_lib_tex3Dgrad", node, goit); 
      return false;

   case EOpTexCube:
      if (argCount == 2)
         writeTex( "textureCube", node, goit);
      else
      {
         current->addLibFunction(EOpTexCubeGrad);
         writeTex( "xlat_lib_texCUBEgrad", node, goit);
      }
      return false;
   case EOpTexCubeProj:   
      writeTex( "textureCubeProj", node, goit); 
      return false;

   case EOpTexCubeLod:    
      current->addLibFunction(EOpTexCubeLod); 
      writeTex( "xlat_lib_texCUBElod", node, goit); 
      return false;

   case EOpTexCubeBias:   
      current->addLibFunction(EOpTexCubeBias); 
      writeTex( "xlat_lib_texCUBEbias", node, goit); 
      return false;

   case EOpTexCubeGrad:   
      current->addLibFunction(EOpTexCubeGrad);
      writeTex( "xlat_lib_texCUBEgrad", node, goit); 
      return false;

   case EOpTexRect:
	   writeTex( "texture2DRect", node, goit);
	   return false;
	   
   case EOpTexRectProj:
	   writeTex( "texture2DRectProj", node, goit);
	   return false;
		   
   case EOpModf:
      current->addLibFunction(EOpModf);
      writeFuncCall( "xlat_lib_modf", node, goit);
      break;

   case EOpLdexp:
      current->addLibFunction(EOpLdexp);
      writeFuncCall( "xlat_lib_ldexp", node, goit);
      break;

   case EOpSinCos:        
      current->addLibFunction(EOpSinCos);
      writeFuncCall ( "xlat_lib_sincos", node, goit);
      break;

   case EOpItof:         
   case EOpFtoi:         
   case EOpSkipPixels:   
   case EOpReadInput:    
   case EOpWritePixel:   
   case EOpBitmapLsb:    
   case EOpBitmapMsb:    
   case EOpWriteOutput:  
   case EOpReadPixel:    
      // These are are all impossible

   default: goit->infoSink.info << "Bad aggregation op\n";
   }


   return false;
}

//=========================================================================================================
/// Traverse a loop node generating the output GLSL code
/// \param preVisit
///   Unused parameter, used by base class version
/// \param node
///   Loop node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
bool TGlslOutputTraverser::traverseLoop( bool preVisit, TIntermLoop *node, TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();

   current->beginStatement();

   if ( node->getTerminal())
   {
      // Process for loop, initial statement was promoted outside the loop
      out << "for ( ; ";
      node->getTest()->traverse(goit);
      out << "; ";
      node->getTerminal()->traverse(goit);
      out << ") ";
      current->beginBlock();
      node->getBody()->traverse(goit);
      current->endBlock();
   }
   else
   {
      if ( node->testFirst())
      {
         // Process while loop
         out << "while ( ";
         node->getTest()->traverse(goit);
         out << " ) ";
         current->beginBlock();
         node->getBody()->traverse(goit);
         current->endBlock();
      }
      else
      {
         // Process do loop
         out << "do ";
         current->beginBlock();
         node->getBody()->traverse(goit);
         current->endBlock();
         current->indent();
         out << "while ( ";
         node->getTest()->traverse(goit);
         out << " )\n";
      }
   }
   return false;
}

//=========================================================================================================
/// Traverse a branch node generating the output GLSL code
/// \param preVisit
///   Unused parameter, used by base class version
/// \param node
///   Branch node to traverse
/// \param it
///   Pointer to tree traverser
//=========================================================================================================
bool TGlslOutputTraverser::traverseBranch( bool preVisit, TIntermBranch *node,  TIntermTraverser *it )
{
   TGlslOutputTraverser* goit = static_cast<TGlslOutputTraverser*>(it);
   GlslFunction *current = goit->current;
   std::stringstream& out = current->getActiveOutput();

   current->beginStatement();

   switch (node->getFlowOp())
   {
   case EOpKill:      out << "discard";           break;
   case EOpBreak:     out << "break";          break;
   case EOpContinue:  out << "continue";       break;
   case EOpReturn:    out << "return ";         break;
   default:           assert(0); break;
   }

   if (node->getExpression())
   {
      node->getExpression()->traverse(it);
   }

   return false;
}

//=================================================================================================================================
//
//          Public methods block
//
//=================================================================================================================================

//=========================================================================================================
/// Create a GLSL structure from a structure TType
/// \param type
///   Type to convert to a GLSL structure
/// \return
///   Pointer to a new GLSL structure created from the type
//=========================================================================================================
GlslStruct *TGlslOutputTraverser::createStructFromType( TType *type )
{
   GlslStruct *s = 0;
   std::string structName = type->getTypeName().c_str();

   //check for anonymous structures
   if (structName.size() == 0)
   {
      std::stringstream temp;
      TTypeList &tList = *type->getStruct();

      //build a mangled name that is hopefully mangled enough to prevent collisions
      temp << "anonStruct";

      for (TTypeList::iterator it = tList.begin(); it != tList.end(); it++)
      {
         TString typeString;
         it->type->buildMangledName(typeString);
         temp << "_" << typeString.c_str();
      }

      structName = temp.str();
   }

   //try to find the struct name
   if ( structMap.find(structName) == structMap.end() )
   {
      //This is a new structure, build a type for it
      TTypeList &tList = *type->getStruct();

      s = new GlslStruct(structName);

      for (TTypeList::iterator it = tList.begin(); it != tList.end(); it++)
      {
         GlslStruct::member m;
         m.name = it->type->getFieldName().c_str();

         if (it->type->hasSemantic())
            m.semantic = it->type->getSemantic().c_str();

         m.type = translateType( it->type);
         m.arraySize = it->type->isArray() ? it->type->getArraySize() : 0;
         s->addMember(m);
      }

      //add it to the list
      structMap[structName] = s;
      structList.push_back(s);
   }
   else
   {
      s = structMap[structName];
   }

   return s;
}

//=========================================================================================================
/// Parse a binary node that represents an variable initalizer
/// \param node
///   Binary intermediate node to parse
/// \return
///   true if initializer was parsed succesfully (with new symbol added if needed), false otherwise.
///   The symbol node will have its initializer set
//=========================================================================================================
bool TGlslOutputTraverser::parseInitializer( TIntermBinary *node )
{
   TIntermTyped *left, *right;

   left = node->getLeft();
   right = node->getRight();

   if (! left->getAsSymbolNode())
      return false; //Something is likely seriously wrong

   if (! right->getAsConstantUnion())
      return false; //only constant initializers

   TIntermSymbol *symNode = left->getAsSymbolNode();
   TIntermConstantUnion *cUnion = right->getAsConstantUnion();

   if (symNode->getBasicType() == EbtStruct)
      return false;

   GlslSymbol * sym = NULL;

   if ( !current->hasSymbol( symNode->getId() ) )
   {
      int array = symNode->getTypePointer()->isArray() ? symNode->getTypePointer()->getArraySize() : 0;
      const char* semantic = "";

      if (symNode->getInfo())
         semantic = symNode->getInfo()->getSemantic().c_str();

      sym = new GlslSymbol( symNode->getSymbol().c_str(), semantic, symNode->getId(),
                            translateType(symNode->getTypePointer()), translateQualifier(symNode->getQualifier()), array);

      current->addSymbol(sym);
   }
   else
      return false; //can't init already declared variable


   sym->setInitializer ( cUnion->getUnionArrayPointer() );

   return true;
}