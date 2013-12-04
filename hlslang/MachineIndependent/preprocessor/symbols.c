// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "slglobals.h"



Scope *NewScopeInPool(MemoryPool *pool)
{
    Scope *lScope;

    lScope = mem_Alloc(pool, sizeof(Scope));
    lScope->pool = pool;
    lScope->symbols = NULL;
    return lScope;
} // NewScopeInPool







Symbol *NewSymbol(SourceLoc *loc, Scope *fScope, int name)
{
    Symbol *lSymb;
    char *pch;
    size_t ii;

    lSymb = (Symbol *) mem_Alloc(fScope->pool, sizeof(Symbol));
    lSymb->left = NULL;
    lSymb->right = NULL;
    lSymb->next = NULL;
    lSymb->name = name;
    lSymb->loc = *loc;
    
    // Clear macro area
    pch = (char *) &lSymb->mac;
    for (ii = 0; ii < sizeof(lSymb->mac); ii++)
        *pch++ = 0;
    return lSymb;
} // NewSymbol



// Using a binary tree is not a good idea for basic atom values because they
// are generated in order.  We'll fix this later (by reversing the bit pattern).
static void lAddToTree(Symbol **fSymbols, Symbol *fSymb)
{
    Symbol *lSymb;
    int lrev, frev;

    lSymb = *fSymbols;
    if (lSymb) {
        frev = GetReversedAtom(atable, fSymb->name);
        while (lSymb) {
            lrev = GetReversedAtom(atable, lSymb->name);
            if (lrev == frev) {
                CPPErrorToInfoLog("GetAtomString(atable, fSymb->name)");
                break;
            } else {
                if (lrev > frev) {
                    if (lSymb->left) {
                        lSymb = lSymb->left;
                    } else {
                        lSymb->left = fSymb;
                        break;
                    }
                } else {
                    if (lSymb->right) {
                        lSymb = lSymb->right;
                    } else {
                        lSymb->right = fSymb;
                        break;
                    }
                }
            }
        }
    } else {
        *fSymbols = fSymb;
    }
} // lAddToTree



// Add a variable, type, or function name to a scope.
Symbol *AddSymbol(SourceLoc *loc, Scope *fScope, int atom)
{
    Symbol *lSymb;

    lSymb = NewSymbol(loc, fScope, atom);
    lAddToTree(&fScope->symbols, lSymb);
    return lSymb;
} // AddSymbol



Symbol *LookUpSymbol(Scope *fScope, int atom)
{
    Symbol *lSymb;
    int rname, ratom;

    ratom = GetReversedAtom(atable, atom);
    lSymb = fScope->symbols;
    while (lSymb) {
        rname = GetReversedAtom(atable, lSymb->name);
        if (rname == ratom) {
            return lSymb;
        } else {
            if (rname > ratom) {
                lSymb = lSymb->left;
            } else {
                lSymb = lSymb->right;
            }
        }
    }
    return NULL;
} // LookUpSymbol
