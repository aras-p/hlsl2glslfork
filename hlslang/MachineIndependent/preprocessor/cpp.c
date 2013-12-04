// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "slglobals.h"

const TSourceLoc gNullSourceLoc = { NULL, 0 };

static int CPPif(yystypepp * yylvalpp);

/* Don't use memory.c's replacements, as we clean up properly here */
#undef malloc
#undef free

static int defineAtom = 0;
static int definedAtom = 0;
static int elseAtom = 0;
static int elifAtom = 0;
static int endifAtom = 0;
static int ifAtom = 0;
static int ifdefAtom = 0;
static int ifndefAtom = 0;
static int includeAtom = 0;
static int lineAtom = 0;
static int pragmaAtom = 0;
static int undefAtom = 0;
static int errorAtom = 0;
static int __LINE__Atom = 0;
static int __FILE__Atom = 0;

static Scope *macros = 0;
#define MAX_MACRO_ARGS  64
#define MAX_IF_NESTING  64


int InitCPP(void)
{
    // Add various atoms needed by the CPP line scanner:
    defineAtom = LookUpAddString(atable, "define");
    definedAtom = LookUpAddString(atable, "defined");
    elifAtom = LookUpAddString(atable, "elif");
    elseAtom = LookUpAddString(atable, "else");
    endifAtom = LookUpAddString(atable, "endif");
    ifAtom = LookUpAddString(atable, "if");
    ifdefAtom = LookUpAddString(atable, "ifdef");
    ifndefAtom = LookUpAddString(atable, "ifndef");
    includeAtom = LookUpAddString(atable, "include");
    lineAtom = LookUpAddString(atable, "line");
    pragmaAtom = LookUpAddString(atable, "pragma");
    undefAtom = LookUpAddString(atable, "undef");
    errorAtom = LookUpAddString(atable, "error");
    __LINE__Atom = LookUpAddString(atable, "__LINE__");
    __FILE__Atom = LookUpAddString(atable, "__FILE__");
    macros = NewScopeInPool(mem_CreatePool(0, 0));
    return 1;
} // InitCPP

int FreeCPP(void)
{
    if (macros)
    {
        mem_FreePool(macros->pool);
        macros = 0;
    }
	
    return 1;
}

int FinalCPP(void)
{
    if (cpp->ifdepth)
        CPPErrorToInfoLog("#if mismatch");
    return 1;
}

static int CPPdefine(yystypepp * yylvalpp)
{
    int token, name, args[MAX_MACRO_ARGS], argc;
    const char *message;
    MacroSymbol mac;
    Symbol *symb;
    SourceLoc dummyLoc;
    memset(&mac, 0, sizeof(mac));
    memset(&dummyLoc, 0, sizeof(dummyLoc));
    token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    if (token != CPP_IDENTIFIER) {
        CPPErrorToInfoLog("#define");
        return token;
    }
    name = yylvalpp->sc_ident;
    token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    if (token == '(' && !yylvalpp->sc_int) {
        // gather arguments
        argc = 0;
        do {
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            if (argc == 0 && token == ')') break;
            if (token != CPP_IDENTIFIER) {
                CPPErrorToInfoLog("#define");
                return token;
            }
            if (argc < MAX_MACRO_ARGS)
                args[argc++] = yylvalpp->sc_ident;
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
        } while (token == ',');
        if (token != ')') {
            CPPErrorToInfoLog("#define");
            return token;
        }
        mac.argc = argc;
        mac.args = mem_Alloc(macros->pool, argc * sizeof(int));
        memcpy(mac.args, args, argc * sizeof(int));
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    }
    mac.body = NewTokenStream(GetAtomString(atable, name), macros->pool);
    while (token != '\n') {
        while (token == '\\') {
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            if (token == '\n')
                token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            else
                RecordToken(mac.body, '\\', yylvalpp);
        }
        RecordToken(mac.body, token, yylvalpp);
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    };
	
    symb = LookUpSymbol(macros, name);
    if (symb) {
        if (!symb->mac.undef) {
            // already defined -- need to make sure they are identical
            if (symb->mac.argc != mac.argc) goto error;
            for (argc=0; argc < mac.argc; argc++)
                if (symb->mac.args[argc] != mac.args[argc])
                    goto error;
            RewindTokenStream(symb->mac.body);
            RewindTokenStream(mac.body);
            do {
                int old_lval, old_token;
                old_token = ReadToken(symb->mac.body, yylvalpp);
                old_lval = yylvalpp->sc_int;
                token = ReadToken(mac.body, yylvalpp);
                if (token != old_token || yylvalpp->sc_int != old_lval) {
                error:
                    StoreStr("Macro Redefined");
                    StoreStr(GetStringOfAtom(atable,name));
                    message=GetStrfromTStr();
                    DecLineNumber();
                    CPPShInfoLogMsg(message);
                    IncLineNumber();
                    ResetTString();
                    break; }
            } while (token > 0);
        }
    } else {
        symb = AddSymbol(&dummyLoc, macros, name);
    }
    symb->mac = mac;
    return '\n';
} // CPPdefine

