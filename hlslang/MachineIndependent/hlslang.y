// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

/**
 * This is bison grammar and production code for parsing the OpenGL 2.0 shading
 * languages.
 */
%{

/* Based on:
ANSI C Yacc grammar

In 1985, Jeff Lee published his Yacc grammar (which is accompanied by a 
matching Lex specification) for the April 30, 1985 draft version of the 
ANSI C standard.  Tom Stockfisch reposted it to net.sources in 1987; that
original, as mentioned in the answer to question 17.25 of the comp.lang.c
FAQ, can be ftp'ed from ftp.uu.net, file usenet/net.sources/ansi.c.grammar.Z.
 
I intend to keep this version as close to the current C Standard grammar as 
possible; please let me know if you discover discrepancies. 

Jutta Degener, 1995 
*/

#include "SymbolTable.h"
#include "ParseHelper.h"
#include "../../include/hlsl2glsl.h"

#ifdef _WIN32
    #define YYPARSE_PARAM parseContext
    #define YYPARSE_PARAM_DECL TParseContext&
    #define YY_DECL int yylex(YYSTYPE* pyylval, TParseContext& parseContext)
    #define YYLEX_PARAM parseContext
#else
    #define YYPARSE_PARAM parseContextLocal
    #define parseContext (*((TParseContext*)(parseContextLocal)))
    #define YY_DECL int yylex(YYSTYPE* pyylval, void* parseContextLocal)
    #define YYLEX_PARAM (void*)(parseContextLocal)
    extern void yyerror(char*);    
#endif

#define FRAG_VERT_ONLY(S, L) {                                                  \
    if (parseContext.language != EShLangFragment &&                             \
        parseContext.language != EShLangVertex) {                               \
        parseContext.error(L, " supported in vertex/fragment shaders only ", S, "", "");   \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define VERTEX_ONLY(S, L) {                                                     \
    if (parseContext.language != EShLangVertex) {                               \
        parseContext.error(L, " supported in vertex shaders only ", S, "", "");            \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define FRAG_ONLY(S, L) {                                                       \
    if (parseContext.language != EShLangFragment) {                             \
        parseContext.error(L, " supported in fragment shaders only ", S, "", "");          \
        parseContext.recover();                                                            \
    }                                                                           \
}

#define UNSUPPORTED_FEATURE(S, L) {                                                       \
    parseContext.error(L, " not supported ", S, "", "");              \
    parseContext.recover();                                                            \
}

#define SET_BASIC_TYPE(RES,PAR,T,PREC) \
	TQualifier qual = parseContext.getDefaultQualifier(); \
	(RES).setBasic(T, qual, (PAR).line); \
	(RES).precision = PREC


%}
%union {
    struct {
        TSourceLoc line;
        union {
            TString *string;
            float f;
            int i;
            bool b;
        };
        TSymbol* symbol;
    } lex;
    struct {
        TSourceLoc line;
        TOperator op;
        union {
            TIntermNode* intermNode;
            TIntermNodePair nodePair;
            TIntermTyped* intermTypedNode;
            TIntermAggregate* intermAggregate;
        };
        union {
            TPublicType type;
            TQualifier qualifier;
            TFunction* function;
            TParameter param;
            TTypeLine typeLine;
            TTypeList* typeList;
	    TAnnotation* ann;
	    TTypeInfo* typeInfo;
        };
    } interm;
}

%{
#ifndef _WIN32
    extern int yylex(YYSTYPE*, void*);
#endif
%}

%pure_parser /* Just in case is called from multiple threads */
%expect 1 /* One shift reduce conflict because of if | else */
%token <lex> ATTRIBUTE CONST_QUAL STATIC_QUAL BOOL_TYPE FLOAT_TYPE INT_TYPE STRING_TYPE FIXED_TYPE HALF_TYPE
%token <lex> BREAK CONTINUE DO ELSE FOR IF DISCARD RETURN
%token <lex> BVEC2 BVEC3 BVEC4 IVEC2 IVEC3 IVEC4 VEC2 VEC3 VEC4 HVEC2 HVEC3 HVEC4 FVEC2 FVEC3 FVEC4
%token <lex> MATRIX2 MATRIX3 MATRIX4 HMATRIX2 HMATRIX3 HMATRIX4 FMATRIX2 FMATRIX3 FMATRIX4
%token <lex> IN_QUAL OUT_QUAL INOUT_QUAL UNIFORM VARYING
%token <lex> STRUCT VOID_TYPE WHILE
%token <lex> SAMPLER1D SAMPLER2D SAMPLER3D SAMPLERCUBE SAMPLER1DSHADOW SAMPLER2DSHADOW SAMPLERRECT
%token <lex> SAMPLERGENERIC VECTOR MATRIX REGISTER TEXTURE SAMPLERSTATE

%token <lex> IDENTIFIER TYPE_NAME FLOATCONSTANT INTCONSTANT BOOLCONSTANT STRINGCONSTANT
%token <lex> FIELD_SELECTION
%token <lex> LEFT_OP RIGHT_OP
%token <lex> INC_OP DEC_OP LE_OP GE_OP EQ_OP NE_OP
%token <lex> AND_OP OR_OP XOR_OP MUL_ASSIGN DIV_ASSIGN ADD_ASSIGN
%token <lex> MOD_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%token <lex> SUB_ASSIGN

%type <lex>  ann_numerical_constant

%token <lex> LEFT_PAREN RIGHT_PAREN LEFT_BRACKET RIGHT_BRACKET LEFT_BRACE RIGHT_BRACE DOT
%token <lex> COMMA COLON EQUAL SEMICOLON BANG DASH TILDE PLUS STAR SLASH PERCENT
%token <lex> LEFT_ANGLE RIGHT_ANGLE VERTICAL_BAR CARET AMPERSAND QUESTION

%type <interm> assignment_operator unary_operator
%type <interm.intermTypedNode> variable_identifier primary_expression postfix_expression
%type <interm.intermTypedNode> expression integer_expression assignment_expression
%type <interm.intermTypedNode> unary_expression multiplicative_expression additive_expression
%type <interm.intermTypedNode> relational_expression equality_expression 
%type <interm.intermTypedNode> conditional_expression constant_expression
%type <interm.intermTypedNode> logical_or_expression logical_xor_expression logical_and_expression
%type <interm.intermTypedNode> shift_expression and_expression exclusive_or_expression inclusive_or_expression
%type <interm.intermTypedNode> function_call initializer condition conditionopt

%type <interm.intermTypedNode> initialization_list sampler_initializer
%type <interm.intermAggregate> initializer_list

%type <interm.intermNode> translation_unit function_definition
%type <interm.intermNode> statement simple_statement
%type <interm.intermAggregate>  statement_list compound_statement 
%type <interm.intermNode> declaration_statement selection_statement expression_statement
%type <interm.intermNode> declaration external_declaration
%type <interm.intermNode> for_init_statement compound_statement_no_new_scope
%type <interm.nodePair> selection_rest_statement for_rest_statement
%type <interm.intermNode> iteration_statement jump_statement statement_no_new_scope
%type <interm> single_declaration init_declarator_list

%type <interm> parameter_declaration parameter_declarator parameter_type_specifier
%type <interm.qualifier> parameter_qualifier

%type <interm.type> type_qualifier fully_specified_type type_specifier 
%type <interm.type> type_specifier_nonarray
%type <interm.type> struct_specifier 
%type <interm.typeLine> struct_declarator 
%type <interm.typeList> struct_declarator_list struct_declaration struct_declaration_list
%type <interm.function> function_header function_declarator function_identifier
%type <interm.function> function_header_with_parameters function_call_header 
%type <interm> function_call_header_with_parameters function_call_header_no_parameters function_call_generic function_prototype
%type <interm> function_call_or_method

%type <interm.ann> annotation annotation_list
%type <interm.typeInfo> type_info
%type <lex> annotation_item semantic

%start translation_unit 
%%

variable_identifier 
    : IDENTIFIER {
        // The symbol table search was done in the lexical phase
        const TSymbol* symbol = $1.symbol;
        const TVariable* variable;
        if (symbol == 0) {
            parseContext.error($1.line, "undeclared identifier", $1.string->c_str(), "");
            parseContext.recover();
            TType type(EbtFloat, EbpUndefined);
            TVariable* fakeVariable = new TVariable($1.string, type);
            parseContext.symbolTable.insert(*fakeVariable);
            variable = fakeVariable;
        } else {
            // This identifier can only be a variable type symbol 
            if (! symbol->isVariable()) {
                parseContext.error($1.line, "variable expected", $1.string->c_str(), "");
                parseContext.recover();
            }
            variable = static_cast<const TVariable*>(symbol);
        }

        // don't delete $1.string, it's used by error recovery, and the pool
        // pop will reclaim the memory

        if (variable->getType().getQualifier() == EvqConst ) {
            constUnion* constArray = variable->getConstPointer();
            TType t(variable->getType());
            $$ = parseContext.intermediate.addConstantUnion(constArray, t, $1.line);        
        } else
            $$ = parseContext.intermediate.addSymbol(variable->getUniqueId(), 
                                                     variable->getName(),
                                                     variable->getInfo(), 
                                                     variable->getType(), $1.line);
    }
    ;

primary_expression
    : variable_identifier {
        $$ = $1;
    }
    | INTCONSTANT {        
        constUnion *unionArray = new constUnion[1];
        unionArray->setIConst($1.i);
        $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EbpUndefined, EvqConst), $1.line);
    }
    | FLOATCONSTANT {
        constUnion *unionArray = new constUnion[1];
        unionArray->setFConst($1.f);
        $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtFloat, EbpUndefined, EvqConst), $1.line);
    }
    | BOOLCONSTANT {
        constUnion *unionArray = new constUnion[1];
        unionArray->setBConst($1.b);
        $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $1.line);
    }
    | LEFT_PAREN expression RIGHT_PAREN {
        $$ = $2;
    }
    ;

