// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "ParseHelper.h"
#include "../Include/InitializeParseContext.h"
#include "osinclude.h"
#include <stdarg.h>


// ------------------------------------------------------------------
// Sub- vector and matrix fields


//
// Look at a '.' field selector string and change it into offsets
// for a vector.
//
bool TParseContext::parseVectorFields(const TString& compString, int vecSize, TVectorFields& fields, int line)
{
   fields.num = (int) compString.size();
   if (fields.num > 4)
   {
      error(line, "illegal vector field selection", compString.c_str(), "");
      return false;
   }

   enum
   {
      exyzw,
      ergba,
      estpq,
   } fieldSet[4];

   for (int i = 0; i < fields.num; ++i)
   {
      switch (compString[i])
      {
      case 'x': 
         fields.offsets[i] = 0;
         fieldSet[i] = exyzw;
         break;
      case 'r': 
         fields.offsets[i] = 0;
         fieldSet[i] = ergba;
         break;
      case 's':
         fields.offsets[i] = 0;
         fieldSet[i] = estpq;
         break;
      case 'y': 
         fields.offsets[i] = 1;
         fieldSet[i] = exyzw;
         break;
      case 'g': 
         fields.offsets[i] = 1;
         fieldSet[i] = ergba;
         break;
      case 't':
         fields.offsets[i] = 1;
         fieldSet[i] = estpq;
         break;
      case 'z': 
         fields.offsets[i] = 2;
         fieldSet[i] = exyzw;
         break;
      case 'b': 
         fields.offsets[i] = 2;
         fieldSet[i] = ergba;
         break;
      case 'p':
         fields.offsets[i] = 2;
         fieldSet[i] = estpq;
         break;

      case 'w': 
         fields.offsets[i] = 3;
         fieldSet[i] = exyzw;
         break;
      case 'a': 
         fields.offsets[i] = 3;
         fieldSet[i] = ergba;
         break;
      case 'q':
         fields.offsets[i] = 3;
         fieldSet[i] = estpq;
         break;
      default:
         error(line, "illegal vector field selection", compString.c_str(), "");
         return false;
      }
   }

   for (int i = 0; i < fields.num; ++i)
   {
      if (fields.offsets[i] >= vecSize)
      {
         error(line, "vector field selection out of range",  compString.c_str(), "");
         return false;
      }

      if (i > 0)
      {
         if (fieldSet[i] != fieldSet[i-1])
         {
            error(line, "illegal - vector component fields not from the same set", compString.c_str(), "");
            return false;
         }
      }
   }

   return true;
}


//
// Look at a '.' field selector string and change it into offsets
// for a matrix.
//
bool TParseContext::parseMatrixFields(const TString& compString, int matSize, TVectorFields& fields, int line)
{
   fields.num = 1;
   fields.offsets[0] = 0;

   if (compString.size() < 3  || compString[0] != '_')
   {
      error(line, "illegal matrix field selection", compString.c_str(), "");
      return false;
   }

   if (compString[1] == 'm')
   {
      //The selection is 0 based with the syntax _m## 
      if ( (compString.size() % 4) != 0 || compString.size() > 16)
      {
         error(line, "illegal matrix field selection", compString.c_str(), "");
         return false;
      }
      for (int ii = 0; ii < (int)compString.size(); ii+=4)
      {
         if (compString[ii] != '_' || compString[ii + 1] != 'm')
         {
            error(line, "illegal matrix field selection", compString.c_str(), "");
            return false;
         }
         if (compString[ii + 2] < '0' || compString[ii + 2] > '3' ||
             compString[ii + 3] < '0' || compString[ii + 3] > '3')
         {
            error(line, "illegal matrix field selection", compString.c_str(), "");
            return false;
         }
         int row = compString[ii + 2] - '0';
         int collumn = compString[ii + 3] - '0';
         if ( row >= matSize || collumn >= matSize)
         {
            error(line, "matrix field selection out of range", compString.c_str(), "");
            return false;
         }
         fields.offsets[ii/4] =  row * 4 + collumn;
      }
      fields.num = compString.size()/4;
   }
   else
   {
      //The selection is 1 based with the syntax _##
      if ( (compString.size() % 3) != 0 || compString.size() > 12)
      {
         error(line, "illegal matrix field selection", compString.c_str(), "");
         return false;
      }
      for (int ii = 0; ii < (int)compString.size(); ii += 3)
      {
         if (compString[ii] != '_')
         {
            error(line, "illegal matrix field selection", compString.c_str(), "");
            return false;
         }
         if (compString[ii + 1] < '1' || compString[ii + 1] > '4' ||
             compString[ii + 2] < '1' || compString[ii + 2] > '4')
         {
            error(line, "illegal matrix field selection", compString.c_str(), "");
            return false;
         }
         int row = compString[ii + 1] - '1';
         int collumn = compString[ii + 2] - '1';
         if ( row >= matSize || collumn >= matSize)
         {
            error(line, "matrix field selection out of range", compString.c_str(), "");
            return false;
         }
         fields.offsets[ii/3] =  row * 4 + collumn;
      }
      fields.num = compString.size()/3;
   }

   return true;
}

// ------------------------------------------------------------------
// Errors

//
// Track whether errors have occurred.
//
void TParseContext::recover()
{
   recoveredFromError = true;
}

//
// Used by flex/bison to output all syntax and parsing errors.
//
void C_DECL TParseContext::error(TSourceLoc nLine, const char *szReason, const char *szToken, 
                                 const char *szExtraInfoFormat, ...)
{
   char szExtraInfo[400];
   va_list marker;

   va_start(marker, szExtraInfoFormat);

   _vsnprintf(szExtraInfo, sizeof(szExtraInfo), szExtraInfoFormat, marker);

   /* VC++ format: file(linenum) : error #: 'token' : extrainfo */
   infoSink.info.prefix(EPrefixError);
   infoSink.info.location(nLine);
   infoSink.info << "'" << szToken <<  "' : " << szReason << " " << szExtraInfo << "\n";

   va_end(marker);

   ++numErrors;
}

//
// Same error message for all places assignments don't work.
//
void TParseContext::assignError(int line, const char* op, TString left, TString right)
{
   error(line, "", op, "cannot convert from '%s' to '%s'",
         right.c_str(), left.c_str());
}

//
// Same error message for all places unary operations don't work.
//
void TParseContext::unaryOpError(int line, char* op, TString operand)
{
   error(line, " wrong operand type", op, 
         "no operation '%s' exists that takes an operand of type %s (or there is no acceptable conversion)",
         op, operand.c_str());
}

//
// Same error message for all binary operations don't work.
//
void TParseContext::binaryOpError(int line, char* op, TString left, TString right)
{
   error(line, " wrong operand types ", op, 
         "no operation '%s' exists that takes a left-hand operand of type '%s' and "
         "a right operand of type '%s' (or there is no acceptable conversion)", 
         op, left.c_str(), right.c_str());
}