static int CPPundef(yystypepp * yylvalpp)
{
    int token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    Symbol *symb;
    if(token == '\n'){
        CPPErrorToInfoLog("#undef");
        return token;
    }
    if (token != CPP_IDENTIFIER)
		goto error;
    symb = LookUpSymbol(macros, yylvalpp->sc_ident);
    if (symb) {
        symb->mac.undef = 1;
    }
    token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    if (token != '\n') {
    error:
        CPPErrorToInfoLog("#undef");
    }
    return token;
} // CPPundef

static int CPPline(yystypepp * yylvalpp);

/*
 * CPPgotoEndOfIfBlock: jump to matching #else, #elif or #endif
 * for the current #if(def). if matchelse if false, ignore #elif
 * and #else and jump directly to #endif
 */

static int CPPgotoEndOfIfBlock(int matchelse, yystypepp * yylvalpp)
{
    int atom;
    int token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
	
    while (token > 0) {
        // jump to next line beginning with a #
        if (token != '#') {
            while (token != '\n')
                token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
			
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            continue;
        }
        if ((token = cpp->currentInput->scan(cpp->currentInput, yylvalpp)) != CPP_IDENTIFIER)
            continue;
		
        atom = yylvalpp->sc_ident;
        if (atom == ifAtom || atom == ifdefAtom || atom == ifndefAtom){
            // recursivelly ignore this imbricated if(def) block
            CPPgotoEndOfIfBlock(0, yylvalpp);
            assert(yylvalpp->sc_ident == endifAtom);
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            if (token != '\n') {
                CPPWarningToInfoLog("unexpected tokens following #else preprocessor directive - expected a newline");
                while (token != '\n')
                    token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            }
            continue;
        }
        else if (atom == endifAtom)
            break;
        else if (matchelse && (atom == elseAtom || atom == elifAtom))
            break;
        else if (atom == lineAtom)
			token = CPPline(yylvalpp);
    }
    return token;
}

enum eval_prec {
    MIN_PREC,
    COND, LOGOR, LOGAND, OR, XOR, AND, EQUAL, RELATION, SHIFT, ADD, MUL, UNARY,
    MAX_PREC
};

static int op_logor(int a, int b) { return a || b; }
static int op_logand(int a, int b) { return a && b; }
static int op_or(int a, int b) { return a | b; }
static int op_xor(int a, int b) { return a ^ b; }
static int op_and(int a, int b) { return a & b; }
static int op_eq(int a, int b) { return a == b; }
static int op_ne(int a, int b) { return a != b; }
static int op_ge(int a, int b) { return a >= b; }
static int op_le(int a, int b) { return a <= b; }
static int op_gt(int a, int b) { return a > b; }
static int op_lt(int a, int b) { return a < b; }
static int op_shl(int a, int b) { return a << b; }
static int op_shr(int a, int b) { return a >> b; }
static int op_add(int a, int b) { return a + b; }
static int op_sub(int a, int b) { return a - b; }
static int op_mul(int a, int b) { return a * b; }
static int op_div(int a, int b) { return a / b; }
static int op_mod(int a, int b) { return a % b; }
static int op_pos(int a) { return a; }
static int op_neg(int a) { return -a; }
static int op_cmpl(int a) { return ~a; }
static int op_not(int a) { return !a; }