postfix_expression
    : primary_expression { 
        $$ = $1;
    } 
    | postfix_expression LEFT_BRACKET integer_expression RIGHT_BRACKET {
        if (!$1) {
            parseContext.error($2.line, " left of '[' is null ", "expression", "");
            YYERROR;
        }
        if (!$1->isArray() && !$1->isMatrix() && !$1->isVector()) {
            if ($1->getAsSymbolNode())
                parseContext.error($2.line, " left of '[' is not of type array, matrix, or vector ", $1->getAsSymbolNode()->getSymbol().c_str(), "");
            else
                parseContext.error($2.line, " left of '[' is not of type array, matrix, or vector ", "expression", "");
            parseContext.recover();
        }
        if ($1->getType().getQualifier() == EvqConst && $3->getQualifier() == EvqConst) {
            if ($1->isArray()) { // constant folding for arrays
                $$ = parseContext.addConstArrayNode($3->getAsConstantUnion()->getUnionArrayPointer()->getIConst(), $1, $2.line);
            } else if ($1->isVector()) {  // constant folding for vectors
                TVectorFields fields;                
                fields.num = 1;
                fields.offsets[0] = $3->getAsConstantUnion()->getUnionArrayPointer()->getIConst(); // need to do it this way because v.xy sends fields integer array
                $$ = parseContext.addConstVectorNode(fields, $1, $2.line);
            } else if ($1->isMatrix()) { // constant folding for matrices
                $$ = parseContext.addConstMatrixNode($3->getAsConstantUnion()->getUnionArrayPointer()->getIConst(), $1, $2.line);
            } 
        } else {
            if ($3->getQualifier() == EvqConst) {
                if (($1->isVector() || $1->isMatrix()) && $1->getType().getNominalSize() <= $3->getAsConstantUnion()->getUnionArrayPointer()->getIConst() && !$1->isArray() ) {
                    parseContext.error($2.line, "", "[", "field selection out of range '%d'", $3->getAsConstantUnion()->getUnionArrayPointer()->getIConst());
                    parseContext.recover();
                } else {
                    if ($1->isArray()) {
                        if ($1->getType().getArraySize() == 0) {
                            if ($1->getType().getMaxArraySize() <= $3->getAsConstantUnion()->getUnionArrayPointer()->getIConst()) {
                                if (parseContext.arraySetMaxSize($1->getAsSymbolNode(), $1->getTypePointer(), $3->getAsConstantUnion()->getUnionArrayPointer()->getIConst(), true, $2.line))
                                    parseContext.recover(); 
                            } else {
                                if (parseContext.arraySetMaxSize($1->getAsSymbolNode(), $1->getTypePointer(), 0, false, $2.line))
                                    parseContext.recover(); 
                            }
                        } else if ( $3->getAsConstantUnion()->getUnionArrayPointer()->getIConst() >= $1->getType().getArraySize()) {
                            parseContext.error($2.line, "", "[", "array index out of range '%d'", $3->getAsConstantUnion()->getUnionArrayPointer()->getIConst());
                            parseContext.recover();
                        }
                    }
                    $$ = parseContext.intermediate.addIndex(EOpIndexDirect, $1, $3, $2.line);
                }
            } else {
                if ($1->isArray() && $1->getType().getArraySize() == 0) {
                    parseContext.error($2.line, "", "[", "array must be redeclared with a size before being indexed with a variable");
                    parseContext.recover();
                }
                
                $$ = parseContext.intermediate.addIndex(EOpIndexIndirect, $1, $3, $2.line);
            }
        } 
        if ($$ == 0) {
            constUnion *unionArray = new constUnion[1];
            unionArray->setFConst(0.0f);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtFloat, EbpUndefined, EvqConst), $2.line);
        } else if ($1->isArray()) {
            if ($1->getType().getStruct())
                $$->setType(TType($1->getType().getStruct(), $1->getType().getTypeName(), EbpUndefined, $1->getLine()));
            else
                $$->setType(TType($1->getBasicType(), $1->getPrecision(), EvqTemporary, $1->getNominalSize(), $1->isMatrix()));
                
            if ($1->getType().getQualifier() == EvqConst)
                $$->getTypePointer()->changeQualifier(EvqConst);
        } else if ($1->isMatrix() && $1->getType().getQualifier() == EvqConst)         
            $$->setType(TType($1->getBasicType(), $1->getPrecision(), EvqConst, $1->getNominalSize()));     
        else if ($1->isMatrix())            
            $$->setType(TType($1->getBasicType(), $1->getPrecision(), EvqTemporary, $1->getNominalSize()));     
        else if ($1->isVector() && $1->getType().getQualifier() == EvqConst)          
            $$->setType(TType($1->getBasicType(), $1->getPrecision(), EvqConst));
        else if ($1->isVector())       
            $$->setType(TType($1->getBasicType(), $1->getPrecision(), EvqTemporary));
        else
            $$->setType($1->getType()); 
    }
    | function_call {
        $$ = $1;
    }
    | postfix_expression DOT FIELD_SELECTION {      
		if (!$1) {
            parseContext.error($3.line, "field selection on null object", ".", "");
            YYERROR;
		}  
        if ($1->isArray()) {
            parseContext.error($3.line, "cannot apply dot operator to an array", ".", "");
            parseContext.recover();
        }

        if ($1->isVector()) {
            TVectorFields fields;
            if (! parseContext.parseVectorFields(*$3.string, $1->getNominalSize(), fields, $3.line)) {
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
            }

            if ($1->getType().getQualifier() == EvqConst) { // constant folding for vector fields
                $$ = parseContext.addConstVectorNode(fields, $1, $3.line);
                if ($$ == 0) {
                    parseContext.recover();
                    $$ = $1;
                }
                else
                    $$->setType(TType($1->getBasicType(), $1->getPrecision(), EvqConst, (int) (*$3.string).size()));
            } else {
                if (fields.num == 1) {
                    constUnion *unionArray = new constUnion[1];
                    unionArray->setIConst(fields.offsets[0]);
                    TIntermTyped* index = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EbpUndefined, EvqConst), $3.line);
                    $$ = parseContext.intermediate.addIndex(EOpIndexDirect, $1, index, $2.line);
                    $$->setType(TType($1->getBasicType(), $1->getPrecision()));
                } else {
                    TString vectorString = *$3.string;
                    TIntermTyped* index = parseContext.intermediate.addSwizzle(fields, $3.line);                
                    $$ = parseContext.intermediate.addIndex(EOpVectorSwizzle, $1, index, $2.line);
                    $$->setType(TType($1->getBasicType(), $1->getPrecision(), EvqTemporary, (int) vectorString.size()));  
                }
            }
        } else if ($1->isMatrix()) {
            TVectorFields fields;
            if (! parseContext.parseMatrixFields(*$3.string, $1->getNominalSize(), fields, $3.line)) {
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
            }

            TString vectorString = *$3.string;
            TIntermTyped* index = parseContext.intermediate.addSwizzle(fields, $3.line);                
            $$ = parseContext.intermediate.addIndex(EOpMatrixSwizzle, $1, index, $2.line);
            $$->setType(TType($1->getBasicType(), $1->getPrecision(), EvqTemporary, fields.num));
                    
        } else if ($1->getBasicType() == EbtStruct) {
            bool fieldFound = false;
            TTypeList* fields = $1->getType().getStruct();
            if (fields == 0) {
                parseContext.error($2.line, "structure has no fields", "Internal Error", "");
                parseContext.recover();
                $$ = $1;
            } else {
                unsigned int i;
                for (i = 0; i < fields->size(); ++i) {
                    if ((*fields)[i].type->getFieldName() == *$3.string) {
                        fieldFound = true;
                        break;
                    }                
                }
                if (fieldFound) {
                    if ($1->getType().getQualifier() == EvqConst) {
                        $$ = parseContext.addConstStruct(*$3.string, $1, $2.line);
                        if ($$ == 0) {
                            parseContext.recover();
                            $$ = $1;
                        }
                        else {
                            $$->setType(*(*fields)[i].type);
                            // change the qualifier of the return type, not of the structure field
                            // as the structure definition is shared between various structures.
                            $$->getTypePointer()->changeQualifier(EvqConst);
                        }
                    } else {
                        constUnion *unionArray = new constUnion[1];
                        unionArray->setIConst(i);
                        TIntermTyped* index = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EbpUndefined, EvqConst), $3.line);
                        $$ = parseContext.intermediate.addIndex(EOpIndexDirectStruct, $1, index, $2.line);                
                        $$->setType(*(*fields)[i].type);
                    }
                } else {
                    parseContext.error($2.line, " no such field in structure", $3.string->c_str(), "");
                    parseContext.recover();
                    $$ = $1;
                }
            }
        } else if ($1->getNominalSize() == 1) {

            // HLSL allows ".xxxx" field selection on single component floats.  Handle that here.
            TVectorFields fields;

            // Check to make sure only the "x" component is accessed.
            if (! parseContext.parseVectorFields(*$3.string, $1->getNominalSize(), fields, $3.line)) {
                fields.num = 1;
                fields.offsets[0] = 0;
                parseContext.recover();
            }

            // Create the appropriate constructor based on the number of ".x"'s there are in the selection field
            TString vectorString = *$3.string;
            TQualifier qualifier = $1->getType().getQualifier() == EvqConst ? EvqConst : EvqTemporary;
            TType type($1->getBasicType(), $1->getPrecision(), qualifier, (int) vectorString.size());
            $$ = parseContext.constructBuiltIn(&type, parseContext.getConstructorOp(type),
                                               $$, $1->getLine(), false);
        } else {
            parseContext.error($2.line, " field selection requires structure, vector, or matrix on left hand side", $3.string->c_str(), "");
            parseContext.recover();
            $$ = $1;
        }
        // don't delete $3.string, it's from the pool
    }
    | postfix_expression INC_OP {
        if (parseContext.lValueErrorCheck($2.line, "++", $1))
            parseContext.recover();
        $$ = parseContext.intermediate.addUnaryMath(EOpPostIncrement, $1, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.unaryOpError($2.line, "++", $1->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    | postfix_expression DEC_OP {
        if (parseContext.lValueErrorCheck($2.line, "--", $1))
            parseContext.recover();
        $$ = parseContext.intermediate.addUnaryMath(EOpPostDecrement, $1, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.unaryOpError($2.line, "--", $1->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    ;

integer_expression 
    : expression {
        if (parseContext.integerErrorCheck($1, "[]"))
            parseContext.recover();
        $$ = $1; 
    }
    ;

function_call 
    : function_call_or_method {
        TFunction* fnCall = $1.function;
        TOperator op = fnCall->getBuiltInOp();
        
        if (op == EOpArrayLength) {
            if ($1.intermNode->getAsTyped() == 0 || $1.intermNode->getAsTyped()->getType().getArraySize() == 0) {
                parseContext.error($1.line, "", fnCall->getName().c_str(), "array must be declared with a size before using this method");
                parseContext.recover();
            }

            constUnion *unionArray = new constUnion[1];
            unionArray->setIConst($1.intermNode->getAsTyped()->getType().getArraySize());
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtInt, EbpUndefined, EvqConst), $1.line);
        } else if (op != EOpNull) {
            //
            // Then this should be a constructor.
            // Don't go through the symbol table for constructors.  
            // Their parameters will be verified algorithmically.
            //
            TType type(EbtVoid, EbpUndefined);  // use this to get the type back
            if (parseContext.constructorErrorCheck($1.line, $1.intermNode, *fnCall, op, &type)) {
                $$ = 0;
            } else {
                //
                // It's a constructor, of type 'type'.
                //
                $$ = parseContext.addConstructor($1.intermNode, &type, op, fnCall, $1.line);
            }
            
            if ($$ == 0) {        
                parseContext.recover();
                $$ = parseContext.intermediate.setAggregateOperator(0, op, $1.line);
            }
            $$->setType(type);
        } else {
            //
            // Not a constructor.  Find it in the symbol table.
            //
            const TFunction* fnCandidate;
            bool builtIn;
            fnCandidate = parseContext.findFunction($1.line, fnCall, &builtIn);
            
            if ( fnCandidate && fnCandidate->getMangledName() != fnCall->getMangledName()) {
                //add constructors to arguments to ensure that they have proper types
                TIntermNode *temp = parseContext.promoteFunctionArguments( $1.intermNode,
                                      fnCandidate);
                if (temp)
                    $1.intermNode = temp;
                else {
                    parseContext.error( $1.intermNode->getLine(), " unable to suitably promote arguments to function",
                                        fnCandidate->getName().c_str(), "");
                    fnCandidate = 0;
                }
            }
            
            if (fnCandidate) {
                //
                // A declared function.  But, it might still map to a built-in
                // operation.
                //
                op = fnCandidate->getBuiltInOp();
                if (builtIn && op != EOpNull) {
                    //
                    // A function call mapped to a built-in operation.
                    //
                    if (fnCandidate->getParamCount() == 1) {
                        //
                        // Treat it like a built-in unary operator.
                        //
                        $$ = parseContext.intermediate.addUnaryMath(op, $1.intermNode, gNullSourceLoc, parseContext.symbolTable);
                        if ($$ == 0)  {
                            parseContext.error($1.intermNode->getLine(), " wrong operand type", "Internal Error", 
                                "built in unary operator function.  Type: %s",
                                static_cast<TIntermTyped*>($1.intermNode)->getCompleteString().c_str());
                            YYERROR;
                        }
                    } else {
                        $$ = parseContext.intermediate.setAggregateOperator($1.intermAggregate, op, $1.line);
                    }
                } else {
                    // This is a real function call
                    
                    $$ = parseContext.intermediate.setAggregateOperator($1.intermAggregate, EOpFunctionCall, $1.line);
                    $$->setType(fnCandidate->getReturnType());                   
                    
                    $$->getAsAggregate()->setName(fnCandidate->getMangledName());
                    $$->getAsAggregate()->setPlainName(fnCandidate->getName());

                    TQualifier qual;
                    for (int i = 0; i < fnCandidate->getParamCount(); ++i) {
                        qual = (*fnCandidate)[i].type->getQualifier();
                        if (qual == EvqOut || qual == EvqInOut) {
                            if (parseContext.lValueErrorCheck($$->getLine(), "assign", $$->getAsAggregate()->getSequence()[i]->getAsTyped())) {
                                parseContext.error($1.intermNode->getLine(), "Constant value cannot be passed for 'out' or 'inout' parameters.", "Error", "");
                                parseContext.recover();
                            }
                        }
                    }
                }
                $$->setType(fnCandidate->getReturnType());
            } else {
                // error message was put out by PaFindFunction()
                // Put on a dummy node for error recovery
                constUnion *unionArray = new constUnion[1];
                unionArray->setFConst(0.0f);
                $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtFloat, EbpUndefined, EvqConst), $1.line);
                parseContext.recover();
            }
        }
        delete fnCall;
    }
    ;

function_call_or_method
    : function_call_generic {
        $$ = $1;
    }
    | postfix_expression DOT function_call_generic {
        if ($1->isArray() && $3.function->getName() == "length") {
            //
            // implement array.length()
            //            
            $$ = $3;
            $$.intermNode = $1;
            $$.function->relateToOperator(EOpArrayLength);
            
        } else {
            parseContext.error($3.line, "methods are not supported", "", "");
            parseContext.recover();
            $$ = $3;
        }
    }
    ;

function_call_generic
    : function_call_header_with_parameters RIGHT_PAREN {
        $$ = $1;
        $$.line = $2.line;
    }
    | function_call_header_no_parameters RIGHT_PAREN {
        $$ = $1;
        $$.line = $2.line;
    }
    ;
    
function_call_header_no_parameters 
    : function_call_header VOID_TYPE {
        $$.function = $1;
        $$.intermNode = 0;
    }
    | function_call_header {
        $$.function = $1;
        $$.intermNode = 0;
    }
    ;

function_call_header_with_parameters
    : function_call_header assignment_expression {
		if (!$2) {
          YYERROR;
		}
		TParameter param = { 0, 0, new TType($2->getType()) };
        $1->addParameter(param);
        $$.function = $1;
        $$.intermNode = $2;
    }
    | function_call_header_with_parameters COMMA assignment_expression {
		if (!$3) {
          YYERROR;
		}
        TParameter param = { 0, 0, new TType($3->getType()) };
        $1.function->addParameter(param);
        $$.function = $1.function;
        $$.intermNode = parseContext.intermediate.growAggregate($1.intermNode, $3, $2.line);
    }
    ;

function_call_header 
    : function_identifier LEFT_PAREN {
        $$ = $1;
    }
    ;

// Grammar Note:  Constructors look like functions, but are recognized as types.
    
function_identifier
    : type_specifier {
        //
        // Constructor
        //
        if ($1.array) {
            //TODO : figure out how to deal with array constructors            
        }

        if ($1.userDef) {
            TString tempString = "";
            TType type($1);
            TFunction *function = new TFunction(&tempString, type, EOpConstructStruct);
            $$ = function;
        } else {
            TOperator op = EOpNull;
            switch ($1.type) {
            case EbtFloat:
                if ($1.matrix) {
                    switch($1.size) {
                    case 2:                                     op = EOpConstructMat2;  break;
                    case 3:                                     op = EOpConstructMat3;  break;
                    case 4:                                     op = EOpConstructMat4;  break;
                    }         
                } else {      
                    switch($1.size) {
                    case 1:                                     op = EOpConstructFloat; break;
                    case 2:                                     op = EOpConstructVec2;  break;
                    case 3:                                     op = EOpConstructVec3;  break;
                    case 4:                                     op = EOpConstructVec4;  break;
                    }       
                }  
                break;               
            case EbtInt:
                switch($1.size) {
                case 1:                                         op = EOpConstructInt;   break;
                case 2:       FRAG_VERT_ONLY("ivec2", $1.line); op = EOpConstructIVec2; break;
                case 3:       FRAG_VERT_ONLY("ivec3", $1.line); op = EOpConstructIVec3; break;
                case 4:       FRAG_VERT_ONLY("ivec4", $1.line); op = EOpConstructIVec4; break;
                }         
                break;    
            case EbtBool:
                switch($1.size) {
                case 1:                                         op = EOpConstructBool;  break;
                case 2:       FRAG_VERT_ONLY("bvec2", $1.line); op = EOpConstructBVec2; break;
                case 3:       FRAG_VERT_ONLY("bvec3", $1.line); op = EOpConstructBVec3; break;
                case 4:       FRAG_VERT_ONLY("bvec4", $1.line); op = EOpConstructBVec4; break;
                }         
                break;
            }
            if (op == EOpNull) {                    
                parseContext.error($1.line, "cannot construct this type", TType::getBasicString($1.type), "");
                parseContext.recover();
                $1.type = EbtFloat;
                op = EOpConstructFloat;
            }            
            TString tempString = "";
            TType type($1);
            TFunction *function = new TFunction(&tempString, type, op);
            $$ = function;
        }
    }
    | IDENTIFIER {
        if (parseContext.reservedErrorCheck($1.line, *$1.string)) 
            parseContext.recover();
        TType type(EbtVoid, EbpUndefined);
	const TString *mangled;
	if ( *$1.string == "main")
	    mangled = NewPoolTString("xlat_main");
	else
	    mangled = $1.string;
        TFunction *function = new TFunction( mangled, type);
        $$ = function;
    }
    | FIELD_SELECTION {
        if (parseContext.reservedErrorCheck($1.line, *$1.string)) 
            parseContext.recover();
        TType type(EbtVoid, EbpUndefined);
        TFunction *function = new TFunction($1.string, type);
        $$ = function;
    }
    ;

unary_expression
    : postfix_expression {
        $$ = $1;
    }
    | INC_OP unary_expression {
        if (parseContext.lValueErrorCheck($1.line, "++", $2))
            parseContext.recover();
        $$ = parseContext.intermediate.addUnaryMath(EOpPreIncrement, $2, $1.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.unaryOpError($1.line, "++", $2->getCompleteString());
            parseContext.recover();
            $$ = $2;
        }
    }
    | DEC_OP unary_expression {
        if (parseContext.lValueErrorCheck($1.line, "--", $2))
            parseContext.recover();
        $$ = parseContext.intermediate.addUnaryMath(EOpPreDecrement, $2, $1.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.unaryOpError($1.line, "--", $2->getCompleteString());
            parseContext.recover();
            $$ = $2;
        }
    }
    | unary_operator unary_expression {
        if ($1.op != EOpNull) {
            $$ = parseContext.intermediate.addUnaryMath($1.op, $2, $1.line, parseContext.symbolTable);
            if ($$ == 0) {
                const char* errorOp = "";
                switch($1.op) {
                case EOpNegative:   errorOp = "-"; break;
                case EOpLogicalNot: errorOp = "!"; break;
                case EOpBitwiseNot: errorOp = "~"; break;
				default: break;
                }
                parseContext.unaryOpError($1.line, const_cast<char*> (errorOp), $2->getCompleteString());
                parseContext.recover();
                $$ = $2;
            }
        } else
            $$ = $2;
    }
    | LEFT_PAREN type_specifier_nonarray RIGHT_PAREN unary_expression {
        // cast operator, insert constructor
        TOperator op = EOpNull;
        switch ($2.type) {
        case EbtFloat:
            if ($2.matrix) {
                switch($2.size) {
                case 2:                                     op = EOpConstructMat2;  break;
                case 3:                                     op = EOpConstructMat3;  break;
                case 4:                                     op = EOpConstructMat4;  break;
                }         
            } else {      
                switch($2.size) {
                case 1:                                     op = EOpConstructFloat; break;
                case 2:                                     op = EOpConstructVec2;  break;
                case 3:                                     op = EOpConstructVec3;  break;
                case 4:                                     op = EOpConstructVec4;  break;
                }       
            }  
            break;               
        case EbtInt:
            switch($2.size) {
            case 1:                                         op = EOpConstructInt;   break;
            case 2:       FRAG_VERT_ONLY("ivec2", $2.line); op = EOpConstructIVec2; break;
            case 3:       FRAG_VERT_ONLY("ivec3", $2.line); op = EOpConstructIVec3; break;
            case 4:       FRAG_VERT_ONLY("ivec4", $2.line); op = EOpConstructIVec4; break;
            }         
            break;    
        case EbtBool:
            switch($2.size) {
            case 1:                                         op = EOpConstructBool;  break;
            case 2:       FRAG_VERT_ONLY("bvec2", $2.line); op = EOpConstructBVec2; break;
            case 3:       FRAG_VERT_ONLY("bvec3", $2.line); op = EOpConstructBVec3; break;
            case 4:       FRAG_VERT_ONLY("bvec4", $2.line); op = EOpConstructBVec4; break;
            }         
            break;
        case EbtStruct:
            op = EOpConstructStruct;
            break;
        }
        if (op == EOpNull) {                    
            parseContext.error($2.line, "cannot cast this type", TType::getBasicString($2.type), "");
            parseContext.recover();
            $2.type = EbtFloat;
            op = EOpConstructFloat;
        }            
        TString tempString = "";
        TType type($2);
        TFunction *function = new TFunction(&tempString, type, op);
        TParameter param = { 0, 0, new TType($4->getType()) };
        function->addParameter(param);
        TType type2(EbtVoid, EbpUndefined);  // use this to get the type back
        if (parseContext.constructorErrorCheck($2.line, $4, *function, op, &type2)) {
            $$ = 0;
        } else {
            //
            // It's a constructor, of type 'type'.
            //
            $$ = parseContext.addConstructor($4, &type2, op, function, $2.line);
        }
        
        if ($$ == 0) {        
            parseContext.recover();
            $$ = parseContext.intermediate.setAggregateOperator(0, op, $2.line);
        }
        $$->setType(type2);
    }
    ;
// Grammar Note:  No traditional style type casts.

unary_operator
    : PLUS  { $$.line = $1.line; $$.op = EOpNull; }
    | DASH  { $$.line = $1.line; $$.op = EOpNegative; }
    | BANG  { $$.line = $1.line; $$.op = EOpLogicalNot; }
    | TILDE { UNSUPPORTED_FEATURE("~", $1.line);  
              $$.line = $1.line; $$.op = EOpBitwiseNot; }
    ;
// Grammar Note:  No '*' or '&' unary ops.  Pointers are not supported.

multiplicative_expression
    : unary_expression { $$ = $1; }
    | multiplicative_expression STAR unary_expression {
        FRAG_VERT_ONLY("*", $2.line);
        $$ = parseContext.intermediate.addBinaryMath(EOpMul, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "*", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    | multiplicative_expression SLASH unary_expression {
        FRAG_VERT_ONLY("/", $2.line); 
        $$ = parseContext.intermediate.addBinaryMath(EOpDiv, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "/", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    | multiplicative_expression PERCENT unary_expression {
        $$ = parseContext.intermediate.addBinaryMath(EOpMod, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "%", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    ;

additive_expression
    : multiplicative_expression { $$ = $1; }
    | additive_expression PLUS multiplicative_expression {  
        $$ = parseContext.intermediate.addBinaryMath(EOpAdd, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "+", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    | additive_expression DASH multiplicative_expression {
        $$ = parseContext.intermediate.addBinaryMath(EOpSub, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "-", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        } 
    }
    ;

shift_expression
    : additive_expression { $$ = $1; }
    | shift_expression LEFT_OP additive_expression {
        UNSUPPORTED_FEATURE("<<", $2.line);
        $$ = parseContext.intermediate.addBinaryMath(EOpLeftShift, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "<<", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    | shift_expression RIGHT_OP additive_expression {
        UNSUPPORTED_FEATURE(">>", $2.line);
        $$ = parseContext.intermediate.addBinaryMath(EOpRightShift, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, ">>", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    ;

relational_expression
    : shift_expression { $$ = $1; }
    | relational_expression LEFT_ANGLE shift_expression { 
        $$ = parseContext.intermediate.addBinaryMath(EOpLessThan, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "<", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        }
    }
    | relational_expression RIGHT_ANGLE shift_expression  { 
        $$ = parseContext.intermediate.addBinaryMath(EOpGreaterThan, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, ">", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        }
    }
    | relational_expression LE_OP shift_expression  { 
        $$ = parseContext.intermediate.addBinaryMath(EOpLessThanEqual, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "<=", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        }
    }
    | relational_expression GE_OP shift_expression  { 
        $$ = parseContext.intermediate.addBinaryMath(EOpGreaterThanEqual, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, ">=", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        }
    }
    ;

equality_expression
    : relational_expression { $$ = $1; }
    | equality_expression EQ_OP relational_expression  {
        $$ = parseContext.intermediate.addBinaryMath(EOpEqual, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "==", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        } else if (($1->isArray() || $3->isArray()))
            parseContext.recover();
    }
    | equality_expression NE_OP relational_expression { 
        $$ = parseContext.intermediate.addBinaryMath(EOpNotEqual, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "!=", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        } else if (($1->isArray() || $3->isArray()))
            parseContext.recover();
    }
    ;

and_expression
    : equality_expression { $$ = $1; }
    | and_expression AMPERSAND equality_expression {
        UNSUPPORTED_FEATURE("&", $2.line);
        $$ = parseContext.intermediate.addBinaryMath(EOpAnd, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "&", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    ;

exclusive_or_expression
    : and_expression { $$ = $1; }
    | exclusive_or_expression CARET and_expression {
        UNSUPPORTED_FEATURE("^", $2.line);
        $$ = parseContext.intermediate.addBinaryMath(EOpExclusiveOr, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "^", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    ;

inclusive_or_expression
    : exclusive_or_expression { $$ = $1; }
    | inclusive_or_expression VERTICAL_BAR exclusive_or_expression {
        UNSUPPORTED_FEATURE("|", $2.line);
        $$ = parseContext.intermediate.addBinaryMath(EOpInclusiveOr, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "|", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        }
    }
    ;

logical_and_expression
    : inclusive_or_expression { $$ = $1; }
    | logical_and_expression AND_OP inclusive_or_expression {
        $$ = parseContext.intermediate.addBinaryMath(EOpLogicalAnd, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "&&", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        }
    }
    ;

logical_xor_expression
    : logical_and_expression { $$ = $1; }
    | logical_xor_expression XOR_OP logical_and_expression  { 
        $$ = parseContext.intermediate.addBinaryMath(EOpLogicalXor, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "^^", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        }
    }
    ;

logical_or_expression
    : logical_xor_expression { $$ = $1; }
    | logical_or_expression OR_OP logical_xor_expression  { 
        $$ = parseContext.intermediate.addBinaryMath(EOpLogicalOr, $1, $3, $2.line, parseContext.symbolTable);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, "||", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            constUnion *unionArray = new constUnion[1];
            unionArray->setBConst(false);
            $$ = parseContext.intermediate.addConstantUnion(unionArray, TType(EbtBool, EbpUndefined, EvqConst), $2.line);
        }
    }
    ;

conditional_expression
    : logical_or_expression { $$ = $1; }
    | logical_or_expression QUESTION expression COLON assignment_expression {
       if (parseContext.boolOrVectorErrorCheck($2.line, $1))
            parseContext.recover();
       
		$$ = parseContext.intermediate.addSelection($1, $3, $5, $2.line);
           
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, ":", $3->getCompleteString(), $5->getCompleteString());
            parseContext.recover();
            $$ = $5;
        }
    }
    ;

assignment_expression
    : conditional_expression { $$ = $1; }
    | unary_expression assignment_operator assignment_expression {        
        if (parseContext.lValueErrorCheck($2.line, "assign", $1))
            parseContext.recover();
        $$ = parseContext.addAssign($2.op, $1, $3, $2.line);
        if ($$ == 0) {
            parseContext.assignError($2.line, "assign", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $1;
        } else if (($1->isArray() || $3->isArray()))
            parseContext.recover();
    }
    ;

assignment_operator
    : EQUAL        {                                    $$.line = $1.line; $$.op = EOpAssign; }
    | MUL_ASSIGN   { FRAG_VERT_ONLY("*=", $1.line);     $$.line = $1.line; $$.op = EOpMulAssign; }
    | DIV_ASSIGN   { FRAG_VERT_ONLY("/=", $1.line);     $$.line = $1.line; $$.op = EOpDivAssign; }
    | MOD_ASSIGN   { UNSUPPORTED_FEATURE("%=", $1.line);   $$.line = $1.line; $$.op = EOpModAssign; }
    | ADD_ASSIGN   {                                    $$.line = $1.line; $$.op = EOpAddAssign; }
    | SUB_ASSIGN   {                                    $$.line = $1.line; $$.op = EOpSubAssign; }
    | LEFT_ASSIGN  { UNSUPPORTED_FEATURE("<<=", $1.line);  $$.line = $1.line; $$.op = EOpLeftShiftAssign; }
    | RIGHT_ASSIGN { UNSUPPORTED_FEATURE("<<=", $1.line);  $$.line = $1.line; $$.op = EOpRightShiftAssign; }
    | AND_ASSIGN   { UNSUPPORTED_FEATURE("&=",  $1.line);  $$.line = $1.line; $$.op = EOpAndAssign; }
    | XOR_ASSIGN   { UNSUPPORTED_FEATURE("^=",  $1.line);  $$.line = $1.line; $$.op = EOpExclusiveOrAssign; }
    | OR_ASSIGN    { UNSUPPORTED_FEATURE("|=",  $1.line);  $$.line = $1.line; $$.op = EOpInclusiveOrAssign; }
    ;

expression
    : assignment_expression {
        $$ = $1;
    }
    | expression COMMA assignment_expression {
        $$ = parseContext.intermediate.addComma($1, $3, $2.line);
        if ($$ == 0) {
            parseContext.binaryOpError($2.line, ",", $1->getCompleteString(), $3->getCompleteString());
            parseContext.recover();
            $$ = $3;
        }
    }
    ;

constant_expression
    : conditional_expression {
        if (parseContext.constErrorCheck($1))
            parseContext.recover();
        $$ = $1;
    }
    ;

declaration
    : function_prototype SEMICOLON   { $$ = 0; }
    | init_declarator_list SEMICOLON { 
        if ($1.intermAggregate)
            $1.intermAggregate->setOperator(EOpSequence); 
        $$ = $1.intermAggregate; 
    }
    ;

function_prototype 
    : function_declarator RIGHT_PAREN  {
        //
        // Multiple declarations of the same function are allowed.
        //
        // If this is a definition, the definition production code will check for redefinitions 
        // (we don't know at this point if it's a definition or not).
        //
        // Redeclarations are allowed.  But, return types and parameter qualifiers must match.
        //        
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find($1->getMangledName()));
        if (prevDec) {
            if (prevDec->getReturnType() != $1->getReturnType()) {
                parseContext.error($2.line, "overloaded functions must have the same return type", $1->getReturnType().getBasicString(), "");
                parseContext.recover();
            }
            for (int i = 0; i < prevDec->getParamCount(); ++i) {
                if ((*prevDec)[i].type->getQualifier() != (*$1)[i].type->getQualifier()) {
                    parseContext.error($2.line, "overloaded functions must have the same parameter qualifiers", (*$1)[i].type->getQualifierString(), "");
                    parseContext.recover();
                }
            }
        }
        
        //
        // If this is a redeclaration, it could also be a definition,
        // in which case, we want to use the variable names from this one, and not the one that's
        // being redeclared.  So, pass back up this declaration, not the one in the symbol table.
        //
        $$.function = $1;
        $$.line = $2.line;

        parseContext.symbolTable.insert(*$$.function);
    }
    | function_declarator RIGHT_PAREN COLON IDENTIFIER {
        //
        // Multiple declarations of the same function are allowed.
        //
        // If this is a definition, the definition production code will check for redefinitions 
        // (we don't know at this point if it's a definition or not).
        //
        // Redeclarations are allowed.  But, return types and parameter qualifiers must match.
        //        
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find($1->getMangledName()));
        if (prevDec) {
            if (prevDec->getReturnType() != $1->getReturnType()) {
                parseContext.error($2.line, "overloaded functions must have the same return type", $1->getReturnType().getBasicString(), "");
                parseContext.recover();
            }
            for (int i = 0; i < prevDec->getParamCount(); ++i) {
                if ((*prevDec)[i].type->getQualifier() != (*$1)[i].type->getQualifier()) {
                    parseContext.error($2.line, "overloaded functions must have the same parameter qualifiers", (*$1)[i].type->getQualifierString(), "");
                    parseContext.recover();
                }
            }
        }
        
        //
        // If this is a redeclaration, it could also be a definition,
        // in which case, we want to use the variable names from this one, and not the one that's
        // being redeclared.  So, pass back up this declaration, not the one in the symbol table.
        //
        $$.function = $1;
        $$.line = $2.line;
        $$.function->setInfo(new TTypeInfo(*$4.string, 0));

        parseContext.symbolTable.insert(*$$.function);
    }
    ;

function_declarator 
    : function_header {
        $$ = $1;
    }
    | function_header_with_parameters { 
        $$ = $1;  
    }
    ;


function_header_with_parameters
    : function_header parameter_declaration {
        // Add the parameter 
        $$ = $1;
        if ($2.param.type->getBasicType() != EbtVoid)
            $1->addParameter($2.param);
        else
            delete $2.param.type;
    }
    | function_header_with_parameters COMMA parameter_declaration {   
        //
        // Only first parameter of one-parameter functions can be void
        // The check for named parameters not being void is done in parameter_declarator 
        //
        if ($3.param.type->getBasicType() == EbtVoid) {
            //
            // This parameter > first is void
            //
            parseContext.error($2.line, "cannot be an argument type except for '(void)'", "void", "");
            parseContext.recover();
            delete $3.param.type;
        } else {
            // Add the parameter 
            $$ = $1; 
            $1->addParameter($3.param);
        }
    }
    ;

function_header 
    : fully_specified_type IDENTIFIER LEFT_PAREN {
        if ($1.qualifier != EvqGlobal && $1.qualifier != EvqTemporary) {
            parseContext.error($2.line, "no qualifiers allowed for function return", getQualifierString($1.qualifier), "");
            parseContext.recover();
        }
        // make sure a sampler is not involved as well...
        if (parseContext.structQualifierErrorCheck($2.line, $1))
            parseContext.recover();
        
        // Add the function as a prototype after parsing it (we do not support recursion) 
        TFunction *function;
        TType type($1);
	const TString* mangled = 0;
	if ( *$2.string == "main")
	    mangled = NewPoolTString( "xlat_main");
	else
	    mangled = $2.string;

        function = new TFunction(mangled, type);
        $$ = function;
    }
    ;

parameter_declarator
    // Type + name 
    : type_specifier IDENTIFIER {
        if ($1.type == EbtVoid) {
            parseContext.error($2.line, "illegal use of type 'void'", $2.string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck($2.line, *$2.string))
            parseContext.recover();
        TParameter param = {$2.string, 0, new TType($1)};
        $$.line = $2.line;
        $$.param = param; 
    }
    | type_specifier IDENTIFIER EQUAL initializer {
        if ($1.type == EbtVoid) {
            parseContext.error($2.line, "illegal use of type 'void'", $2.string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck($2.line, *$2.string))
            parseContext.recover();
        TParameter param = {$2.string, 0, new TType($1)};
        $$.line = $2.line;
        $$.param = param; 
        
        //TODO: add initializer support
    }
    | type_specifier IDENTIFIER register_specifier {
        // register is being ignored
        if ($1.type == EbtVoid) {
            parseContext.error($2.line, "illegal use of type 'void'", $2.string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck($2.line, *$2.string))
            parseContext.recover();
        TParameter param = {$2.string, 0, new TType($1)};
        $$.line = $2.line;
        $$.param = param; 
    }
    | type_specifier IDENTIFIER COLON IDENTIFIER {
        //Parameter with semantic
        if ($1.type == EbtVoid) {
            parseContext.error($2.line, "illegal use of type 'void'", $2.string->c_str(), "");
            parseContext.recover();
        }
        if (parseContext.reservedErrorCheck($2.line, *$2.string))
            parseContext.recover();
        TParameter param = {$2.string, new TTypeInfo(*$4.string, 0), new TType($1)};
        $$.line = $2.line;
        $$.param = param; 
    }
    | type_specifier IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET {
        // Check that we can make an array out of this type
        if (parseContext.arrayTypeErrorCheck($3.line, $1))
            parseContext.recover();
            
        if (parseContext.reservedErrorCheck($2.line, *$2.string))
            parseContext.recover();
            
        int size;
        if (parseContext.arraySizeErrorCheck($3.line, $4, size))
            parseContext.recover();
        $1.setArray(true, size);
             
        TType* type = new TType($1);
        TParameter param = { $2.string, 0, type };
        $$.line = $2.line;
        $$.param = param;
    }
    | type_specifier IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET COLON IDENTIFIER {
        // Check that we can make an array out of this type
        if (parseContext.arrayTypeErrorCheck($3.line, $1))
            parseContext.recover();
            
        if (parseContext.reservedErrorCheck($2.line, *$2.string))
            parseContext.recover();
            
        int size;
        if (parseContext.arraySizeErrorCheck($3.line, $4, size))
            parseContext.recover();
        $1.setArray(true, size);
             
        TType* type = new TType($1);
        TParameter param = { $2.string, new TTypeInfo(*$7.string, 0), type };
        $$.line = $2.line;
        $$.param = param;
    }
    ;

parameter_declaration 
    // 
    // The only parameter qualifier a parameter can have are 
    // IN_QUAL, OUT_QUAL, INOUT_QUAL, or CONST.
    //
    
    //
    // Type + name 
    //
    : type_qualifier parameter_qualifier parameter_declarator {
        $$ = $3;
        if (parseContext.paramErrorCheck($3.line, $1.qualifier, $2, $$.param.type))
            parseContext.recover();
    }
    | parameter_qualifier parameter_declarator {
        $$ = $2;
        if (parseContext.parameterSamplerErrorCheck($2.line, $1, *$2.param.type))
            parseContext.recover();
        if (parseContext.paramErrorCheck($2.line, EvqTemporary, $1, $$.param.type))
            parseContext.recover();
    }
    //
    // Only type 
    //
    | type_qualifier parameter_qualifier parameter_type_specifier {
        $$ = $3;
        if (parseContext.paramErrorCheck($3.line, $1.qualifier, $2, $$.param.type))
            parseContext.recover();
    }
    | parameter_qualifier parameter_type_specifier {
        $$ = $2;
        if (parseContext.parameterSamplerErrorCheck($2.line, $1, *$2.param.type))
            parseContext.recover();
        if (parseContext.paramErrorCheck($2.line, EvqTemporary, $1, $$.param.type))
            parseContext.recover();
    }
    ;
    
parameter_qualifier
    : /* empty */ {
        $$ = EvqIn;
    }
    | IN_QUAL {
        $$ = EvqIn;
    }
    | OUT_QUAL {
        $$ = EvqOut;
    }
    | INOUT_QUAL {
        $$ = EvqInOut;
    }
    ;

parameter_type_specifier 
    : type_specifier {
        TParameter param = { 0, 0, new TType($1) };
        $$.param = param;
    }
    ;

init_declarator_list
    : single_declaration {
        $$ = $1;
    } 
    | init_declarator_list COMMA IDENTIFIER type_info {
        $$ = $1;
        if (parseContext.structQualifierErrorCheck($3.line, $$.type))
            parseContext.recover();
        
        if (parseContext.nonInitConstErrorCheck($3.line, *$3.string, $$.type))
            parseContext.recover();

        if (parseContext.nonInitErrorCheck($3.line, *$3.string, $4, $$.type))
            parseContext.recover();
    }
    | init_declarator_list COMMA IDENTIFIER LEFT_BRACKET RIGHT_BRACKET type_info {
        if (parseContext.structQualifierErrorCheck($3.line, $1.type))
            parseContext.recover();
            
        if (parseContext.nonInitConstErrorCheck($3.line, *$3.string, $1.type))
            parseContext.recover();

        $$ = $1;
        
        if (parseContext.arrayTypeErrorCheck($4.line, $1.type) || parseContext.arrayQualifierErrorCheck($4.line, $1.type))
            parseContext.recover();
        else {
            $1.type.setArray(true);
            TVariable* variable;
            if (parseContext.arrayErrorCheck($4.line, *$3.string, $6, $1.type, variable))
                parseContext.recover();
        }
    }
    | init_declarator_list COMMA IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET type_info {
        if (parseContext.structQualifierErrorCheck($3.line, $1.type))
            parseContext.recover();
            
        if (parseContext.nonInitConstErrorCheck($3.line, *$3.string, $1.type))
            parseContext.recover();
        
        $$ = $1;

        if (parseContext.arrayTypeErrorCheck($4.line, $1.type) || parseContext.arrayQualifierErrorCheck($4.line, $1.type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck($4.line, $5, size))
                parseContext.recover();
            $1.type.setArray(true, size);        
            TVariable* variable;
            if (parseContext.arrayErrorCheck($4.line, *$3.string, $7, $1.type, variable))
                parseContext.recover();
        }
    }
    | init_declarator_list COMMA IDENTIFIER LEFT_BRACKET RIGHT_BRACKET type_info EQUAL initializer {
        if (parseContext.structQualifierErrorCheck($3.line, $1.type))
            parseContext.recover();
            
        $$ = $1;
            
        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck($4.line, $1.type) || parseContext.arrayQualifierErrorCheck($4.line, $1.type))
            parseContext.recover();
        else {
			$1.type.setArray(true, $8->getType().getArraySize());
            if (parseContext.arrayErrorCheck($4.line, *$3.string, $1.type, variable))
                parseContext.recover();
        }

        {
            TIntermNode* intermNode;
            if (!parseContext.executeInitializer($3.line, *$3.string, $6, $1.type, $8, intermNode, variable)) {
                //
                // build the intermediate representation
                //
                if (intermNode)
                    $$.intermAggregate = parseContext.intermediate.growAggregate($1.intermNode, intermNode, $7.line);
                else
                    $$.intermAggregate = $1.intermAggregate;
            } else {
                parseContext.recover();
                $$.intermAggregate = 0;
            }
        }
    }
    | init_declarator_list COMMA IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET type_info EQUAL initializer {
        if (parseContext.structQualifierErrorCheck($3.line, $1.type))
            parseContext.recover();
            
        $$ = $1;
            
        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck($4.line, $1.type) || parseContext.arrayQualifierErrorCheck($4.line, $1.type))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck($4.line, $5, size))
                parseContext.recover();
            $1.type.setArray(true, size);
            if (parseContext.arrayErrorCheck($4.line, *$3.string, $7, $1.type, variable))
                parseContext.recover();
        }

        {
            TIntermNode* intermNode;
            if (!parseContext.executeInitializer($3.line, *$3.string, $7, $1.type, $9, intermNode, variable)) {
                //
                // build the intermediate representation
                //
                if (intermNode)
                    $$.intermAggregate = parseContext.intermediate.growAggregate($1.intermNode, intermNode, $8.line);
                else
                    $$.intermAggregate = $1.intermAggregate;
            } else {
                parseContext.recover();
                $$.intermAggregate = 0;
            }
        }
    }
    | init_declarator_list COMMA IDENTIFIER type_info EQUAL initializer {
        if (parseContext.structQualifierErrorCheck($3.line, $1.type))
            parseContext.recover();
        
        $$ = $1;
        
        TIntermNode* intermNode;
	if ( !IsSampler($1.type.type)) {
	    if (!parseContext.executeInitializer($3.line, *$3.string, $4, $1.type, $6, intermNode)) {
		//
		// build the intermediate representation
		//
		if (intermNode)
		    $$.intermAggregate = parseContext.intermediate.growAggregate($1.intermNode, intermNode, $5.line);
		else
		    $$.intermAggregate = $1.intermAggregate;
	    } else {
		parseContext.recover();
		$$.intermAggregate = 0;
	    }
	}
	else {
	    //Special code to skip initializers for samplers
	    $$ = $1;
	    if (parseContext.structQualifierErrorCheck($3.line, $$.type))
		parseContext.recover();
	    
	    if (parseContext.nonInitConstErrorCheck($3.line, *$3.string, $$.type))
		parseContext.recover();
	    
	    if (parseContext.nonInitErrorCheck($3.line, *$3.string, $4, $$.type))
		parseContext.recover();
	}
    }
    ;

single_declaration 
    : fully_specified_type {
        $$.type = $1;
        $$.intermAggregate = 0;
    }    
    | fully_specified_type IDENTIFIER type_info {
        $$.intermAggregate = 0;
        $$.type = $1;

        if (parseContext.structQualifierErrorCheck($2.line, $$.type))
            parseContext.recover();
        
        if (parseContext.nonInitConstErrorCheck($2.line, *$2.string, $$.type))
            parseContext.recover();

        if (parseContext.nonInitErrorCheck($2.line, *$2.string, $3, $$.type))
            parseContext.recover();
    }
    | fully_specified_type IDENTIFIER LEFT_BRACKET RIGHT_BRACKET type_info {
        $$.intermAggregate = 0;
        if (parseContext.structQualifierErrorCheck($2.line, $1))
            parseContext.recover();

        if (parseContext.nonInitConstErrorCheck($2.line, *$2.string, $1))
            parseContext.recover();

        $$.type = $1;

        if (parseContext.arrayTypeErrorCheck($3.line, $1) || parseContext.arrayQualifierErrorCheck($3.line, $1))
            parseContext.recover();
        else {
            $1.setArray(true);        
            TVariable* variable;
            if (parseContext.arrayErrorCheck($3.line, *$2.string, $5, $1, variable))
                parseContext.recover();
        }
    }
    | fully_specified_type IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET type_info {
        $$.intermAggregate = 0;
        if (parseContext.structQualifierErrorCheck($2.line, $1))
            parseContext.recover();

        if (parseContext.nonInitConstErrorCheck($2.line, *$2.string, $1))
            parseContext.recover();
            
        $$.type = $1;

        if (parseContext.arrayTypeErrorCheck($3.line, $1) || parseContext.arrayQualifierErrorCheck($3.line, $1))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck($3.line, $4, size))
                parseContext.recover();
            
            $1.setArray(true, size);
            TVariable* variable;
            if (parseContext.arrayErrorCheck($3.line, *$2.string, $6, $1, variable))
                parseContext.recover();
        }
    }
    | fully_specified_type IDENTIFIER LEFT_BRACKET RIGHT_BRACKET type_info EQUAL initializer {
        $$.intermAggregate = 0;

        if (parseContext.structQualifierErrorCheck($2.line, $1))
            parseContext.recover();

        $$.type = $1;

        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck($3.line, $1) || parseContext.arrayQualifierErrorCheck($3.line, $1))
            parseContext.recover();
        else {
            $1.setArray(true, $7->getType().getArraySize());
            if (parseContext.arrayErrorCheck($3.line, *$2.string, $5, $1, variable))
                parseContext.recover();
        }

        
	     {        
            TIntermNode* intermNode;
            if (!parseContext.executeInitializer($2.line, *$2.string, $5, $1, $7, intermNode, variable)) {
                //
                // Build intermediate representation
                //
                if (intermNode)
                    $$.intermAggregate = parseContext.intermediate.makeAggregate(intermNode, $6.line);
                else
                    $$.intermAggregate = 0;
            } else {
                parseContext.recover();
                $$.intermAggregate = 0;
            }
        }
    }
    | fully_specified_type IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET type_info EQUAL initializer {
        $$.intermAggregate = 0;

        if (parseContext.structQualifierErrorCheck($2.line, $1))
            parseContext.recover();

        $$.type = $1;

        TVariable* variable = 0;
        if (parseContext.arrayTypeErrorCheck($3.line, $1) || parseContext.arrayQualifierErrorCheck($3.line, $1))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck($3.line, $4, size))
                parseContext.recover();
            
            $1.setArray(true, size);
            if (parseContext.arrayErrorCheck($3.line, *$2.string, $6, $1, variable))
                parseContext.recover();
        }
        
        {        
            TIntermNode* intermNode;
            if (!parseContext.executeInitializer($2.line, *$2.string, $6, $1, $8, intermNode, variable)) {
                //
                // Build intermediate representation
                //
                if (intermNode)
                    $$.intermAggregate = parseContext.intermediate.makeAggregate(intermNode, $7.line);
                else
                    $$.intermAggregate = 0;
            } else {
                parseContext.recover();
                $$.intermAggregate = 0;
            }
        }        
    }
    | fully_specified_type IDENTIFIER type_info EQUAL initializer {
        if (parseContext.structQualifierErrorCheck($2.line, $1))
            parseContext.recover();

        $$.type = $1;

        TIntermNode* intermNode;
	if ( !IsSampler($1.type)) {
	    if (!parseContext.executeInitializer($2.line, *$2.string, $3, $1, $5, intermNode)) {
		//
		// Build intermediate representation
		//
		if (intermNode)
		    $$.intermAggregate = parseContext.intermediate.makeAggregate(intermNode, $4.line);
		else
		    $$.intermAggregate = 0;
	    } else {
		parseContext.recover();
		$$.intermAggregate = 0;
	    }
	}
	else {
	    //Skip sampler initializers for now
	    $$.intermAggregate = 0;

	    if (parseContext.structQualifierErrorCheck($2.line, $$.type))
		parseContext.recover();
	    
	    if (parseContext.nonInitConstErrorCheck($2.line, *$2.string, $$.type))
		parseContext.recover();
	    
	    if (parseContext.nonInitErrorCheck($2.line, *$2.string, $3, $$.type))
		parseContext.recover();
	}
    }
    
    ;

// Grammar Note:  No 'enum', or 'typedef'.


//
// Input/output semantics:
//   float must be 16 or 32 bits
//   float alignment restrictions?
//   check for only one input and only one output
//   sum of bitfields has to be multiple of 32
//


fully_specified_type
    : type_specifier {
        $$ = $1;                       
    }
    | type_qualifier type_specifier  {                
        if ($2.array && parseContext.arrayQualifierErrorCheck($2.line, $1)) {
            parseContext.recover();
            $2.setArray(false);
        }
        
        if ($1.qualifier == EvqAttribute &&
            ($2.type == EbtBool || $2.type == EbtInt)) {
            parseContext.error($2.line, "cannot be bool or int", getQualifierString($1.qualifier), "");
            parseContext.recover();
        }
        if (($1.qualifier == EvqVaryingIn || $1.qualifier == EvqVaryingOut) &&
            ($2.type == EbtBool || $2.type == EbtInt)) {
            parseContext.error($2.line, "cannot be bool or int", getQualifierString($1.qualifier), "");
            parseContext.recover();
        }
        $$ = $2; 
        $$.qualifier = $1.qualifier;
    }
    ;

type_qualifier
    : CONST_QUAL {
        $$.setBasic(EbtVoid, EvqConst, $1.line);
    }
    | STATIC_QUAL {
        if (parseContext.globalErrorCheck($1.line, parseContext.symbolTable.atGlobalLevel(), "static"))
            parseContext.recover();
        $$.setBasic(EbtVoid, EvqStatic, $1.line);
    }
    | STATIC_QUAL CONST_QUAL {
        if (parseContext.globalErrorCheck($1.line, parseContext.symbolTable.atGlobalLevel(), "static const"))
            parseContext.recover();
        $$.setBasic(EbtVoid, EvqStaticConst, $1.line);
    }
    | ATTRIBUTE { 
        VERTEX_ONLY("attribute", $1.line);
        if (parseContext.globalErrorCheck($1.line, parseContext.symbolTable.atGlobalLevel(), "attribute"))
            parseContext.recover();
        $$.setBasic(EbtVoid, EvqAttribute, $1.line);
    }
    | VARYING {
        if (parseContext.globalErrorCheck($1.line, parseContext.symbolTable.atGlobalLevel(), "varying"))
            parseContext.recover();
        if (parseContext.language == EShLangVertex)
            $$.setBasic(EbtVoid, EvqVaryingOut, $1.line);
        else
            $$.setBasic(EbtVoid, EvqVaryingIn, $1.line);
    }
    | UNIFORM {
        if (parseContext.globalErrorCheck($1.line, parseContext.symbolTable.atGlobalLevel(), "uniform"))
            parseContext.recover();
        $$.setBasic(EbtVoid, EvqUniform, $1.line);
    }
    ;

type_specifier
    : type_specifier_nonarray {
        $$ = $1;
    }
    | type_specifier_nonarray LEFT_BRACKET constant_expression RIGHT_BRACKET {
        $$ = $1;
        
        if (parseContext.arrayTypeErrorCheck($2.line, $1))
            parseContext.recover();
        else {
            int size;
            if (parseContext.arraySizeErrorCheck($2.line, $3, size))
                parseContext.recover();
            $$.setArray(true, size);
        }
    }
    ;

type_specifier_nonarray
    : VOID_TYPE {
        SET_BASIC_TYPE($$,$1,EbtVoid,EbpUndefined);
    }
    | FLOAT_TYPE {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpHigh);
    }
    | HALF_TYPE {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpMedium);
    }
    | FIXED_TYPE {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpLow);
    }
    | INT_TYPE {
        SET_BASIC_TYPE($$,$1,EbtInt,EbpHigh);
    }
    | BOOL_TYPE {
        SET_BASIC_TYPE($$,$1,EbtBool,EbpHigh);
    }
    | VECTOR LEFT_ANGLE FLOAT_TYPE COMMA INTCONSTANT RIGHT_ANGLE {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( $5.i > 4 || $5.i < 1 ) {
            parseContext.error($2.line, "vector dimension out of range", "", "");
            parseContext.recover();
            $$.setBasic(EbtFloat, qual, $1.line);
        } else {
            $$.setBasic(EbtFloat, qual, $1.line);
            $$.setAggregate($5.i);
        }
    }
    | VECTOR LEFT_ANGLE INT_TYPE COMMA INTCONSTANT RIGHT_ANGLE {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( $5.i > 4 || $5.i < 1 ) {
            parseContext.error($2.line, "vector dimension out of range", "", "");
            parseContext.recover();
            $$.setBasic(EbtInt, qual, $1.line);
        } else {
            $$.setBasic(EbtInt, qual, $1.line);
            $$.setAggregate($5.i);
        }
    }
    | VECTOR LEFT_ANGLE BOOL_TYPE COMMA INTCONSTANT RIGHT_ANGLE {
        TQualifier qual = parseContext.getDefaultQualifier();
        if ( $5.i > 4 || $5.i < 1 ) {
            parseContext.error($2.line, "vector dimension out of range", "", "");
            parseContext.recover();
            $$.setBasic(EbtBool, qual, $1.line);
        } else {
            $$.setBasic(EbtBool, qual, $1.line);
            $$.setAggregate($5.i);
        }
    }
    | VEC2 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpHigh);
        $$.setAggregate(2);
    }
    | VEC3 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpHigh);
        $$.setAggregate(3);
    }
    | VEC4 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpHigh);
        $$.setAggregate(4);
    }
    | HVEC2 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpMedium);
        $$.setAggregate(2);
    }
    | HVEC3 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpMedium);
        $$.setAggregate(3);
    }
    | HVEC4 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpMedium);
        $$.setAggregate(4);
    }
    | FVEC2 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpLow);
        $$.setAggregate(2);
    }
    | FVEC3 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpLow);
        $$.setAggregate(3);
    }
    | FVEC4 {
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpLow);
        $$.setAggregate(4);
    }
    | BVEC2 {
        SET_BASIC_TYPE($$,$1,EbtBool,EbpHigh);
        $$.setAggregate(2);
    }
    | BVEC3 {
        SET_BASIC_TYPE($$,$1,EbtBool,EbpHigh);
        $$.setAggregate(3);
    }
    | BVEC4 {
        SET_BASIC_TYPE($$,$1,EbtBool,EbpHigh);
        $$.setAggregate(4);
    }
    | IVEC2 {
        SET_BASIC_TYPE($$,$1,EbtInt,EbpHigh);
        $$.setAggregate(2);
    }
    | IVEC3 {
        SET_BASIC_TYPE($$,$1,EbtInt,EbpHigh);
        $$.setAggregate(3);
    }
    | IVEC4 {
        SET_BASIC_TYPE($$,$1,EbtInt,EbpHigh);
        $$.setAggregate(4);
    }
    | MATRIX2 {
        FRAG_VERT_ONLY("mat2", $1.line); 
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpHigh);
        $$.setAggregate(2, true);
    }
    | MATRIX3 { 
        FRAG_VERT_ONLY("mat3", $1.line); 
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpHigh);
        $$.setAggregate(3, true);
    }
    | MATRIX4 { 
        FRAG_VERT_ONLY("mat4", $1.line);
        SET_BASIC_TYPE($$,$1,EbtFloat,EbpHigh);
        $$.setAggregate(4, true);
    }
	| HMATRIX2 {
		FRAG_VERT_ONLY("mat2", $1.line);
		SET_BASIC_TYPE($$,$1,EbtFloat,EbpMedium);
		$$.setAggregate(2, true);
	}
	| HMATRIX3 { 
		FRAG_VERT_ONLY("mat3", $1.line);
		SET_BASIC_TYPE($$,$1,EbtFloat,EbpMedium);
		$$.setAggregate(3, true);
	}
	| HMATRIX4 { 
		FRAG_VERT_ONLY("mat4", $1.line);
		SET_BASIC_TYPE($$,$1,EbtFloat,EbpMedium);
		$$.setAggregate(4, true);
	}
	| FMATRIX2 {
		FRAG_VERT_ONLY("mat2", $1.line);
		SET_BASIC_TYPE($$,$1,EbtFloat,EbpLow);
		$$.setAggregate(2, true);
	}
	| FMATRIX3 { 
		FRAG_VERT_ONLY("mat3", $1.line);
		SET_BASIC_TYPE($$,$1,EbtFloat,EbpLow);
		$$.setAggregate(3, true);
	}
	| FMATRIX4 { 
		FRAG_VERT_ONLY("mat4", $1.line);
		SET_BASIC_TYPE($$,$1,EbtFloat,EbpLow);
		$$.setAggregate(4, true);
	}
	| TEXTURE {
        FRAG_VERT_ONLY("texture", $1.line);
        SET_BASIC_TYPE($$,$1,EbtTexture,EbpUndefined);
    }  
    | SAMPLERGENERIC {
        FRAG_VERT_ONLY("sampler", $1.line);
        SET_BASIC_TYPE($$,$1,EbtSamplerGeneric,EbpUndefined);
    }  
    | SAMPLER1D {
        FRAG_VERT_ONLY("sampler1D", $1.line);
        SET_BASIC_TYPE($$,$1,EbtSampler1D,EbpUndefined);
    } 
    | SAMPLER2D {
        FRAG_VERT_ONLY("sampler2D", $1.line);
        SET_BASIC_TYPE($$,$1,EbtSampler2D,EbpUndefined);
    } 
    | SAMPLER3D {
        FRAG_VERT_ONLY("sampler3D", $1.line);
        SET_BASIC_TYPE($$,$1,EbtSampler3D,EbpUndefined);
    } 
    | SAMPLERCUBE {
        FRAG_VERT_ONLY("samplerCube", $1.line);
        SET_BASIC_TYPE($$,$1,EbtSamplerCube,EbpUndefined);
    } 
    | SAMPLERRECT {
        FRAG_VERT_ONLY("samplerRECT", $1.line);
        SET_BASIC_TYPE($$,$1,EbtSamplerRect,EbpUndefined);
    } 
    | SAMPLER1DSHADOW {
        FRAG_VERT_ONLY("sampler1DShadow", $1.line);
        SET_BASIC_TYPE($$,$1,EbtSampler1DShadow,EbpUndefined);
    } 
    | SAMPLER2DSHADOW {
        FRAG_VERT_ONLY("sampler2DShadow", $1.line);
        SET_BASIC_TYPE($$,$1,EbtSampler2DShadow,EbpUndefined);
    }     
    | struct_specifier {
        FRAG_VERT_ONLY("struct", $1.line);
        $$ = $1;
        $$.qualifier = parseContext.getDefaultQualifier();
    }
    | TYPE_NAME {     
        //
        // This is for user defined type names.  The lexical phase looked up the 
        // type.
        //
        TType& structure = static_cast<TVariable*>($1.symbol)->getType();
        SET_BASIC_TYPE($$,$1,EbtStruct,EbpUndefined);
        $$.userDef = &structure;
    }
    ;

