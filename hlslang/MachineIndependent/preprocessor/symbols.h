// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#if !defined(__SYMBOLS_H)
#define __SYMBOLS_H 1

#include "memory.h"

typedef enum symbolkind {
   MACRO_S
} symbolkind;

// Typedefs for things defined here in "symbols.h":

typedef struct Scope_Rec Scope;
typedef struct Symbol_Rec Symbol;

typedef struct SymbolList_Rec {
    struct SymbolList_Rec *next;
    Symbol *symb;
} SymbolList;

struct Scope_Rec {
    Scope *next, *prev;     // doubly-linked list of all scopes
    Scope *parent;
    Scope *funScope;        // Points to base scope of enclosing function
    MemoryPool *pool;       // pool used for allocation in this scope
    Symbol *symbols;
    
	int level;              // 0 = super globals, 1 = globals, etc.

    // Only used at global scope (level 1):
    SymbolList *programs;   // List of programs for this compilation.
};


// Symbol table is a simple binary tree.

#include "cpp.h"        // to get MacroSymbol def

struct Symbol_Rec {
    Symbol *left, *right;
    Symbol *next;
    int name;       // Name atom
    SourceLoc loc;
    symbolkind kind;
    union {
        MacroSymbol mac;
    } details;
};

extern Scope *CurrentScope;
extern Scope *GlobalScope;
extern Scope *ScopeList;

Scope *NewScopeInPool(MemoryPool *);
#define NewScope()      NewScopeInPool(CurrentScope->pool)
void PushScope(Scope *fScope);
Scope *PopScope(void);
Symbol *NewSymbol(SourceLoc *loc, Scope *fScope, int name, symbolkind kind);
Symbol *AddSymbol(SourceLoc *loc, Scope *fScope, int atom, symbolkind kind);
Symbol *LookUpLocalSymbol(Scope *fScope, int atom);
Symbol *LookUpSymbol(Scope *fScope, int atom);
void CPPErrorToInfoLog(char *);


#endif // !defined(__SYMBOLS_H)