struct {
    int token, prec, (*op)(int, int);
} binop[] = {
    { CPP_OR_OP, LOGOR, op_logor },
    { CPP_AND_OP, LOGAND, op_logand },
    { '|', OR, op_or },
    { '^', XOR, op_xor },
    { '&', AND, op_and },
    { CPP_EQ_OP, EQUAL, op_eq },
    { CPP_NE_OP, EQUAL, op_ne },
    { '>', RELATION, op_gt },
    { CPP_GE_OP, RELATION, op_ge },
    { '<', RELATION, op_lt },
    { CPP_LE_OP, RELATION, op_le },
    { CPP_LEFT_OP, SHIFT, op_shl },
    { CPP_RIGHT_OP, SHIFT, op_shr },
    { '+', ADD, op_add },
    { '-', ADD, op_sub },
    { '*', MUL, op_mul },
    { '/', MUL, op_div },
    { '%', MUL, op_mod },
};

struct {
    int token, (*op)(int);
} unop[] = {
    { '+', op_pos },
    { '-', op_neg },
    { '~', op_cmpl },
    { '!', op_not },
};

#define ALEN(A) (sizeof(A)/sizeof(A[0]))

static int eval(int token, int prec, int *res, int *err, yystypepp * yylvalpp)
{
    int         i, val;
    Symbol      *s;
    if (token == CPP_IDENTIFIER) {
        if (yylvalpp->sc_ident == definedAtom) {
            int needclose = 0;
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            if (token == '(') {
                needclose = 1;
                token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            }
            if (token != CPP_IDENTIFIER)
                goto error;
            *res = (s = LookUpSymbol(macros, yylvalpp->sc_ident))
			? !s->mac.undef : 0;
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            if (needclose) {
                if (token != ')')
                    goto error;
                token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            }
        } else if (MacroExpand(yylvalpp->sc_ident, yylvalpp)) {
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            return eval(token, prec, res, err, yylvalpp);
        } else {
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            *res = 0;
            *err = 0;
			
        }
    } else if (token == CPP_INTCONSTANT) {
        *res = yylvalpp->sc_int;
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    } else if (token == '(') {
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
        token = eval(token, MIN_PREC, res, err, yylvalpp);
        if (!*err) {
            if (token != ')')
                goto error;
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
        }
    } else {
        for (i = ALEN(unop) - 1; i >= 0; i--) {
            if (unop[i].token == token)
                break;
        }
        if (i >= 0) {
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            token = eval(token, UNARY, res, err, yylvalpp);
            *res = unop[i].op(*res);
        } else {
            goto error;
        }
    }
    while (!*err) {
        if (token == ')' || token == '\n') break;
        for (i = ALEN(binop) - 1; i >= 0; i--) {
            if (binop[i].token == token)
                break;
        }
        if (i < 0 || binop[i].prec <= prec)
            break;
        val = *res;
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
        token = eval(token, binop[i].prec, res, err, yylvalpp);
        *res = binop[i].op(val, *res);
    }
    return token;
error:
    CPPErrorToInfoLog("incorrect preprocessor directive");
    *err = 1;
    *res = 0;
    return token;
} // eval

static int CPPelse(yystypepp* yylvalpp)
{
    int token = CPPgotoEndOfIfBlock(1, yylvalpp);
    if (yylvalpp->sc_ident == endifAtom)
    {
        cpp->ifdepth--;
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    }
    else if (yylvalpp->sc_ident == elifAtom)
    {
        cpp->ifdepth--; // will be reincremented by CPPif
        token = CPPif(yylvalpp);
    }
    else if (yylvalpp->sc_ident == elseAtom)
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    else
    {
        CPPErrorToInfoLog("unclosed #if block");
        return 0;
    }
	return token;
}