struct_specifier
    : STRUCT IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE {
        TType* structure = new TType($4, *$2.string, EbpUndefined, $2.line);
        TVariable* userTypeDef = new TVariable($2.string, *structure, true);
        if (! parseContext.symbolTable.insert(*userTypeDef)) {
            parseContext.error($2.line, "redefinition", $2.string->c_str(), "struct");
            parseContext.recover();
        }
        $$.setBasic(EbtStruct, EvqTemporary, $1.line);
        $$.userDef = structure;
    }
    | STRUCT LEFT_BRACE struct_declaration_list RIGHT_BRACE {
        TType* structure = new TType($3, TString(""), EbpUndefined, $1.line);
        $$.setBasic(EbtStruct, EvqTemporary, $1.line);
        $$.userDef = structure;
    }
    ;
    
struct_declaration_list
    : struct_declaration {
        $$ = $1;
    }
    | struct_declaration_list struct_declaration {
        $$ = $1;
        for (unsigned int i = 0; i < $2->size(); ++i) {
            for (unsigned int j = 0; j < $$->size(); ++j) {
                if ((*$$)[j].type->getFieldName() == (*$2)[i].type->getFieldName()) {
                    parseContext.error((*$2)[i].line, "duplicate field name in structure:", "struct", (*$2)[i].type->getFieldName().c_str());
                    parseContext.recover();
                }
            }
            $$->push_back((*$2)[i]);
        }
    }
    ;
    