//
// Both test and if necessary, spit out an error, to see if the node is really
// an l-value that can be operated on this way.
//
// Returns true if the was an error.
//
bool TParseContext::lValueErrorCheck(int line, char* op, TIntermTyped* node)
{
   TIntermSymbol* symNode = node->getAsSymbolNode();
   TIntermBinary* binaryNode = node->getAsBinaryNode();

   if (binaryNode)
   {
      bool errorReturn;

      switch (binaryNode->getOp())
      {
      case EOpIndexDirect:
      case EOpIndexIndirect:
      case EOpIndexDirectStruct:
         return lValueErrorCheck(line, op, binaryNode->getLeft());
      case EOpVectorSwizzle:
         errorReturn = lValueErrorCheck(line, op, binaryNode->getLeft());
         if (!errorReturn)
         {
            int offset[4] = {0,0,0,0};

            TIntermTyped* rightNode = binaryNode->getRight();
            TIntermAggregate *aggrNode = rightNode->getAsAggregate();

            for (TIntermSequence::iterator p = aggrNode->getSequence().begin(); 
                p != aggrNode->getSequence().end(); p++)
            {
               int value = (*p)->getAsTyped()->getAsConstantUnion()->getUnionArrayPointer()->getIConst();
               offset[value]++;     
               if (offset[value] > 1)
               {
                  error(line, " l-value of swizzle cannot have duplicate components", op, "", "");

                  return true;
               }
            }
         }
      case EOpMatrixSwizzle:
         errorReturn = lValueErrorCheck(line, op, binaryNode->getLeft());
         if (!errorReturn)
         {
            int offset[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

            TIntermTyped* rightNode = binaryNode->getRight();
            TIntermAggregate *aggrNode = rightNode->getAsAggregate();

            for (TIntermSequence::iterator p = aggrNode->getSequence().begin(); 
                p != aggrNode->getSequence().end(); p++)
            {
               int value = (*p)->getAsTyped()->getAsConstantUnion()->getUnionArrayPointer()->getIConst();
               offset[value]++;     
               if (offset[value] > 1)
               {
                  error(line, " l-value of swizzle cannot have duplicate components", op, "", "");

                  return true;
               }
            }
         }

         return errorReturn;
      default: 
         break;
      }
      error(line, " l-value required", op, "", "");

      return true;
   }


   const char* symbol = 0;
   if (symNode != 0)
      symbol = symNode->getSymbol().c_str();

   const char* message = 0;
   switch (node->getQualifier())
   {
   case EvqConst:          message = "can't modify a const";        break;
   case EvqConstReadOnly:  message = "can't modify a const";        break;
   case EvqAttribute:      message = "can't modify an attribute";   break;
   case EvqUniform:
      // mark this uniform as mutable
      node->getTypePointer()->changeQualifier( EvqMutableUniform);
      break;
   case EvqVaryingIn:      message = "can't modify a varying";      break;
   case EvqInput:          message = "can't modify an input";       break;
   case EvqFace:           message = "can't modify gl_FrontFacing";   break;
   case EvqFragCoord:      message = "can't modify gl_FragCoord";   break;
   default:

      //
      // Type that can't be written to?
      //
      switch (node->getBasicType())
      {
      case EbtSamplerGeneric:
      case EbtSampler1D:
      case EbtSampler2D:
      case EbtSampler3D:
      case EbtSamplerCube:
      case EbtSampler1DShadow:
      case EbtSampler2DShadow:
      case EbtSamplerRect:       // ARB_texture_rectangle
      case EbtSamplerRectShadow: // ARB_texture_rectangle
         message = "can't modify a sampler";
         break;
      case EbtVoid:
         message = "can't modify void";
         break;
      default: 
         break;
      }
   }

   if (message == 0 && binaryNode == 0 && symNode == 0)
   {
      error(line, " l-value required", op, "", "");

      return true;
   }


   //
   // Everything else is okay, no error.
   //
   if (message == 0)
      return false;

   //
   // If we get here, we have an error and a message.
   //
   if (symNode)
      error(line, " l-value required", op, "\"%s\" (%s)", symbol, message);
   else
      error(line, " l-value required", op, "(%s)", message);

   return true;
}

//
// Both test, and if necessary spit out an error, to see if the node is really
// a constant.
//
// Returns true if the was an error.
//
bool TParseContext::constErrorCheck(TIntermTyped* node)
{
   if (node->getQualifier() == EvqConst)
      return false;

   error(node->getLine(), "constant expression required", "", "");

   return true;
}

//
// Both test, and if necessary spit out an error, to see if the node is really
// an integer.
//
// Returns true if the was an error.
//
bool TParseContext::integerErrorCheck(TIntermTyped* node, char* token)
{
   if (node->getBasicType() == EbtInt && node->getNominalSize() == 1)
      return false;

   error(node->getLine(), "integer expression required", token, "");

   return true;
}

//
// Both test, and if necessary spit out an error, to see if we are currently
// globally scoped.
//
// Returns true if the was an error.
//
bool TParseContext::globalErrorCheck(int line, bool global, char* token)
{
   if (global)
      return false;

   error(line, "only allowed at global scope", token, "");

   return true;
}

//
// For now, keep it simple:  if it starts "gl_", it's reserved, independent
// of scope.  Except, if the symbol table is at the built-in push-level,
// which is when we are parsing built-ins.
//
// Returns true if there was an error.
//
bool TParseContext::reservedErrorCheck(int line, const TString& identifier)
{
   if (!symbolTable.atBuiltInLevel())
   {
      if (identifier.substr(0, 3) == TString("gl_"))
      {
         error(line, "reserved built-in name", "gl_", "");
         return true;
      }
      if (identifier.find("__") != TString::npos)
      {
         //error(line, "Two consecutive underscores are reserved for future use.", identifier.c_str(), "", "");
         //return true;
         infoSink.info.message(EPrefixWarning, "Two consecutive underscores are reserved for future use.", line);
         return false;
      }
   }

   return false;
}

//
// Make sure there is enough data provided to the constructor to build
// something of the type of the constructor.  Also returns the type of
// the constructor.
//
// Returns true if there was an error in construction.
//
bool TParseContext::constructorErrorCheck(int line, TIntermNode* node, TFunction& function, TOperator op, TType* type)
{
   *type = function.getReturnType();

   bool constructingMatrix = false;
   switch (op)
   {
   case EOpConstructMat2:
   case EOpConstructMat3:
   case EOpConstructMat4:
      constructingMatrix = true;
      break;
   default: 
      break;
   }

   //
   // Note: It's okay to have too many components available, but not okay to have unused
   // arguments.  'full' will go to true when enough args have been seen.  If we loop
   // again, there is an extra argument, so 'overfull' will become true.
   //

   int size = 0;
   bool constType = true;
   bool full = false;
   bool overFull = false;
   bool matrixInMatrix = false;
   bool arrayArg = false;
   for (int i = 0; i < function.getParamCount(); ++i)
   {
      size += function[i].type->getObjectSize();

      if (constructingMatrix && function[i].type->isMatrix())
         matrixInMatrix = true;
      if (full)
         overFull = true;
      if (op != EOpConstructStruct && !type->isArray() && size >= type->getObjectSize())
         full = true;
      if (function[i].type->getQualifier() != EvqConst)
         constType = false;
      if (function[i].type->isArray())
         arrayArg = true;
   }

   if (constType)
      type->changeQualifier(EvqConst);

   if (type->isArray() && type->getArraySize() != function.getParamCount())
   {
      error(line, "array constructor needs one argument per array element", "constructor", "");
      return true;
   }

   if (arrayArg && op != EOpConstructStruct)
   {
      error(line, "constructing from a non-dereferenced array", "constructor", "");
      return true;
   }

   if (overFull)
   {
      error(line, "too many arguments", "constructor", "");
      return true;
   }

   if (op == EOpConstructStruct && !type->isArray() && (type->getStruct()->size() != function.getParamCount() && function.getParamCount() != 1))
   {
      error(line, "Number of constructor parameters does not match the number of structure fields", "constructor", "");
      return true;
   }

   if (( size != 1 && size < type->getObjectSize()) ||
       (op == EOpConstructStruct && size < type->getObjectSize()) && size > 1)
   {
      error(line, "not enough data provided for construction", "constructor", "");
      return true;
   }

   TIntermTyped* typed = node->getAsTyped();
   if (typed == 0)
   {
      error(line, "constructor argument does not have a type", "constructor", "");
      return true;
   }
   if (op != EOpConstructStruct && IsSampler(typed->getBasicType()))
   {
      error(line, "cannot convert a sampler", "constructor", "");
      return true;
   }
   if (typed->getBasicType() == EbtVoid)
   {
      error(line, "cannot convert a void", "constructor", "");
      return true;
   }

   return false;
}

// This function checks to see if a void variable has been declared and raise an error message for such a case
//
// returns true in case of an error
//
bool TParseContext::voidErrorCheck(int line, const TString& identifier, const TPublicType& pubType)
{
   if (pubType.type == EbtVoid)
   {
      error(line, "illegal use of type 'void'", identifier.c_str(), "");
      return true;
   }

   return false;
}

// This function checks to see if the node (for the expression) contains a scalar boolean expression or not
//
// returns true in case of an error
//
bool TParseContext::boolErrorCheck(int line, const TIntermTyped* type)
{
   // In HLSL, any float or int will be automatically casted to a bool, so the basic type can be bool,
   // float, or int.
   if ((type->getBasicType() != EbtBool && type->getBasicType() != EbtInt && type->getBasicType() != EbtFloat) || 
        type->isArray() || type->isMatrix() || type->isVector())
   {
      error(line, "boolean expression expected", "", "");
      return true;
   }

   return false;
}


bool TParseContext::boolOrVectorErrorCheck(int line, const TIntermTyped* type)
{
	// In HLSL, any float or int will be automatically casted to a bool, so the basic type can be bool,
	// float, or int.
	if ((type->getBasicType() != EbtBool && type->getBasicType() != EbtInt && type->getBasicType() != EbtFloat) || 
        type->isArray() || type->isMatrix())
	{
		error(line, "boolean or vector expression expected", "", "");
		return true;
	}
	
	return false;
}


// This function checks to see if the node (for the expression) contains a scalar boolean expression or not
//
// returns true in case of an error
//
bool TParseContext::boolErrorCheck(int line, const TPublicType& pType)
{
   // In HLSL, any float or int will be automatically casted to a bool, so the basic type can be bool,
   // float, or int.   
   if ((pType.type != EbtBool && pType.type != EbtInt && pType.type != EbtFloat) || 
        pType.array || pType.matrix || (pType.size > 1))
   {
      error(line, "boolean expression expected", "", "");
      return true;
   }

   return false;
}

bool TParseContext::samplerErrorCheck(int line, const TPublicType& pType, const char* reason)
{
   if (pType.type == EbtStruct)
   {
      if (containsSampler(*pType.userDef))
      {
         error(line, reason, TType::getBasicString(pType.type), "(structure contains a sampler)");

         return true;
      }

      return false;
   }
   else if (IsSampler(pType.type))
   {
      error(line, reason, TType::getBasicString(pType.type), "");

      return true;
   }

   return false;
}

bool TParseContext::structQualifierErrorCheck(int line, const TPublicType& pType)
{
   if ((pType.qualifier == EvqVaryingIn || pType.qualifier == EvqVaryingOut || pType.qualifier == EvqAttribute) &&
       pType.type == EbtStruct)
   {
      error(line, "cannot be used with a structure", getQualifierString(pType.qualifier), "");

      return true;
   }


   return false;
}

bool TParseContext::parameterSamplerErrorCheck(int line, TQualifier qualifier, const TType& type)
{
   if ((qualifier == EvqOut || qualifier == EvqInOut) && 
       type.getBasicType() != EbtStruct && IsSampler(type.getBasicType()))
   {
      error(line, "samplers cannot be output parameters", type.getBasicString(), "");
      return true;
   }

   return false;
}

bool TParseContext::containsSampler(TType& type)
{
   if (IsSampler(type.getBasicType()))
      return true;

   if (type.getBasicType() == EbtStruct)
   {
      TTypeList& structure = *type.getStruct();
      for (unsigned int i = 0; i < structure.size(); ++i)
      {
         if (containsSampler(*structure[i].type))
            return true;
      }
   }

   return false;
}

bool TParseContext::insertBuiltInArrayAtGlobalLevel()
{
   TString *name = NewPoolTString("gl_TexCoord");
   TSymbol* symbol = symbolTable.find(*name);
   if (!symbol)
   {
      error(0, "INTERNAL ERROR finding symbol", name->c_str(), "");
      return true;
   }
   TVariable* variable = static_cast<TVariable*>(symbol);

   TVariable* newVariable = new TVariable(name, variable->getType());

   if (! symbolTable.insert(*newVariable))
   {
      delete newVariable;
      error(0, "INTERNAL ERROR inserting new symbol", name->c_str(), "");
      return true;
   }

   return false;
}



//
// Do size checking for an array type's size.
//
// Returns true if there was an error.
//
bool TParseContext::arraySizeErrorCheck(int line, TIntermTyped* expr, int& size)
{
   TIntermConstantUnion* constant = expr->getAsConstantUnion();
   if (constant == 0 || constant->getBasicType() != EbtInt)
   {
      error(line, "array size must be a constant integer expression", "", "");
      return true;
   }

   size = constant->getUnionArrayPointer()->getIConst();

   if (size <= 0)
   {
      error(line, "array size must be a positive integer", "", "");
      size = 1;
      return true;
   }

   return false;
}

//
// See if this qualifier can be an array.
//
// Returns true if there is an error.
//
bool TParseContext::arrayQualifierErrorCheck(int line, TPublicType type)
{
   if (type.qualifier == EvqAttribute)
   {
      error(line, "cannot declare arrays of this qualifier", TType(type).getCompleteString().c_str(), "");
      return true;
   }

   return false;
}

//
// See if this type can be an array.
//
// Returns true if there is an error.
//
bool TParseContext::arrayTypeErrorCheck(int line, TPublicType type)
{
   //
   // Can the type be an array?
   //
   if (type.array)
   {
      error(line, "multidimentional arrays ar not supported", TType(type).getCompleteString().c_str(), "");
      return true;
   }

   return false;
}

//
// Do all the semantic checking for declaring an array, with and 
// without a size, and make the right changes to the symbol table.
//
// size == 0 means no specified size.
//
// Returns true if there was an error.
//
bool TParseContext::arrayErrorCheck(int line, TString& identifier, TPublicType type, TVariable*& variable)
{

   return arrayErrorCheck( line, identifier, 0, type, variable);
}

//
// Do all the semantic checking for declaring an array, with and 
// without a size, and make the right changes to the symbol table.
//
// size == 0 means no specified size.
//
// Returns true if there was an error.
//
bool TParseContext::arrayErrorCheck(int line, TString& identifier, const TTypeInfo *info, TPublicType type, TVariable*& variable)
{
   //
   // Don't check for reserved word use until after we know it's not in the symbol table,
   // because reserved arrays can be redeclared.
   //

   switch (type.qualifier)
   {
   case EvqGlobal: type.qualifier = EvqUniform; break; // according to hlsl, non static globals are uniforms
   case EvqStatic: type.qualifier = EvqGlobal; break;
   case EvqStaticConst: type.qualifier = EvqConst; break;
   }

   bool builtIn = false; 
   bool sameScope = false;
   TSymbol* symbol = symbolTable.find(identifier, &builtIn, &sameScope);
   if (symbol == 0 || !sameScope)
   {
      if (reservedErrorCheck(line, identifier))
         return true;

      variable = new TVariable(&identifier, info, TType(type));

      if (type.arraySize)
         variable->getType().setArraySize(type.arraySize);

      if (! symbolTable.insert(*variable))
      {
         delete variable;
         error(line, "INTERNAL ERROR inserting new symbol", identifier.c_str(), "");
         return true;
      }
   }
   else
   {
      if (! symbol->isVariable())
      {
         error(line, "variable expected", identifier.c_str(), "");
         return true;
      }

      variable = static_cast<TVariable*>(symbol);
      if (! variable->getType().isArray())
      {
         error(line, "redeclaring non-array as array", identifier.c_str(), "");
         return true;
      }
      if (variable->getType().getArraySize() > 0)
      {
         error(line, "redeclaration of array with size", identifier.c_str(), "");
         return true;
      }

      if (! variable->getType().sameElementType(TType(type)))
      {
         error(line, "redeclaration of array with a different type", identifier.c_str(), "");
         return true;
      }

      TType* t = variable->getArrayInformationType();
      while (t != 0)
      {
         if (t->getMaxArraySize() > type.arraySize)
         {
            error(line, "higher index value already used for the array", identifier.c_str(), "");
            return true;
         }
         t->setArraySize(type.arraySize);
         t = t->getArrayInformationType();
      }

      if (type.arraySize)
         variable->getType().setArraySize(type.arraySize);
   } 

   if (voidErrorCheck(line, identifier, type))
      return true;

   return false;
}

bool TParseContext::arraySetMaxSize(TIntermSymbol *node, TType* type, int size, bool updateFlag, TSourceLoc line)
{
   bool builtIn = false;
   TSymbol* symbol = symbolTable.find(node->getSymbol(), &builtIn);
   if (symbol == 0)
   {
      error(line, " undeclared identifier", node->getSymbol().c_str(), "");
      return true;
   }
   TVariable* variable = static_cast<TVariable*>(symbol);

   type->setArrayInformationType(variable->getArrayInformationType());
   variable->updateArrayInformationType(type);

   // special casing to test index value of gl_TexCoord. If the accessed index is >= gl_MaxTextureCoords
   // its an error
   if (node->getSymbol() == "gl_TexCoord")
   {
      TSymbol* texCoord = symbolTable.find("gl_MaxTextureCoords", &builtIn);
      if (texCoord == 0)
      {
         infoSink.info.message(EPrefixInternalError, "gl_MaxTextureCoords not defined", line);
         return true;
      }

      int texCoordValue = static_cast<TVariable*>(texCoord)->getConstPointer()[0].getIConst();
      if (texCoordValue <= size)
      {
         error(line, "", "[", "gl_TexCoord can only have a max array size of up to gl_MaxTextureCoords", "");
         return true;
      }
   }

   // we dont want to update the maxArraySize when this flag is not set, we just want to include this 
   // node type in the chain of node types so that its updated when a higher maxArraySize comes in.
   if (!updateFlag)
      return false;

   size++;
   variable->getType().setMaxArraySize(size);
   type->setMaxArraySize(size);
   TType* tt = type;

   while (tt->getArrayInformationType() != 0)
   {
      tt = tt->getArrayInformationType();
      tt->setMaxArraySize(size);
   }

   return false;
}

//
// Enforce non-initializer type/qualifier rules.
//
// Returns true if there was an error.
//
bool TParseContext::nonInitConstErrorCheck(int line, TString& identifier, TPublicType& type)
{
   //
   // Make the qualifier make sense.
   //
   if (type.qualifier == EvqConst)
   {
      type.qualifier = EvqTemporary;
      error(line, "variables with qualifier 'const' must be initialized", identifier.c_str(), "");
      return true;
   }

   return false;
}

//
// Do semantic checking for a variable declaration that has no initializer,
// and update the symbol table.
//
// Returns true if there was an error.
//
bool TParseContext::nonInitErrorCheck(int line, TString& identifier, TPublicType& type)
{

   return nonInitErrorCheck( line, identifier, 0, type);
}

//
// Do semantic checking for a variable declaration that has no initializer,
// and update the symbol table.
//
// Returns true if there was an error.
//
bool TParseContext::nonInitErrorCheck(int line, TString& identifier, const TTypeInfo *info, TPublicType& type)
{
   if (reservedErrorCheck(line, identifier))
      recover();

   switch (type.qualifier)
   {
   case EvqGlobal: type.qualifier = EvqUniform; break; // according to hlsl, non static globals are uniforms
   case EvqStatic: type.qualifier = EvqGlobal; break;
   case EvqStaticConst: type.qualifier = EvqConst; break;
   }

   TVariable* variable = new TVariable(&identifier, info, TType(type));

   if (! symbolTable.insert(*variable))
   {
      error(line, "redefinition", variable->getName().c_str(), "");
      delete variable;
      return true;
   }

   if (voidErrorCheck(line, identifier, type))
      return true;

   return false;
}

bool TParseContext::paramErrorCheck(int line, TQualifier qualifier, TQualifier paramQualifier, TType* type)
{
   if (qualifier != EvqConst && qualifier != EvqTemporary && qualifier != EvqUniform)
   {
      error(line, "qualifier not allowed on function parameter", getQualifierString(qualifier), "");
      return true;
   }
   if (qualifier == EvqConst && paramQualifier != EvqIn)
   {
      error(line, "qualifier not allowed with ", getQualifierString(qualifier), getQualifierString(paramQualifier));
      return true;
   }

   if (qualifier == EvqUniform)
      type->changeQualifier(EvqUniform);
   else if (qualifier == EvqConst)
      type->changeQualifier(EvqConstReadOnly);
   else
      type->changeQualifier(paramQualifier);

   return false;
}


// ------------------------------------------------------------------
// Non-Errors.


//
// Look up a function name in the symbol table, and make sure it is a function.
//
// Return the function symbol if found, otherwise 0.
//
const TFunction* TParseContext::findFunction(int line, TFunction* call, bool *builtIn)
{
   const TSymbol* symbol = symbolTable.find(call->getMangledName(), builtIn);

   if (symbol == 0)
   {
      // The symbol was not found, look for a unambiguous type conversion
      bool ambiguous = false;

      symbol = symbolTable.findCompatible( call, builtIn, ambiguous);

      if (symbol == 0)
      {
         if (ambiguous)
            error(line, "cannot resolve function call unambiguously", call->getName().c_str(), "(check parameter types)");
         else
            error(line, "no matching overloaded function found", call->getName().c_str(), "");
         return 0;
      }
   }

   if (! symbol->isFunction())
   {
      error(line, "function name expected", call->getName().c_str(), "");
      return 0;
   }

   const TFunction* function = static_cast<const TFunction*>(symbol);

   return function;
}

//
// Initializers show up in several places in the grammar.  Have one set of
// code to handle them here.
//
bool TParseContext::executeInitializer(TSourceLoc line, TString& identifier, TPublicType& pType, 
                                       TIntermTyped* initializer, TIntermNode*& intermNode, TVariable* variable)
{

   return executeInitializer( line, identifier, 0, pType, initializer, intermNode, variable);
}

//
// Initializers show up in several places in the grammar.  Have one set of
// code to handle them here.
//
bool TParseContext::executeInitializer(TSourceLoc line, TString& identifier, const TTypeInfo *info, TPublicType& pType, 
                                       TIntermTyped* initializer, TIntermNode*& intermNode, TVariable* variable)
{
   switch (pType.qualifier)
   {
   case EvqGlobal: pType.qualifier = EvqUniform; break; // according to hlsl, non static globals are uniforms
   case EvqStatic: pType.qualifier = EvqGlobal; break;
   case EvqStaticConst: pType.qualifier = EvqConst; break;
   }

   // HLSL2GLSL fails spectacularly on const array variables.
   // I can't quite untangle where it messes them up, so as a workaround let's
   // change const arrays to regular arrays.
   if (pType.array && pType.qualifier == EvqConst && variable)
   {
	   pType.qualifier = EvqTemporary;
	   variable->changeQualifier (EvqTemporary);
   }

   TType type = TType(pType);

   //check to see if we have a blind aggregate
   TIntermAggregate *agg = initializer->getAsAggregate();

   if (agg && agg->getOp() == EOpNull)
   {
      //temporarily replace the qualifier to create a const type
      TQualifier qual = pType.qualifier;
      pType.qualifier = EvqConst;
      TType tType = TType(pType);
      pType.qualifier = qual;

      initializer = addConstructor( agg, &tType, getConstructorOp(tType), 0, agg->getLine());
      if ( initializer == NULL )
         return true;
   }

   if (variable == 0)
   {
      if (reservedErrorCheck(line, identifier))
         return true;

      if (voidErrorCheck(line, identifier, pType))
         return true;

      //
      // add variable to symbol table
      //
      variable = new TVariable(&identifier, info, type);
      if (! symbolTable.insert(*variable))
      {
         error(line, "redefinition", variable->getName().c_str(), "");
         return true;
         // don't delete variable, it's used by error recovery, and the pool 
         // pop will take care of the memory
      }
   }

   //
   // identifier must be of type constant, a global, or a temporary
   //
   TQualifier qualifier = variable->getType().getQualifier();
   if ((qualifier != EvqTemporary) && (qualifier != EvqGlobal) && (qualifier != EvqConst) && (qualifier != EvqUniform))
   {
      error(line, " cannot initialize this type of qualifier ", variable->getType().getQualifierString(), "");
      return true;
   }

   //
   // test for and propagate constant
   //

   if (qualifier == EvqConst)
   {
      if (qualifier != initializer->getType().getQualifier())
      {
         error(line, " assigning non-constant to", "=", "'%s'", variable->getType().getCompleteString().c_str());
         variable->getType().changeQualifier(EvqTemporary);
         return true;
      }
      if (type != initializer->getType())
      {
         error(line, " non-matching types for const initializer ", 
               variable->getType().getQualifierString(), "");
         variable->getType().changeQualifier(EvqTemporary);
         return true;
      }
      if (initializer->getAsConstantUnion())
      {
         constUnion* unionArray = variable->getConstPointer();

         if (type.getObjectSize() == 1 && type.getBasicType() != EbtStruct)
         {
            *unionArray = (initializer->getAsConstantUnion()->getUnionArrayPointer())[0];
         }
         else
         {
            variable->shareConstPointer(initializer->getAsConstantUnion()->getUnionArrayPointer());
         }
      }
      else if (initializer->getAsSymbolNode())
      {
         const TSymbol* symbol = symbolTable.find(initializer->getAsSymbolNode()->getSymbol());
         const TVariable* tVar = static_cast<const TVariable*>(symbol);

         constUnion* constArray = tVar->getConstPointer();
         variable->shareConstPointer(constArray);
      }
      else
      {
         error(line, " cannot assign to", "=", "'%s'", variable->getType().getCompleteString().c_str());
         variable->getType().changeQualifier(EvqTemporary);
         return true;
      }
   }

   if (qualifier == EvqUniform )
   {
      if ( initializer->getType().getQualifier() != EvqConst)
      {
         error(line, " Attempting to initialize uniform with non-constant", "", "");
         variable->getType().changeQualifier(EvqTemporary);
         return true;
      }
      if (type != initializer->getType())
      {
         //TODO: might need type promotion here
      }
      if (! initializer->getAsConstantUnion())
      {
         error(line, " Uniform intializers must be literal constants", "", "");
         variable->getType().changeQualifier(EvqTemporary);
         return true;
      }
   }

   //TODO: need to complicate this some...
   if (qualifier != EvqConst)
   {
      TIntermSymbol* intermSymbol = intermediate.addSymbol(variable->getUniqueId(), variable->getName(), variable->getInfo(), variable->getType(), line);
      intermNode = intermediate.addAssign(EOpInitialize, intermSymbol, initializer, line);
      if (intermNode == 0)
      {
         assignError(line, "=", intermSymbol->getCompleteString(), initializer->getCompleteString());
         return true;
      }
   }
   else
      intermNode = 0;

   return false;
}

bool TParseContext::areAllChildConst(TIntermAggregate* aggrNode)
{
   if (!aggrNode->isConstructor())
      return false;

   bool allConstant = true;

   // check if all the child nodes are constants so that they can be inserted into 
   // the parent node
   if (aggrNode)
   {
      TIntermSequence &childSequenceVector = aggrNode->getSequence() ;
      for (TIntermSequence::iterator p = childSequenceVector.begin(); 
          p != childSequenceVector.end(); p++)
      {
         if (!(*p)->getAsTyped()->getAsConstantUnion())
            return false;
      }
   }

   return allConstant;
}

// Returns false if this was a matrix that still needs a transpose
static bool TransposeMatrixConstructorArgs (const TType* type, TIntermSequence& args)
{
	if (!type->isMatrix())
		return true;
	if (args.size() != type->getObjectSize())
		return false;

	// HLSL vs. GLSL construct matrices in transposed order, so transpose the arguments for the constructor
	const int size = type->getNominalSize();
	for (int r = 0; r < size; ++r)
	{
		for (int c = r+1; c < size; ++c)
		{
			size_t idx1 = r*size+c;
			size_t idx2 = c*size+r;
			std::swap (args[idx1], args[idx2]);
		}
	}

	return true;
}

static void TransposeMatrixConstructorConstUnion (const TType* type, constUnion* args)
{
	if (!type->isMatrix())
		return;

	// HLSL vs. GLSL construct matrices in transposed order, so transpose the arguments for the constructor
	const int size = type->getNominalSize();
	for (int r = 0; r < size; ++r)
	{
		for (int c = r+1; c < size; ++c)
		{
			size_t idx1 = r*size+c;
			size_t idx2 = c*size+r;
			std::swap (args[idx1], args[idx2]);
		}
	}
}


// This function is used to test for the correctness of the parameters passed to various constructor functions
// and also convert them to the right datatype if it is allowed and required. 
//
// Returns 0 for an error or the constructed node (aggregate or typed) for no error.
//
TIntermTyped* TParseContext::addConstructor(TIntermNode* node, const TType* type, TOperator op, TFunction* fnCall, TSourceLoc line)
{
   if (node == 0)
      return 0;

   TIntermAggregate* aggrNode = node->getAsAggregate();

   TTypeList::iterator memberTypes;
   if (op == EOpConstructStruct)
      memberTypes = type->getStruct()->begin();

   TType elementType = *type;
   if (type->isArray())
      elementType.clearArrayness();

   bool singleArg;
   if (aggrNode)
   {
      if (aggrNode->getOp() != EOpNull || aggrNode->getSequence().size() == 1)
         singleArg = true;
      else
         singleArg = false;
   }
   else
      singleArg = true;

   if ( singleArg && op == EOpConstructStruct && node->getAsConstantUnion())
   {
      TIntermConstantUnion *cNode = node->getAsConstantUnion();

      if (cNode->getSize() == 1)
      {
         //create new aggrgate
         TIntermAggregate *tempAgg = 0;
         TTypeList::iterator sTypes = memberTypes;

         for ( ; sTypes != type->getStruct()->end(); sTypes++)
         {
            TOperator top = getConstructorOp( *(*sTypes).type);
            TIntermTyped *tNode = constructBuiltIn( (*sTypes).type, top, cNode, cNode->getLine(), false);
            if (tNode->getAsAggregate())
               tNode = foldConstConstructor(tNode->getAsAggregate(), *(*sTypes).type);
            tempAgg = intermediate.growAggregate( tempAgg, tNode, cNode->getLine());
         }

         if (tempAgg)
         {
            if (tempAgg->getSequence().size() ==1)
            {
               // If the sequence has only one element, use the first element in the sequence as the node
               node = *tempAgg->getSequence().begin();
               singleArg = true;
            }
            else
            {
               singleArg = false;
               aggrNode = tempAgg;
            }
         }

      }
   }

   TIntermTyped *newNode;
   if (singleArg)
   {
      // If structure constructor or array constructor is being called 
      // for only one parameter inside the structure, we need to call constructStruct function once.

      if (type->isArray())
         newNode = constructBuiltIn ( type, op, node, node->getLine(), true );
      else if (op == EOpConstructStruct)
         newNode = constructStruct(node, (*memberTypes).type, 1, node->getLine(), false);
      else
         newNode = constructBuiltIn(type, op, node, node->getLine(), false);

      if (newNode && newNode->getAsAggregate())
      {
         TIntermTyped* constConstructor = foldConstConstructor(newNode->getAsAggregate(), *type);
         if (constConstructor)
            return constConstructor;
      }

      return newNode;
   }

   if ( type->isArray() && (int)aggrNode->getSequence().size() > type->getArraySize())
      return constructArray( aggrNode, type, op, line);

   //
   // Handle list of arguments.
   //
   TIntermSequence &sequenceVector = aggrNode->getSequence() ;    // Stores the information about the parameter to the constructor
   // if the structure constructor contains more than one parameter, then construct
   // each parameter

   int paramCount = 0;  // keeps a track of the constructor parameter number being checked    

   // for each parameter to the constructor call, check to see if the right type is passed or convert them 
   // to the right type if possible (and allowed).
   // for structure constructors, just check if the right type is passed, no conversion is allowed.

   for (TIntermSequence::iterator p = sequenceVector.begin(); 
       p != sequenceVector.end(); p++, paramCount++)
   {
      if (type->isArray())
         newNode = constructBuiltIn ( &elementType, op, *p, node->getLine(), true );
      else if (op == EOpConstructStruct)
         newNode = constructStruct(*p, (memberTypes[paramCount]).type, paramCount+1, node->getLine(), true);
      else
         newNode = constructBuiltIn(type, op, *p, node->getLine(), true);

      if (newNode)
      {
         *p = newNode;
      }
   }


   TIntermTyped* constructor = intermediate.setAggregateOperator(aggrNode, op, line);
   TIntermTyped* constConstructor = foldConstConstructor(constructor->getAsAggregate(), *type);
   if (constConstructor)
      return constConstructor;

	if (!TransposeMatrixConstructorArgs (type, sequenceVector))
	{
		TIntermTyped* transpose = intermediate.addUnaryMath (EOpTranspose, constructor, line, symbolTable);
		return transpose;
	}

   return constructor;
}

TIntermTyped* TParseContext::foldConstConstructor(TIntermAggregate* aggrNode, const TType& type)
{
   bool canBeFolded = areAllChildConst(aggrNode);
   aggrNode->setType(type);
   if (canBeFolded)
   {
      bool returnVal = false;
      constUnion* unionArray = new constUnion[type.getObjectSize()];
      if (aggrNode->getSequence().size() == 1)
      {
         returnVal = intermediate.parseConstTree(aggrNode->getLine(), aggrNode, unionArray, aggrNode->getOp(), symbolTable,  type, true);
      }
      else
      {
         returnVal = intermediate.parseConstTree(aggrNode->getLine(), aggrNode, unionArray, aggrNode->getOp(), symbolTable,  type);
      }
      if (returnVal)
         return 0;

	  TransposeMatrixConstructorConstUnion (&type, unionArray);

      return intermediate.addConstantUnion(unionArray, type, aggrNode->getLine());
   }

   return 0;
}

// Function for determining which basic constructor op to use
TOperator TParseContext::getConstructorOp( const TType& type)
{
   TOperator op = EOpNull;

   switch (type.getBasicType())
   {
   case EbtFloat:
      if ( type.isMatrix())
      {
         switch (type.getNominalSize())
         {
         case 2: op = EOpConstructMat2; break;
         case 3: op = EOpConstructMat3; break;
         case 4: op = EOpConstructMat4; break;
         default: op = EOpNull; break;
         }
      }
      else
      {
         switch (type.getNominalSize())
         {
         case 1: op = EOpConstructFloat; break;
         case 2: op = EOpConstructVec2; break;
         case 3: op = EOpConstructVec3; break;
         case 4: op = EOpConstructVec4; break;
         default: op = EOpNull; break;
         }
      }
      break;
   case EbtInt:
      switch (type.getNominalSize())
      {
      case 1: op = EOpConstructInt; break;
      case 2: op = EOpConstructIVec2; break;
      case 3: op = EOpConstructIVec3; break;
      case 4: op = EOpConstructIVec4; break;
      default: op = EOpNull; break;
      }
      break;
   case EbtBool:
      switch (type.getNominalSize())
      {
      case 1: op = EOpConstructBool; break;
      case 2: op = EOpConstructBVec2; break;
      case 3: op = EOpConstructBVec3; break;
      case 4: op = EOpConstructBVec4; break;
      default: op = EOpNull; break;
      }
      break;
   case EbtStruct: op = EOpConstructStruct; break;
   default:
      op = EOpNull;
      break;
   }
   return op;
}

// This function promotes the function arguments contained in node to
// match the types from func.
TIntermNode* TParseContext::promoteFunctionArguments( TIntermNode *node, const TFunction* func)
{
   TIntermAggregate *aggNode = node->getAsAggregate();
   TIntermTyped *tNode = 0;
   TIntermNode *ret = 0;

   if (aggNode)
   {
      //This is a function call with multiple arguments
      TIntermSequence &seq = aggNode->getSequence();
      int paramNum = 0;

      assert( (int)seq.size() == func->getParamCount());

      for ( TIntermSequence::iterator it = seq.begin(); it != seq.end(); it++, paramNum++)
      {
         tNode = (*it)->getAsTyped();

         if ( tNode != 0 && tNode->getType() != *(*func)[paramNum].type)
         {
            TOperator op = getConstructorOp(*(*func)[paramNum].type);

            // Handle upward promotion of vectors:
            //   HLSL allows upward promotion of vectors as a special case to function calls.  For example,
            //   the call mul( mf4, vf3 ) will end up upward promoting the second argument from a float3
            //   to a float4 ( xyz, 0 ).  This code here generalizes this case, where in the case that
            //   an upward promotion of a vector is required, the necessary constants initializers are
            //   added to an aggregate.
            if ( tNode->getNominalSize() < (*func)[paramNum].type->getNominalSize() &&
                 tNode->isVector() && (*func)[paramNum].type->isVector() )
            {
               TIntermAggregate *tempAgg = 0;

               // Add the vector being uprward promoted
               tempAgg = intermediate.growAggregate ( tempAgg, tNode, node->getLine() );                   

               // Determine the number of trailing 0's required
               int nNumZerosToPad = (*func)[paramNum].type->getNominalSize() - tNode->getNominalSize();
               for ( int nPad = 0; nPad < nNumZerosToPad; nPad++ )
               {
                  // Create a new constant with value 0.0
                  constUnion *unionArray = new constUnion[1];
                  unionArray->setFConst(0.0f);

                  // Add the constant to the aggregrate node
                  TIntermConstantUnion *cUnion = intermediate.addConstantUnion(unionArray, TType(EbtFloat, EbpUndefined, EvqConst), tNode->getLine());                      
                  tempAgg = intermediate.growAggregate ( tempAgg, cUnion, tNode->getLine()); 
               }

               // Construct the built-in with padding
               tNode = constructBuiltIn ( (*func)[paramNum].type, op, tempAgg, node->getLine(), false);

            }
            else
            {
               tNode = constructBuiltIn( (*func)[paramNum].type, op, tNode, node->getLine(), false);
            }
         }

         if ( !tNode )
         {
            //function parameters need to have a type
            ret = 0;
            break;
         }

         ret = intermediate.growAggregate( ret, tNode, node->getLine());
      }
   }
   else
   {
      assert( func->getParamCount() == 1);
      TOperator op = getConstructorOp(*(*func)[0].type);
      ret = constructBuiltIn( (*func)[0].type, op, node, node->getLine(), false);
   }

   return ret;
}

TIntermTyped* TParseContext::addAssign(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc loc)
{
   TIntermTyped *tNode;


   if ( (tNode = intermediate.addAssign(op,left,right,loc)) == 0)
   {
      //need to convert
      TOperator cop = getConstructorOp( left->getType());
      TType type = left->getType();
      tNode = constructBuiltIn( &type, cop, right, loc, false);
      tNode = intermediate.addAssign( op, left, tNode, loc);
   }

   return tNode;
}

// Function for constructor implementation. Calls addUnaryMath with appropriate EOp value
// for the parameter to the constructor (passed to this function). Essentially, it converts
// the parameter types correctly. If a constructor expects an int (like ivec2) and is passed a 
// float, then float is converted to int.
//
// Returns 0 for an error or the constructed node.
//
TIntermTyped* TParseContext::constructBuiltIn(const TType* type, TOperator op, TIntermNode* node, TSourceLoc line, bool subset)
{
   TIntermTyped* newNode;
   TOperator basicOp;

   //
   // First, convert types as needed.
   //
   switch (op)
   {
   case EOpConstructVec2:
   case EOpConstructVec3:
   case EOpConstructVec4:
   case EOpConstructMat2:
   case EOpConstructMat3:
   case EOpConstructMat4:
   case EOpConstructFloat:
      basicOp = EOpConstructFloat;
      break;

   case EOpConstructIVec2:
   case EOpConstructIVec3:
   case EOpConstructIVec4:
   case EOpConstructInt:
      basicOp = EOpConstructInt;
      break;

   case EOpConstructBVec2:
   case EOpConstructBVec3:
   case EOpConstructBVec4:
   case EOpConstructBool:
      basicOp = EOpConstructBool;
      break;

   default:
      error(line, "unsupported construction", "", "");
      recover();

      return 0;
   }
   newNode = intermediate.addUnaryMath(basicOp, node, node->getLine(), symbolTable);
   if (newNode == 0)
   {
      error(line, "can't convert", "constructor", "");
      return 0;
   }

   //
   // Now, if there still isn't an operation to do the construction, and we need one, add one.
   //

   // Otherwise, skip out early.
   if (subset || newNode != node && newNode->getType() == *type)
      return newNode;

   //now perform HLSL style matrix conversions
   if ( newNode->getTypePointer()->isMatrix() && type->isMatrix())
   {
      switch (type->getNominalSize())
      {
      case 2:
         op = EOpConstructMat2FromMat;
         break;
      case 3:
         op = EOpConstructMat3FromMat;
         break;
      case 4:
         //there is no way to down convert to a mat4
         assert(0);
      }
   }

   // setAggregateOperator will insert a new node for the constructor, as needed.
   newNode = intermediate.setAggregateOperator(newNode, op, line);
   newNode->setType(*type);
   return newNode;
}

// This function tests for the type of the parameters to the structures constructors. Raises
// an error message if the expected type does not match the parameter passed to the constructor.
//
// Returns 0 for an error or the input node itself if the expected and the given parameter types match.
//
TIntermTyped* TParseContext::constructStruct(TIntermNode* node, TType* type, int paramCount, TSourceLoc line, bool subset)
{
   if (*type == node->getAsTyped()->getType())
   {
      if (subset)
         return node->getAsTyped();
      else
         return intermediate.setAggregateOperator(node->getAsTyped(), EOpConstructStruct, line);
   }
   else
   {
      error(line, "", "constructor", "cannot convert parameter %d from '%s' to '%s'", paramCount,
            node->getAsTyped()->getType().getBasicString(), type->getBasicString());
      recover();
   }

   return 0;
}

//
// This function tries to construct an array from an aggregate
//
TIntermTyped* TParseContext::constructArray(TIntermAggregate* aggNode, const TType* type, TOperator op, TSourceLoc line)
{
   TType elementType = *type;
   int elementCount = type->getArraySize();
   TIntermAggregate *newAgg = 0;

   elementType.clearArrayness();


   TIntermSequence &seq = aggNode->getSequence();
   TIntermSequence::iterator sit = seq.begin();

   // Count the total size of the initializer sequence and make sure it matches the array size
   int nInitializerSize = 0;
   while ( sit != seq.end() )
   {
      nInitializerSize += (*sit)->getAsTyped()->getNominalSize();
      sit++;
   }

   if ( nInitializerSize != elementType.getObjectSize() * elementCount)
   {
      error(line, "", "constructor", "improper number of arguments to array constructor, expected %d got %d",
            elementType.getObjectSize() *elementCount, nInitializerSize);
      recover();
      return 0;
   }

   sit = seq.begin();

   // Loop over each of the elements in the initializer sequence and add to the constructors
   for (int ii = 0; ii < elementCount; ii++)
   {
      TIntermAggregate *tempAgg = 0;

      // Initialize this element of the array
      int nInitSize = 0;
      while ( nInitSize < elementType.getObjectSize() )
      {
         tempAgg = intermediate.growAggregate( tempAgg, *sit, line);
         nInitSize += (*sit)->getAsTyped()->getNominalSize();
         sit++;
      }

      // Check to make sure that the initializer does not span array size boundaries.  This is allowed in
      // HLSL, although currently not supported by the translator.  It could be done, it will just make
      // the array code generation much more complicated, because it will have to potentially break up
      // elements that span array boundaries.
      if ( nInitSize != elementType.getObjectSize() )
      {
         error ( line, "", "constructor", "can not handle initializers that span array element boundaries");
         recover();
         return 0;           
      }
      newAgg = intermediate.growAggregate( newAgg, addConstructor( tempAgg, &elementType, op, 0, line), line);
   }

   return addConstructor( newAgg, type, op, 0, line);

}

//
// This function returns the tree representation for the vector field(s) being accessed from contant vector.
// If only one component of vector is accessed (v.x or v[0] where v is a contant vector), then a contant node is
// returned, else an aggregate node is returned (for v.xy). The input to this function could either be the symbol
// node or it could be the intermediate tree representation of accessing fields in a constant structure or column of 
// a constant matrix.
//
TIntermTyped* TParseContext::addConstVectorNode(TVectorFields& fields, TIntermTyped* node, TSourceLoc line)
{
   TIntermTyped* typedNode;
   TIntermConstantUnion* tempConstantNode = node->getAsConstantUnion();

   constUnion *unionArray;
   if (tempConstantNode)
   {
      unionArray = tempConstantNode->getUnionArrayPointer();

      if (!unionArray)
      {  // this error message should never be raised
         infoSink.info.message(EPrefixInternalError, "constUnion not initialized in addConstVectorNode function", line);
         recover();

         return node;
      }
   }
   else
   { // The node has to be either a symbol node or an aggregate node or a tempConstant node, else, its an error
      error(line, "Cannot offset into the vector", "Error", "");
      recover();

      return 0;
   }

   constUnion* constArray = new constUnion[fields.num];

   for (int i = 0; i < fields.num; i++)
   {
      if (fields.offsets[i] >= node->getType().getObjectSize())
      {
         error(line, "", "[", "vector field selection out of range '%d'", fields.offsets[i]);
         recover();
         fields.offsets[i] = 0;
      }

      constArray[i] = unionArray[fields.offsets[i]];

   } 
   typedNode = intermediate.addConstantUnion(constArray, node->getType(), line);
   return typedNode;
}


// This function returns the row being accessed from a constant matrix. The values are retrieved from
// the symbol table and parse-tree is built for a vector (each row of a matrix is a vector). The input 
// to the function could either be a symbol node (m[0] where m is a constant matrix)that represents a 
// constant matrix or it could be the tree representation of the constant matrix (s.m1[0] where s is a constant structure)
TIntermTyped* TParseContext::addConstMatrixNode(int index, TIntermTyped* node, TSourceLoc line)
{
   TIntermTyped* typedNode;
   TIntermConstantUnion* tempConstantNode = node->getAsConstantUnion();

   if (index >= node->getType().getNominalSize())
   {
      error(line, "", "[", "matrix field selection out of range '%d'", index);
      recover();
      index = 0;
   }

   if (tempConstantNode)
   {
      constUnion* unionArray = tempConstantNode->getUnionArrayPointer();
      int size = tempConstantNode->getType().getNominalSize();
	  constUnion* res = new constUnion[size];
	  for (int i = 0; i < size; ++i)
		  res[i] = unionArray[size*i+index];

      typedNode = intermediate.addConstantUnion(res, tempConstantNode->getType(), line);
   }
   else
   {
      error(line, "Cannot offset into the matrix", "Error", "");
      recover();

      return 0;
   }

   return typedNode;
}


//
// This function returns an element of an array accessed from a constant array. The values are retrieved from
// the symbol table and parse-tree is built for the type of the element. The input 
// to the function could either be a symbol node (a[0] where a is a constant array)that represents a 
// constant array or it could be the tree representation of the constant array (s.a1[0] where s is a constant structure)
//
TIntermTyped* TParseContext::addConstArrayNode(int index, TIntermTyped* node, TSourceLoc line)
{
   TIntermTyped* typedNode;
   TIntermConstantUnion* tempConstantNode = node->getAsConstantUnion();
   TType arrayElementType = node->getType();
   arrayElementType.clearArrayness();

   if (index >= node->getType().getArraySize())
   {
      error(line, "", "[", "array field selection out of range '%d'", index);
      recover();
      index = 0;
   }

   int arrayElementSize = arrayElementType.getObjectSize();

   if (tempConstantNode)
   {
      constUnion* unionArray = tempConstantNode->getUnionArrayPointer();
      typedNode = intermediate.addConstantUnion(&unionArray[arrayElementSize * index], tempConstantNode->getType(), line);
   }
   else
   {
      error(line, "Cannot offset into the array", "Error", "");
      recover();

      return 0;
   }

   return typedNode;
}


//
// This function returns the value of a particular field inside a constant structure from the symbol table. 
// If there is an embedded/nested struct, it appropriately calls addConstStructNested or addConstStructFromAggr
// function and returns the parse-tree with the values of the embedded/nested struct.
//
TIntermTyped* TParseContext::addConstStruct(TString& identifier, TIntermTyped* node, TSourceLoc line)
{
   TTypeList* fields = node->getType().getStruct();
   TIntermTyped *typedNode;
   int instanceSize = 0;
   unsigned int index = 0;
   TIntermConstantUnion *tempConstantNode = node->getAsConstantUnion();

   for ( index = 0; index < fields->size(); ++index)
   {
      if ((*fields)[index].type->getFieldName() == identifier)
      {
         break;
      }
      else
      {
         instanceSize += (*fields)[index].type->getObjectSize();
      }
   }

   if (tempConstantNode)
   {
      constUnion* constArray = tempConstantNode->getUnionArrayPointer();

      typedNode = intermediate.addConstantUnion(constArray+instanceSize, tempConstantNode->getType(), line); // type will be changed in the calling function
   }
   else
   {
      error(line, "Cannot offset into the structure", "Error", "");
      recover();

      return 0;
   }

   return typedNode;
}

//
// This function takes two aggragates, and merges them into a single one
// It does not nest the right inside the left, as growAggregate would
TIntermAggregate* TParseContext::mergeAggregates( TIntermAggregate *left, TIntermAggregate *right)
{
   TIntermAggregate *node = left;
   if (!left)
      return right;

   if (right)
   {
      TIntermSequence &seq = right->getSequence();

      for ( TIntermSequence::iterator it = seq.begin(); it != seq.end(); it++)
      {
         node = intermediate.growAggregate( node, *it, right->getLine());
      }
   }

   return node;
}

OS_TLSIndex GlobalParseContextIndex = OS_INVALID_TLS_INDEX;

bool InitializeParseContextIndex()
{
   if (GlobalParseContextIndex != OS_INVALID_TLS_INDEX)
   {
      assert(0 && "InitializeParseContextIndex(): Parse Context already initalised");
      return false;
   }

   //
   // Allocate a TLS index.
   //
   GlobalParseContextIndex = OS_AllocTLSIndex();

   if (GlobalParseContextIndex == OS_INVALID_TLS_INDEX)
   {
      assert(0 && "InitializeParseContextIndex(): Parse Context already initalised");
      return false;
   }

   return true;
}

bool InitializeGlobalParseContext()
{
   if (GlobalParseContextIndex == OS_INVALID_TLS_INDEX)
   {
      assert(0 && "InitializeGlobalParseContext(): Parse Context index not initalised");
      return false;
   }

   TThreadParseContext *lpParseContext = static_cast<TThreadParseContext *>(OS_GetTLSValue(GlobalParseContextIndex));
   if (lpParseContext != 0)
   {
      assert(0 && "InitializeParseContextIndex(): Parse Context already initalised");
      return false;
   }

   TThreadParseContext *lpThreadData = new TThreadParseContext();
   if (lpThreadData == 0)
   {
      assert(0 && "InitializeGlobalParseContext(): Unable to create thread parse context");
      return false;
   }

   lpThreadData->lpGlobalParseContext = 0;
   OS_SetTLSValue(GlobalParseContextIndex, lpThreadData);

   return true;
}

TParseContextPointer& GetGlobalParseContext()
{
   //
   // Minimal error checking for speed
   //

   TThreadParseContext *lpParseContext = static_cast<TThreadParseContext *>(OS_GetTLSValue(GlobalParseContextIndex));

   return lpParseContext->lpGlobalParseContext;
}

bool FreeParseContext()
{
   if (GlobalParseContextIndex == OS_INVALID_TLS_INDEX)
   {
      assert(0 && "FreeParseContext(): Parse Context index not initalised");
      return false;
   }

   TThreadParseContext *lpParseContext = static_cast<TThreadParseContext *>(OS_GetTLSValue(GlobalParseContextIndex));
   if (lpParseContext)
      delete lpParseContext;

   return true;
}

bool FreeParseContextIndex()
{
   OS_TLSIndex tlsiIndex = GlobalParseContextIndex;

   if (GlobalParseContextIndex == OS_INVALID_TLS_INDEX)
   {
      assert(0 && "FreeParseContextIndex(): Parse Context index not initalised");
      return false;
   }

   GlobalParseContextIndex = OS_INVALID_TLS_INDEX;

   return OS_FreeTLSIndex(tlsiIndex);
}