static int CPPif(yystypepp * yylvalpp) {
    int token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    int res = 0, err = 0;
    cpp->ifdepth++;
    if(cpp->ifdepth >MAX_IF_NESTING){
        CPPErrorToInfoLog("max #if nesting depth exceeded");
        return 0;
    }
    token = eval(token, MIN_PREC, &res, &err, yylvalpp);
    if (token != '\n') {
        CPPWarningToInfoLog("unexpected tokens following the preprocessor directive - expected a newline");
        while (token != '\n')
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    }
    if (!res && !err) {
        token = CPPelse(yylvalpp);
    }
	
    return token;
} // CPPif

static int CPPifdef(int defined, yystypepp * yylvalpp)
{
    int token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    int name = yylvalpp->sc_ident;
    if(++cpp->ifdepth >MAX_IF_NESTING){
        CPPErrorToInfoLog("max #if nesting depth exceeded");
        return 0;
    }
    if (token != CPP_IDENTIFIER) {
		defined ? CPPErrorToInfoLog("ifdef"):CPPErrorToInfoLog("ifndef");
    } else {
        Symbol *s = LookUpSymbol(macros, name);
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
        if (token != '\n') {
            CPPWarningToInfoLog("unexpected tokens following #ifdef preprocessor directive - expected a newline");
            while (token != '\n')
                token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
        }
        if (((s && !s->mac.undef) ? 1 : 0) != defined)
        {
            token = CPPelse(yylvalpp);
        }
    }
    return token;
} // CPPifdef

static int CPPline(yystypepp * yylvalpp)
{
    int token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    if(token=='\n'){
        DecLineNumber();
        CPPErrorToInfoLog("#line");
        IncLineNumber();
        return token;
    }
    else if (token == CPP_INTCONSTANT) {
        TSourceLoc line;
        line.line=atoi(yylvalpp->symbol_name);
        line.file=NULL;
        
        // NOTE: we deliberate deferr calling SetLineNumber until after we've scanned
        // the rest of the line so that the new line-number is for the "next" line 
        // parsed (consistent with other preprocessors including GPP).
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
		
		// Modified for HLSL, which allows #line X "str", whereas GLSL is #line X Y
        if (token == CPP_STRCONSTANT) {
            line.file = GetStringOfAtom(atable,yylvalpp->sc_ident);
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            if(token=='\n')
                SetLineNumber(line);
            else
                CPPErrorToInfoLog("#line");
        }
        else if (token == '\n'){
            SetLineNumber(line);
            return token;
        }
        else{
            CPPErrorToInfoLog("#line");
        }
    }
    else{
		CPPErrorToInfoLog("#line");
    }
    return token;
}

static int CPPerror(yystypepp * yylvalpp) {
	
    int token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    const char *message;
	
    while (token != '\n') {
        if (token == CPP_FLOATCONSTANT || token == CPP_INTCONSTANT){
            StoreStr(yylvalpp->symbol_name);
        }else if(token == CPP_IDENTIFIER || token == CPP_STRCONSTANT){
            StoreStr(GetStringOfAtom(atable,yylvalpp->sc_ident));
        }else {
            StoreStr(GetStringOfAtom(atable,token));
        }
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    }
    DecLineNumber();
    //store this msg into the shader's information log..set the Compile Error flag!!!!
    message=GetStrfromTStr();
    CPPShInfoLogMsg(message);
    ResetTString();
    cpp->CompileError=1;
    IncLineNumber();
    return '\n';
}//CPPerror