struct_declaration
    : type_specifier struct_declarator_list SEMICOLON {
        $$ = $2;
        
        if (parseContext.voidErrorCheck($1.line, (*$2)[0].type->getFieldName(), $1)) {
            parseContext.recover();
        }
        for (unsigned int i = 0; i < $$->size(); ++i) {
            //
            // Careful not to replace already know aspects of type, like array-ness
            //
            TType* type = (*$$)[i].type;
            type->setBasicType($1.type);
            type->setPrecision($1.precision);
            type->setNominalSize($1.size);
            type->setMatrix($1.matrix);
            
            // don't allow arrays of arrays
            if (type->isArray()) {
                if (parseContext.arrayTypeErrorCheck($1.line, $1))
                    parseContext.recover();
            }
            if ($1.array)
                type->setArraySize($1.arraySize);
            if ($1.userDef) {
                type->setStruct($1.userDef->getStruct());
                type->setTypeName($1.userDef->getTypeName());
            }
        }
    }
    ;
        
struct_declarator_list
    : struct_declarator {
        $$ = NewPoolTTypeList();
        $$->push_back($1);
    }
    | struct_declarator_list COMMA struct_declarator {
        $$->push_back($3);
    }
    ;
    
struct_declarator
    : IDENTIFIER {
        $$.type = new TType(EbtVoid, EbpUndefined);
        $$.line = $1.line;
        $$.type->setFieldName(*$1.string);
    }
    | IDENTIFIER COLON IDENTIFIER {
        $$.type = new TType(EbtVoid, EbpUndefined);
        $$.line = $1.line;
        $$.type->setFieldName(*$1.string);
        $$.type->setSemantic(*$3.string);
    }
    | IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET {
        $$.type = new TType(EbtVoid, EbpUndefined);
        $$.line = $1.line;
        $$.type->setFieldName(*$1.string);
        
        int size;
        if (parseContext.arraySizeErrorCheck($2.line, $3, size))
            parseContext.recover();
        $$.type->setArraySize(size);
    }
    | IDENTIFIER LEFT_BRACKET constant_expression RIGHT_BRACKET COLON IDENTIFIER {
        $$.type = new TType(EbtVoid, EbpUndefined);
        $$.line = $1.line;
        $$.type->setFieldName(*$1.string);
        
        int size;
        if (parseContext.arraySizeErrorCheck($2.line, $3, size))
            parseContext.recover();
        $$.type->setArraySize(size);
        $$.type->setSemantic(*$6.string);
    }
    ;
    


