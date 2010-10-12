// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "slglobals.h"


Scope *ScopeList = NULL;
Scope *CurrentScope = NULL;
Scope *GlobalScope = NULL;

static void unlinkScope(void *_scope) {
    Scope *scope = _scope;

    if (scope->next)
        scope->next->prev = scope->prev;
    if (scope->prev)
        scope->prev->next = scope->next;
    else
        ScopeList = scope->next;
}



Scope *NewScopeInPool(MemoryPool *pool)
{
    Scope *lScope;

    lScope = mem_Alloc(pool, sizeof(Scope));
    lScope->pool = pool;
    lScope->parent = NULL;
    lScope->funScope = NULL;
    lScope->symbols = NULL;
    
    lScope->level = 0;

    lScope->programs = NULL;
    if ((lScope->next = ScopeList))
        ScopeList->prev = lScope;
    lScope->prev = 0;
    ScopeList = lScope;
    mem_AddCleanup(pool, unlinkScope, lScope);
    return lScope;
} // NewScope


void PushScope(Scope *fScope)
{
    Scope *lScope;

    if (CurrentScope) {
        fScope->level = CurrentScope->level + 1;
        if (fScope->level == 1) {
            if (!GlobalScope) {
                /* HACK - CTD -- if GlobalScope==NULL and level==1, we're
                 * defining a function in the superglobal scope.  Things
                 * will break if we leave the level as 1, so we arbitrarily
                 * set it to 2 */
                fScope->level = 2;
            }
        }
        if (fScope->level >= 2) {
            lScope = fScope;
            while (lScope->level > 2)
                lScope = lScope->next;
            fScope->funScope = lScope;
        }
    } else {
        fScope->level = 0;
    }
    fScope->parent = CurrentScope;
    CurrentScope = fScope;
} // PushScope



Scope *PopScope(void)
{
    Scope *lScope;

    lScope = CurrentScope;
    if (CurrentScope)
        CurrentScope = CurrentScope->parent;
    return lScope;
} // PopScope



Symbol *NewSymbol(SourceLoc *loc, Scope *fScope, int name, symbolkind kind)
{
    Symbol *lSymb;
    char *pch;
    int ii;

    lSymb = (Symbol *) mem_Alloc(fScope->pool, sizeof(Symbol));
    lSymb->left = NULL;
    lSymb->right = NULL;
    lSymb->next = NULL;
    lSymb->name = name;
    lSymb->loc = *loc;
    lSymb->kind = kind;
    
    // Clear union area:

    pch = (char *) &lSymb->details;
    for (ii = 0; ii < sizeof(lSymb->details); ii++)
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
Symbol *AddSymbol(SourceLoc *loc, Scope *fScope, int atom, symbolkind kind)
{
    Symbol *lSymb;

    if (!fScope)
        fScope = CurrentScope;
    lSymb = NewSymbol(loc, fScope, atom, kind);
    lAddToTree(&fScope->symbols, lSymb);
    return lSymb;
} // AddSymbol



Symbol *LookUpLocalSymbol(Scope *fScope, int atom)
{
    Symbol *lSymb;
    int rname, ratom;

    ratom = GetReversedAtom(atable, atom);
    if (!fScope)
        fScope = CurrentScope;
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
} // LookUpLocalSymbol



Symbol *LookUpSymbol(Scope *fScope, int atom)
{
    Symbol *lSymb;

    if (!fScope)
        fScope = CurrentScope;
    while (fScope) {
        lSymb = LookUpLocalSymbol(fScope, atom);
        if (lSymb)
            return lSymb;
        fScope = fScope->parent;
    }
    return NULL;
} // LookUpSymbol