static int CPPpragma(yystypepp * yylvalpp)
{
    char SrcStrName[2];
    char** allTokens;
    size_t tokenCount = 0;
    size_t maxTokenCount = 10;
    const char* SrcStr;
    size_t i;
	
    int token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
	
    if (token=='\n') {
        DecLineNumber();
        CPPErrorToInfoLog("#pragma");
        IncLineNumber();
        return token;
    }
	
    allTokens = (char**)malloc(sizeof(char*) * maxTokenCount);
	
    while (token != '\n') {
        if (tokenCount >= maxTokenCount) {
            maxTokenCount *= 2;
            allTokens = (char**)realloc((char**)allTokens, sizeof(char*) * maxTokenCount);
        }
        switch (token) {
			case CPP_IDENTIFIER:
				SrcStr = GetAtomString(atable, yylvalpp->sc_ident);
				allTokens[tokenCount] = (char*)malloc(strlen(SrcStr) + 1);
				strcpy(allTokens[tokenCount++], SrcStr);
				break;
			case CPP_INTCONSTANT:
				SrcStr = yylvalpp->symbol_name;
				allTokens[tokenCount] = (char*)malloc(strlen(SrcStr) + 1);
				strcpy(allTokens[tokenCount++], SrcStr);
				break;
			case CPP_FLOATCONSTANT:
				SrcStr = yylvalpp->symbol_name;
				allTokens[tokenCount] = (char*)malloc(strlen(SrcStr) + 1);
				strcpy(allTokens[tokenCount++], SrcStr);
				break;
			case -1:
				// EOF
				CPPShInfoLogMsg("#pragma directive must end with a newline");
				return token;
			default:
				SrcStrName[0] = token;
				SrcStrName[1] = '\0';
				allTokens[tokenCount] = (char*)malloc(2);
				strcpy(allTokens[tokenCount++], SrcStrName);
        }
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    }
	
	cpp->currentInput->ungetch(cpp->currentInput, token);
	// just ignore #pragmas
    token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
	
    for (i = 0; i < tokenCount; ++i) {
        free (allTokens[i]);
    }
    free (allTokens);
	
    return token;
} // CPPpragma


int readCPPline(yystypepp * yylvalpp)
{
    int token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    const char *message;
	
    if (token == CPP_IDENTIFIER) {
        if (yylvalpp->sc_ident == defineAtom) {
			token = CPPdefine(yylvalpp);
        } else if (yylvalpp->sc_ident == elseAtom) {
            token = CPPgotoEndOfIfBlock(1, yylvalpp);
            if (yylvalpp->sc_ident != endifAtom)
            {
                CPPErrorToInfoLog("#else or #elif mismatch");
                cpp->CompileError=1;
            }
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            if (token != '\n')
                CPPWarningToInfoLog("unexpected tokens following #else preprocessor directive - expected a newline");
			
			if (!cpp->ifdepth){
				CPPErrorToInfoLog("#else or #elif mismatch");
				cpp->CompileError=1;
			}
			else
				--cpp->ifdepth;
        } else if (yylvalpp->sc_ident == elifAtom) {
            token = CPPgotoEndOfIfBlock(0, yylvalpp);
            assert(yylvalpp->sc_ident == endifAtom);
            token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
            if (!cpp->ifdepth){
                CPPErrorToInfoLog("#else or #elif mismatch");
                cpp->CompileError=1;
            }
            else
                --cpp->ifdepth;
        } else if (yylvalpp->sc_ident == endifAtom) {
			if (!cpp->ifdepth){
				CPPErrorToInfoLog("#endif mismatch");
				cpp->CompileError=1;
			}
			else
				--cpp->ifdepth;
        } else if (yylvalpp->sc_ident == ifAtom) {
			token = CPPif(yylvalpp);
        } else if (yylvalpp->sc_ident == ifdefAtom) {
			token = CPPifdef(1, yylvalpp);
        } else if (yylvalpp->sc_ident == ifndefAtom) {
			token = CPPifdef(0, yylvalpp);
        } else if (yylvalpp->sc_ident == lineAtom) {
			token = CPPline(yylvalpp);
        } else if (yylvalpp->sc_ident == pragmaAtom) {
			token = CPPpragma(yylvalpp);
        } else if (yylvalpp->sc_ident == undefAtom) {
			token = CPPundef(yylvalpp);
        } else if (yylvalpp->sc_ident == errorAtom) {
			token = CPPerror(yylvalpp);
        } else {
            StoreStr("Invalid Directive");
            StoreStr(GetStringOfAtom(atable,yylvalpp->sc_ident));
            message=GetStrfromTStr();
            CPPShInfoLogMsg(message);
            ResetTString();
        }
    }
    while (token != '\n' && token != 0 && token != EOF) {
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
    }
	
    return token;
} // readCPPline


static int eof_scan(InputSrc *in, yystypepp * yylvalpp) { return -1; }
static int noop_getch(InputSrc *in) { return -1; }
static void noop_ungetc(InputSrc *in, int ch) { }