initializer
    : assignment_expression { $$ = $1; }
    | initialization_list { $$ = $1; }
    | sampler_initializer { $$ = $1; }
    ;

declaration_statement 
    : declaration { $$ = $1; }
    ;

statement
    : compound_statement  { $$ = $1; }
    | simple_statement    { $$ = $1; }
    ;

// Grammar Note:  No labeled statements; 'goto' is not supported.

simple_statement 
    : declaration_statement { $$ = $1; }
    | expression_statement  { $$ = $1; } 
    | selection_statement   { $$ = $1; }
    | iteration_statement   { $$ = $1; }
    | jump_statement        { $$ = $1; }
    ;

compound_statement
    : LEFT_BRACE RIGHT_BRACE { $$ = 0; }
    | LEFT_BRACE { parseContext.symbolTable.push(); } statement_list { parseContext.symbolTable.pop(); } RIGHT_BRACE {
        if ($3 != 0)            
            $3->setOperator(EOpSequence); 
        $$ = $3;
    }
    ;

statement_no_new_scope 
    : compound_statement_no_new_scope { $$ = $1; }
    | simple_statement                { $$ = $1; }
    ;

compound_statement_no_new_scope 
    // Statement that doesn't create a new scope, for selection_statement, iteration_statement 
    : LEFT_BRACE RIGHT_BRACE { 
        $$ = 0; 
    }
    | LEFT_BRACE statement_list RIGHT_BRACE { 
        if ($2)
            $2->setOperator(EOpSequence); 
        $$ = $2; 
    }
    ;

