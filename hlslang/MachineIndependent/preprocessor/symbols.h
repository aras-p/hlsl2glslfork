// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#if !defined(__SYMBOLS_H)
#define __SYMBOLS_H 1

#include "memory.h"

// Typedefs for things defined here in "symbols.h":

typedef struct Scope_Rec Scope;
typedef struct Symbol_Rec Symbol;

typedef struct SymbolList_Rec {
    struct SymbolList_Rec *next;
    Symbol *symb;
} SymbolList;

struct Scope_Rec {
    MemoryPool *pool;       // pool used for allocation in this scope
    Symbol *symbols;
};


// Symbol table is a simple binary tree.

#include "cpp.h"        // to get MacroSymbol def

struct Symbol_Rec {
    Symbol *left, *right;
    Symbol *next;
    int name;       // Name atom
    SourceLoc loc;
    MacroSymbol mac;
};


Scope *NewScopeInPool(MemoryPool *);
Symbol *NewSymbol(SourceLoc *loc, Scope *fScope, int name);
Symbol *AddSymbol(SourceLoc *loc, Scope *fScope, int atom);
Symbol *LookUpLocalSymbol(Scope *fScope, int atom);
Symbol *LookUpSymbol(Scope *fScope, int atom);
void CPPErrorToInfoLog(char *);


#endif // !defined(__SYMBOLS_H)