static void PushEofSrc() {
    InputSrc *in = malloc(sizeof(InputSrc));
    memset(in, 0, sizeof(InputSrc));
    in->scan = eof_scan;
    in->getch = noop_getch;
    in->ungetch = noop_ungetc;
    in->prev = cpp->currentInput;
    cpp->currentInput = in;
}

static void PopEofSrc() {
    if (cpp->currentInput->scan == eof_scan) {
        InputSrc *in = cpp->currentInput;
        cpp->currentInput = in->prev;
        free(in);
    }
}

static TokenStream *PrescanMacroArg(TokenStream *a, yystypepp * yylvalpp) {
    int token;
    TokenStream *n;
    RewindTokenStream(a);
    do {
        token = ReadToken(a, yylvalpp);
        if (token == CPP_IDENTIFIER && LookUpSymbol(macros, yylvalpp->sc_ident))
            break;
    } while (token > 0);
    if (token <= 0) return a;
    n = NewTokenStream("macro arg", 0);
    PushEofSrc();
    ReadFromTokenStream(a, 0);
    while ((token = cpp->currentInput->scan(cpp->currentInput, yylvalpp)) > 0) {
        if (token == CPP_IDENTIFIER && MacroExpand(yylvalpp->sc_ident, yylvalpp))
            continue;
        RecordToken(n, token, yylvalpp);
    }
    PopEofSrc();
    DeleteTokenStream(a);
    return n;
} // PrescanMacroArg

typedef struct MacroInputSrc {
    InputSrc    base;
    MacroSymbol *mac;
    TokenStream **args;
} MacroInputSrc;



static int expand_macro_param(MacroInputSrc* in, yystypepp* yylvalpp)
{
	int i;
	for (i = in->mac->argc-1; i>=0; i--)
		if (in->mac->args[i] == yylvalpp->sc_ident) break;
	if (i >= 0) {
		ReadFromTokenStream(in->args[i], 0);
		return cpp->currentInput->scan(cpp->currentInput, yylvalpp);
	}
	return -1;
}


/* macro_scan ---
 ** return the next token for a macro expanion, handling macro args
 */
static int macro_scan(MacroInputSrc *in, yystypepp * yylvalpp) {
    int i;
    int token = ReadToken(in->mac->body, yylvalpp);
	

	// check if we need to paste token with next
	yystypepp right_valpp;
	int right_tok = PeekTokenType(in->mac->body);
	if (right_tok == CPP_TOKENPASTE)
	{
		int left_exp;
		int right_exp;
		const char* left_name;
		const char* right_name;
		char newname[MAX_SYMBOL_NAME_LEN + 1] = { 0 };
		ReadToken(in->mac->body, &right_valpp); // dump the CPP_TOKENPASTE token
		right_tok = ReadToken(in->mac->body, &right_valpp);
		assert(right_tok > 0);

		left_exp = expand_macro_param(in, yylvalpp);
		if (left_exp <= 0)
			left_exp = token;

		right_exp = expand_macro_param(in, &right_valpp);
		if (right_exp <= 0)
			right_exp = right_tok;

		// FIXME: need more cases?
		if (left_exp == CPP_IDENTIFIER)
			left_name  = GetStringOfAtom(atable, yylvalpp->sc_ident);
		else if (left_exp == CPP_INTCONSTANT)
			left_name = yylvalpp->symbol_name;
		else
			assert(0);

		if (right_exp == CPP_IDENTIFIER)
			right_name = GetStringOfAtom(atable, right_valpp.sc_ident);
		else if (right_exp == CPP_INTCONSTANT)
			right_name = right_valpp.symbol_name;
		else
			assert(0);

		strncat(newname, left_name, MAX_SYMBOL_NAME_LEN);
		strncat(newname, right_name, MAX_SYMBOL_NAME_LEN - strlen(newname));
		yylvalpp->sc_ident = LookUpAddString(atable, newname);
		return left_exp;
	}
	
    if (token == CPP_IDENTIFIER) {
		int expanded = expand_macro_param(in, yylvalpp);
		if (expanded > 0)
			return expanded;
    }
    if (token > 0) return token;
    in->mac->busy = 0;
    cpp->currentInput = in->base.prev;
    if (in->args) {
        for (i=in->mac->argc-1; i>=0; i--)
            DeleteTokenStream(in->args[i]);
        free(in->args);
    }
    free(in);
    return cpp->currentInput->scan(cpp->currentInput, yylvalpp);
} // macro_scan