statement_list
    : statement {
        $$ = parseContext.intermediate.makeAggregate($1, gNullSourceLoc); 
    }
    | statement_list statement { 
        $$ = parseContext.intermediate.growAggregate($1, $2, gNullSourceLoc);
    }
    ;

expression_statement
    : SEMICOLON  { $$ = 0; }
    | expression SEMICOLON  { $$ = static_cast<TIntermNode*>($1); }
    ;

selection_statement
    : IF LEFT_PAREN expression RIGHT_PAREN selection_rest_statement { 
        if (parseContext.boolErrorCheck($1.line, $3))
            parseContext.recover();
        $$ = parseContext.intermediate.addSelection($3, $5, $1.line);
    }
    ;

selection_rest_statement 
    : statement ELSE statement {
        $$.node1 = $1;
        $$.node2 = $3;
    }
    | statement { 
        $$.node1 = $1;
        $$.node2 = 0;
    }
    ;

// Grammar Note:  No 'switch'.  Switch statements not supported.

condition
    // In 1996 c++ draft, conditions can include single declarations 
    : expression {
        $$ = $1;
        if (parseContext.boolErrorCheck($1->getLine(), $1))
            parseContext.recover();          
    }
    | fully_specified_type IDENTIFIER EQUAL initializer {
        TIntermNode* intermNode;
        if (parseContext.structQualifierErrorCheck($2.line, $1))
            parseContext.recover();
        if (parseContext.boolErrorCheck($2.line, $1))
            parseContext.recover();
        
        if (!parseContext.executeInitializer($2.line, *$2.string, $1, $4, intermNode))
            $$ = $4;
        else {
            parseContext.recover();
            $$ = 0;
        }
    }
    ;

