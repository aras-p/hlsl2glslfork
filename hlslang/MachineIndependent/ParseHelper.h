// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef _PARSER_HELPER_INCLUDED_
#define _PARSER_HELPER_INCLUDED_

#include "SymbolTable.h"
#include "localintermediate.h"


//
// The following are extra variables needed during parsing, grouped together so
// they can be passed to the parser without needing a global.
//
struct TParseContext
{
   TParseContext(TSymbolTable& symt, TIntermediate& interm, EShLanguage L, TInfoSink& is) : 
   intermediate(interm), symbolTable(symt), infoSink(is), language(L), treeRoot(0),
   recoveredFromError(false), numErrors(0), lexAfterType(false), loopNestingLevel(0), 
   inTypeParen(false)
   {
   }
   TIntermediate& intermediate; // to hold and build a parse tree
   TSymbolTable& symbolTable;   // symbol table that goes with the language currently being parsed
   TInfoSink& infoSink;
   EShLanguage language;        // vertex or fragment language (future: pack or unpack)
   TIntermNode* treeRoot;       // root of parse tree being created
   bool recoveredFromError;     // true if a parse error has occurred, but we continue to parse
   int numErrors;
   bool lexAfterType;           // true if we've recognized a type, so can only be looking for an identifier
   int loopNestingLevel;        // 0 if outside all loops
   bool inTypeParen;            // true if in parentheses, looking only for an identifier
   const TType* currentFunctionType;  // the return type of the function that's currently being parsed
   bool functionReturnsValue;   // true if a non-void function has a return

   void C_DECL error(TSourceLoc, const char *szReason, const char *szToken, 
                     const char *szExtraInfoFormat, ...);
   bool reservedErrorCheck(const TSourceLoc& line, const TString& identifier);
   void recover();

   TQualifier getDefaultQualifier() const { return symbolTable.atGlobalLevel() ? EvqGlobal : EvqTemporary; }

   bool parseVectorFields(const TString&, int vecSize, TVectorFields&, const TSourceLoc& line);
   bool parseMatrixFields(const TString&, int matSize, TVectorFields&, const TSourceLoc& line);
   void assignError(const TSourceLoc& line, const char* op, TString left, TString right);
   void unaryOpError(const TSourceLoc& line, char* op, TString operand);
   void binaryOpError(const TSourceLoc& line, char* op, TString left, TString right);
   bool lValueErrorCheck(const TSourceLoc& line, char* op, TIntermTyped*);
   bool constErrorCheck(TIntermTyped* node);
   bool integerErrorCheck(TIntermTyped* node, const char* token);
   bool globalErrorCheck(const TSourceLoc& line, bool global, char* token);
   bool constructorErrorCheck(const TSourceLoc& line, TIntermNode*, TFunction&, TOperator, TType*);
   bool arraySizeErrorCheck(const TSourceLoc& line, TIntermTyped* expr, int& size);
   bool arrayQualifierErrorCheck(const TSourceLoc& line, TPublicType type);
   bool arrayTypeErrorCheck(const TSourceLoc& line, TPublicType type);
   bool arrayErrorCheck(const TSourceLoc& line, TString& identifier, TPublicType type, TVariable*& variable);
   bool arrayErrorCheck(const TSourceLoc& line, TString& identifier, const TTypeInfo *info, TPublicType type, TVariable*& variable);
   bool insertBuiltInArrayAtGlobalLevel();
   bool voidErrorCheck(const TSourceLoc& line, const TString&, const TPublicType&);
   bool boolErrorCheck(const TSourceLoc& line, const TIntermTyped*);
   bool boolErrorCheck(const TSourceLoc& line, const TPublicType&);
   bool boolOrVectorErrorCheck(const TSourceLoc& line, const TIntermTyped* type);
   bool samplerErrorCheck(const TSourceLoc& line, const TPublicType& pType, const char* reason);
   bool structQualifierErrorCheck(const TSourceLoc& line, const TPublicType& pType);
   bool parameterSamplerErrorCheck(const TSourceLoc& line, TQualifier qualifier, const TType& type);
   bool containsSampler(TType& type);
   bool nonInitConstErrorCheck(const TSourceLoc& line, TString& identifier, TPublicType& type);
   bool nonInitErrorCheck(const TSourceLoc& line, TString& identifier, const TTypeInfo *info, TPublicType& type);
   bool nonInitErrorCheck(const TSourceLoc& line, TString& identifier, TPublicType& type);
   bool paramErrorCheck(const TSourceLoc& line, TQualifier qualifier, TQualifier paramQualifier, TType* type);
   const TFunction* findFunction(const TSourceLoc& line, TFunction* pfnCall, bool *builtIn = 0);
   bool executeInitializer(TSourceLoc line, TString& identifier, const TTypeInfo *info, TPublicType& pType, 
                           TIntermTyped* initializer, TIntermNode*& intermNode, TVariable* variable = 0);
   bool executeInitializer(TSourceLoc line, TString& identifier, TPublicType& pType, 
                           TIntermTyped* initializer, TIntermNode*& intermNode, TVariable* variable = 0);
   bool areAllChildConst(TIntermAggregate* aggrNode);
   TIntermTyped* addConstructor(TIntermNode*, const TType*, TOperator, TFunction*, TSourceLoc);
   TIntermTyped* foldConstConstructor(TIntermAggregate* aggrNode, const TType& type);
   TIntermTyped* constructArray(TIntermAggregate*, const TType*, TOperator, TSourceLoc);
   TIntermTyped* constructStruct(TIntermNode*, TType*, int, TSourceLoc, bool subset);
   TIntermTyped* constructBuiltIn(const TType*, TOperator, TIntermNode*, TSourceLoc, bool subset);
   TIntermTyped* addConstVectorNode(TVectorFields&, TIntermTyped*, TSourceLoc);
   TIntermTyped* addConstMatrixNode(int , TIntermTyped*, TSourceLoc);
   TIntermTyped* addConstArrayNode(int index, TIntermTyped* node, TSourceLoc line);
   TIntermTyped* addConstStruct(TString& , TIntermTyped*, TSourceLoc);
   TIntermTyped* addAssign(TOperator op, TIntermTyped* left, TIntermTyped* right, TSourceLoc);
   TIntermAggregate* mergeAggregates( TIntermAggregate *left, TIntermAggregate *right);
   bool arraySetMaxSize(TIntermSymbol*, TType*, int, bool, TSourceLoc);
   TOperator getConstructorOp( const TType&);
   TIntermNode* promoteFunctionArguments( TIntermNode *node, const TFunction* func);
   TString HashErrMsg; 
   bool AfterEOF;
};

int PaParseString(char* source, TParseContext&);
void PaReservedWord();
int PaIdentOrType(TString& id, TParseContext&, TSymbol*&);
int PaParseComment(TSourceLoc &lineno, TParseContext&);
void setInitialState();

typedef TParseContext* TParseContextPointer;
extern TParseContextPointer& GetGlobalParseContext();
#define GlobalParseContext GetGlobalParseContext()

typedef struct TThreadParseContextRec
{
   TParseContext *lpGlobalParseContext;
} TThreadParseContext;

#endif // _PARSER_HELPER_INCLUDED_