/* MacroExpand
 ** check an identifier (atom) to see if it a macro that should be expanded.
 ** If it is, push an InputSrc that will produce the appropriate expansion
 ** and return TRUE.  If not, return FALSE.
 */

int MacroExpand(int atom, yystypepp * yylvalpp)
{
    Symbol              *sym = LookUpSymbol(macros, atom);
    MacroInputSrc       *in;
    int i,j, token, depth=0;
    const char *message;
    if (atom == __LINE__Atom) {
        yylvalpp->sc_int = GetLineNumber().line;
        sprintf(yylvalpp->symbol_name,"%d",yylvalpp->sc_int);
        UngetToken(CPP_INTCONSTANT, yylvalpp);
        return 1;
    }
    if (atom == __FILE__Atom) {
        const char* file = GetLineNumber().file;
        if(!file) file = "";
        yylvalpp->sc_ident = LookUpAddString(atable, file);
        UngetToken(CPP_STRCONSTANT, yylvalpp);
        return 1;
    }
    if (!sym || sym->mac.undef) return 0;
    if (sym->mac.busy) return 0;        // no recursive expansions
    in = malloc(sizeof(*in));
    memset(in, 0, sizeof(*in));
    in->base.scan = (void *)macro_scan;
    in->base.line = cpp->currentInput->line;
    in->mac = &sym->mac;
    if (sym->mac.args) {
        token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
        if (token != '(') {
            UngetToken(token, yylvalpp);
            yylvalpp->sc_ident = atom;
            return 0;
        }
        in->args = malloc(in->mac->argc * sizeof(TokenStream *));
        for (i=0; i<in->mac->argc; i++)
            in->args[i] = NewTokenStream("macro arg", 0);
        i=0;j=0;
        do{
            depth = 0;
            while(1) {
                token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
                if (token <= 0) {
                    StoreStr("EOF in Macro ");
                    StoreStr(GetStringOfAtom(atable,atom));
                    message=GetStrfromTStr();
                    CPPShInfoLogMsg(message);
                    ResetTString();
                    return 1;
                }
                if((in->mac->argc==0) && (token!=')')) break;
                if (depth == 0 && (token == ',' || token == ')')) break;
                if (token == '(') depth++;
                if (token == ')') depth--;
                RecordToken(in->args[i], token, yylvalpp);
                j=1;
            }
            if (token == ')') {
                if((in->mac->argc==1) &&j==0)
                    break;
                i++;
                break;
            }
            i++;
        }while(i < in->mac->argc);
		
        if (i < in->mac->argc) {
            StoreStr("Too few args in Macro ");
            StoreStr(GetStringOfAtom(atable,atom));
            message=GetStrfromTStr();
            CPPShInfoLogMsg(message);
            ResetTString();
        } else if (token != ')') {
            depth=0;
            while (token >= 0 && (depth > 0 || token != ')')) {
                if (token == ')') depth--;
                token = cpp->currentInput->scan(cpp->currentInput, yylvalpp);
                if (token == '(') depth++;
            }
			
            if (token <= 0) {
                StoreStr("EOF in Macro ");
                StoreStr(GetStringOfAtom(atable,atom));
                message=GetStrfromTStr();
                CPPShInfoLogMsg(message);
                ResetTString();
                return 1;
            }
            StoreStr("Too many args in Macro ");
            StoreStr(GetStringOfAtom(atable,atom));
            message=GetStrfromTStr();
            CPPShInfoLogMsg(message);
            ResetTString();
        }
        for (i=0; i<in->mac->argc; i++) {
            in->args[i] = PrescanMacroArg(in->args[i], yylvalpp);
        }
    }
	
    /*retain the input source*/
    in->base.prev = cpp->currentInput;
    sym->mac.busy = 1;
    RewindTokenStream(sym->mac.body);
    cpp->currentInput = &in->base;
    return 1;
} // MacroExpand