iteration_statement
    : WHILE LEFT_PAREN { parseContext.symbolTable.push(); ++parseContext.loopNestingLevel; } condition RIGHT_PAREN statement_no_new_scope { 
        parseContext.symbolTable.pop();
        $$ = parseContext.intermediate.addLoop($6, $4, 0, true, $1.line);
        --parseContext.loopNestingLevel;
    }
    | DO { ++parseContext.loopNestingLevel; } statement WHILE LEFT_PAREN expression RIGHT_PAREN SEMICOLON {
        if (parseContext.boolErrorCheck($8.line, $6))
            parseContext.recover();
                    
        $$ = parseContext.intermediate.addLoop($3, $6, 0, false, $4.line);
        --parseContext.loopNestingLevel;
    }
    | FOR LEFT_PAREN { parseContext.symbolTable.push(); ++parseContext.loopNestingLevel; } for_init_statement for_rest_statement RIGHT_PAREN statement_no_new_scope {
        parseContext.symbolTable.pop();
        $$ = parseContext.intermediate.makeAggregate($4, $2.line);
        $$ = parseContext.intermediate.growAggregate(
                $$,
                parseContext.intermediate.addLoop($7, reinterpret_cast<TIntermTyped*>($5.node1), reinterpret_cast<TIntermTyped*>($5.node2), true, $1.line),
                $1.line);
        $$->getAsAggregate()->setOperator(EOpSequence);
        --parseContext.loopNestingLevel;
    }
    ;

for_init_statement 
    : expression_statement {
        $$ = $1; 
    } 
    | declaration_statement {
        $$ = $1;
    }
    ;

conditionopt 
    : condition { 
        $$ = $1; 
    }
    | /* May be null */ { 
        $$ = 0; 
    }
    ;

for_rest_statement 
    : conditionopt SEMICOLON { 
        $$.node1 = $1;
        $$.node2 = 0;
    }
    | conditionopt SEMICOLON expression  {
        $$.node1 = $1;
        $$.node2 = $3;
    }
    ;

jump_statement
    : CONTINUE SEMICOLON {
        if (parseContext.loopNestingLevel <= 0) {
            parseContext.error($1.line, "continue statement only allowed in loops", "", "");
            parseContext.recover();
        }        
        $$ = parseContext.intermediate.addBranch(EOpContinue, $1.line);
    }
    | BREAK SEMICOLON {
        if (parseContext.loopNestingLevel <= 0) {
            parseContext.error($1.line, "break statement only allowed in loops", "", "");
            parseContext.recover();
        }        
        $$ = parseContext.intermediate.addBranch(EOpBreak, $1.line);
    }
    | RETURN SEMICOLON {
        $$ = parseContext.intermediate.addBranch(EOpReturn, $1.line);
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid) {
            parseContext.error($1.line, "non-void function must return a value", "return", "");
            parseContext.recover();
        }
    }
    | RETURN expression SEMICOLON {        
        TIntermTyped *temp = $2;
        if (parseContext.currentFunctionType->getBasicType() == EbtVoid) {
            parseContext.error($1.line, "void function cannot return a value", "return", "");
            parseContext.recover();
        } else if (*(parseContext.currentFunctionType) != $2->getType()) {
            TOperator op = parseContext.getConstructorOp( *(parseContext.currentFunctionType));
            if ( op != EOpNull)
                temp = parseContext.constructBuiltIn( (parseContext.currentFunctionType), op, $2, $1.line, false);
            else
                temp = 0;
            if ( temp == 0) {
                parseContext.error($1.line, "function return is not matching type:", "return", "");
                parseContext.recover();
                temp = $2;
            }
        }
        $$ = parseContext.intermediate.addBranch(EOpReturn, temp, $1.line);
        parseContext.functionReturnsValue = true;
    }
    | DISCARD SEMICOLON {
        FRAG_ONLY("discard", $1.line);
        $$ = parseContext.intermediate.addBranch(EOpKill, $1.line);
    }        
    ;

// Grammar Note:  No 'goto'.  Gotos are not supported.

translation_unit
    : external_declaration { 
        $$ = $1; 
        parseContext.treeRoot = $$; 
    }
    | translation_unit external_declaration {
        $$ = parseContext.intermediate.growAggregate($1, $2, gNullSourceLoc);
        parseContext.treeRoot = $$;
    }
    ;

external_declaration
    : function_definition { 
        $$ = $1; 
    }
    | declaration { 
        $$ = $1; 
    }
    | SEMICOLON { $$ = 0; }
    ;

function_definition
    : function_prototype {
        TFunction& function = *($1.function);
        TFunction* prevDec = static_cast<TFunction*>(parseContext.symbolTable.find(function.getMangledName()));
        //
        // Note:  'prevDec' could be 'function' if this is the first time we've seen function
        // as it would have just been put in the symbol table.  Otherwise, we're looking up
        // an earlier occurance.
        //
        if (prevDec->isDefined()) {
            //
            // Then this function already has a body.
            //
            parseContext.error($1.line, "function already has a body", function.getName().c_str(), "");
            parseContext.recover();
        }
        prevDec->setDefined();
        
        //
        // Raise error message if main function takes any parameters or return anything other than void
        //
        /* TODO: this is unneeded for HLSL, but it might need to name mangle 
        if (function.getName() == "main") {
            if (function.getParamCount() > 0) {
                parseContext.error($1.line, "function cannot take any parameter(s)", function.getName().c_str(), "");
                parseContext.recover();
            }
            if (function.getReturnType().getBasicType() != EbtVoid) {
                parseContext.error($1.line, "", function.getReturnType().getBasicString(), "main function cannot return a value");
                parseContext.recover();
            }            
        }
        */
   
        //
        // New symbol table scope for body of function plus its arguments
        //
        parseContext.symbolTable.push();
        
        //
        // Remember the return type for later checking for RETURN statements.
        //
        parseContext.currentFunctionType = &(prevDec->getReturnType());
        parseContext.functionReturnsValue = false;
        
        // 
        // Insert parameters into the symbol table.
        // If the parameter has no name, it's not an error, just don't insert it 
        // (could be used for unused args).
        //
        // Also, accumulate the list of parameters into the HIL, so lower level code
        // knows where to find parameters.
        //
        TIntermAggregate* paramNodes = new TIntermAggregate;
        for (int i = 0; i < function.getParamCount(); i++) {
            TParameter& param = function[i];
            if (param.name != 0) {
                TVariable *variable = new TVariable(param.name, param.info, *param.type);
                // 
                // Insert the parameters with name in the symbol table.
                //
                if (! parseContext.symbolTable.insert(*variable)) {
                    parseContext.error($1.line, "redefinition", variable->getName().c_str(), "");
                    parseContext.recover();
                    delete variable;
                }
                //
                // Transfer ownership of name pointer to symbol table.
                //
                param.name = 0;
                
                //
                // Add the parameter to the HIL
                //                
                paramNodes = parseContext.intermediate.growAggregate(
                                               paramNodes, 
                                               parseContext.intermediate.addSymbol(variable->getUniqueId(),
                                                                       variable->getName(),
                                                                       variable->getInfo(),
                                                                       variable->getType(), $1.line), 
                                               $1.line);
            } else {
                paramNodes = parseContext.intermediate.growAggregate(paramNodes, parseContext.intermediate.addSymbol(0, "", param.info, *param.type, $1.line), $1.line);
            }
        }
        parseContext.intermediate.setAggregateOperator(paramNodes, EOpParameters, $1.line);
        $1.intermAggregate = paramNodes;
        parseContext.loopNestingLevel = 0;
    }
    compound_statement_no_new_scope {
        //?? Check that all paths return a value if return type != void ?
        //   May be best done as post process phase on intermediate code
        if (parseContext.currentFunctionType->getBasicType() != EbtVoid && ! parseContext.functionReturnsValue) {
            parseContext.error($1.line, "function does not return a value:", "", $1.function->getName().c_str());
            parseContext.recover();
        }
        parseContext.symbolTable.pop();
        $$ = parseContext.intermediate.growAggregate($1.intermAggregate, $3, gNullSourceLoc);
        parseContext.intermediate.setAggregateOperator($$, EOpFunction, $1.line);
        $$->getAsAggregate()->setName($1.function->getMangledName().c_str());
        $$->getAsAggregate()->setPlainName($1.function->getName().c_str());
        $$->getAsAggregate()->setType($1.function->getReturnType());
        
	if ( $1.function->getInfo())
	    $$->getAsAggregate()->setSemantic($1.function->getInfo()->getSemantic());
    }
    ;
   
initialization_list
    : LEFT_BRACE initializer_list RIGHT_BRACE {
       $$ = $2;
    }
    | LEFT_BRACE initializer_list COMMA RIGHT_BRACE {
       $$ = $2;
    }
    ;

    
initializer_list
    : assignment_expression {
        //create a new aggNode
       $$ = parseContext.intermediate.makeAggregate( $1, $1->getLine());       
    }
    | initialization_list {
       //take the inherited aggNode and return it
       $$ = $1->getAsAggregate();       
    }
    | initializer_list COMMA assignment_expression {
        // append to the aggNode
       $$ = parseContext.intermediate.growAggregate( $1, $3, $3->getLine());       
    }
    | initializer_list COMMA initialization_list {
       // append all children or $3 to $1 
       $$ = parseContext.mergeAggregates( $1, $3->getAsAggregate());
    }
    ;

annotation
    : LEFT_ANGLE RIGHT_ANGLE {
        //empty annotation
	  $$ = 0;
    }
    | LEFT_ANGLE annotation_list RIGHT_ANGLE {
      $$ = $2;
    }
    ; 

annotation_list
    : annotation_item {
        $$ = new TAnnotation;
	$$->addKey( *$1.string);
    }
    | annotation_list annotation_item {
        $1->addKey( *$2.string);
	$$ = $1;
    }
    ;

annotation_item
    : ann_type IDENTIFIER EQUAL ann_literal SEMICOLON {
        $$.string = $2.string;
    }
    ;

ann_type
    : FLOAT_TYPE {}
    | HALF_TYPE {}
    | FIXED_TYPE {}
    | INT_TYPE {}
    | BOOL_TYPE {}
    | STRING_TYPE {}
    | BVEC2 {}
    | BVEC3 {}
    | BVEC4 {}
    | IVEC2 {}
    | IVEC3 {}
    | IVEC4 {}
    | VEC2 {}
    | VEC3 {}
    | VEC4 {}
    | HVEC2 {}
    | HVEC3 {}
    | HVEC4 {}
    | FVEC2 {}
    | FVEC3 {}
    | FVEC4 {}
    ;

ann_literal
: ann_numerical_constant {}
| STRINGCONSTANT {}
| ann_literal_constructor {}
| ann_literal_init_list {}
;

ann_numerical_constant
: INTCONSTANT {
    $$.f = (float)$1.i;
}
| BOOLCONSTANT {
    $$.f = ($1.b) ? 1.0f : 0.0f;
}
| FLOATCONSTANT {
    $$ = $1;
}
;

ann_literal_constructor
: ann_type LEFT_PAREN ann_value_list RIGHT_PAREN {}
;

ann_value_list
: ann_numerical_constant {}
| ann_value_list COMMA ann_numerical_constant {}
;

ann_literal_init_list
: LEFT_BRACE ann_value_list RIGHT_BRACE {}
;

register_specifier
    : COLON REGISTER LEFT_PAREN IDENTIFIER RIGHT_PAREN {
        // This is being thrown away
    }
    ;

semantic
: COLON IDENTIFIER { $$.string = $2.string;}
;

type_info
: { $$ = 0;}
| semantic { $$ = new TTypeInfo( *$1.string, 0); }
| register_specifier { $$ = 0; }
| annotation { $$ = new TTypeInfo( "", $1); }
| semantic annotation { $$ = new TTypeInfo( *$1.string, $2); }
| semantic register_specifier { $$ = new TTypeInfo( *$1.string, 0); }
| register_specifier annotation { $$ = new TTypeInfo( "", $2); }
| semantic register_specifier annotation { $$ = new TTypeInfo( *$1.string, $3); }
;

sampler_initializer
: SAMPLERSTATE LEFT_BRACE sampler_init_list RIGHT_BRACE {
    constUnion *cUnion = new constUnion[1];
    cUnion[0].setFConst(0.0f);
    $$ = parseContext.intermediate.addConstantUnion( cUnion, TType(EbtFloat, EbpUndefined, EvqConst, 1), $1.line);
}
| SAMPLERSTATE LEFT_BRACE RIGHT_BRACE {
}
;

sampler_init_list
: sampler_init_item {
}
| sampler_init_list sampler_init_item {
}
;

sampler_init_item
: IDENTIFIER EQUAL IDENTIFIER SEMICOLON {}
| IDENTIFIER EQUAL LEFT_ANGLE IDENTIFIER RIGHT_ANGLE SEMICOLON {}
| IDENTIFIER EQUAL LEFT_PAREN IDENTIFIER RIGHT_PAREN SEMICOLON {}
| TEXTURE EQUAL IDENTIFIER SEMICOLON {}
| TEXTURE EQUAL LEFT_ANGLE IDENTIFIER RIGHT_ANGLE SEMICOLON {}
| TEXTURE EQUAL LEFT_PAREN IDENTIFIER RIGHT_PAREN SEMICOLON {}
;

%%
