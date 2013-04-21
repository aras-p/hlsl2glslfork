// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "slglobals.h"


// ------------------------------------------------------------------
// Preprocessor and Token Recorder and Playback


// Copy a string to a malloc'ed block and convert it into something suitable for an ID
static char *idstr(const char *fstr, MemoryPool *pool)
{
    size_t len;
    char *str, *t;
    const char *f;

    len = strlen(fstr);
    if (!pool)
        str = (char *) malloc(len + 1);
    else
        str = (char *) mem_Alloc(pool, len + 1);
    
    for (f=fstr, t=str; *f; f++) {
        if (isalnum(*f)) *t++ = *f;
        else if (*f == '.' || *f == '/') *t++ = '_';
    }
    *t = 0;
    return str;
} // idstr



static TokenBlockStruct *lNewBlock(TokenStream *fTok, MemoryPool *pool)
{
    TokenBlockStruct *lBlock;

    if (!pool)
        lBlock = (TokenBlockStruct *) malloc(sizeof(TokenBlockStruct) + 256);
    else
        lBlock = (TokenBlockStruct *) mem_Alloc(pool, sizeof(TokenBlockStruct) + 256);
    lBlock->count = 0;
    lBlock->current = 0;
    lBlock->data = (unsigned char *) lBlock + sizeof(TokenBlockStruct);
    lBlock->max = 256;
    lBlock->next = NULL;
    if (fTok->head) {
        fTok->current->next = lBlock;
    } else {
        fTok->head = lBlock;
    }
    fTok->current = lBlock;
    return lBlock;
} // lNewBlock



static void lAddByte(TokenStream *fTok, unsigned char fVal)
{
    TokenBlockStruct *lBlock;
    lBlock = fTok->current;
    if (lBlock->count >= lBlock->max)
        lBlock = lNewBlock(fTok, 0);
    lBlock->data[lBlock->count++] = fVal;
} // lAddByte



static int lReadByte(TokenStream *pTok)
{
    TokenBlockStruct *lBlock;
    int lval = -1;

    lBlock = pTok->current;
    if (lBlock) {
        if (lBlock->current >= lBlock->count) {
            lBlock = lBlock->next;
            if (lBlock)
                lBlock->current = 0;
            pTok->current = lBlock;
        }
        if (lBlock)
            lval = lBlock->data[lBlock->current++];
    }
    return lval;
} // lReadByte


/*
 * lPeekByte() - Peek the next byte from a stream.
 *
 */
static int lPeekByte(TokenStream *pTok)
{
	TokenBlockStruct *lBlock;
	int lval = -1;

	lBlock = pTok->current;
	if (lBlock) {
		if (lBlock->current >= lBlock->count) {
			lBlock = lBlock->next;
			if (lBlock)
				lval = lBlock->data[0];
		}
		else if (lBlock)
			lval = lBlock->data[lBlock->current];
	}
	return lval;
} // lPeekByte



// ------------------------------------------------------------------
// Global Functions


TokenStream *NewTokenStream(const char *name, MemoryPool *pool)
{
    TokenStream *pTok;

    if (!pool)
        pTok = (TokenStream *) malloc(sizeof(TokenStream));
    else
        pTok = (TokenStream*)mem_Alloc(pool, sizeof(TokenStream));
    pTok->next = NULL;
    pTok->name = idstr(name, pool);
    pTok->head = NULL;
    pTok->current = NULL;
    lNewBlock(pTok, pool);
    return pTok;
} // NewTokenStream



void DeleteTokenStream(TokenStream *pTok)
{
    TokenBlockStruct *pBlock, *nBlock;

    if (pTok) {
        pBlock = pTok->head;
        while (pBlock) {
            nBlock = pBlock->next;
            free(pBlock);
            pBlock = nBlock;
        }
        if (pTok->name)
            free(pTok->name);
        free(pTok);
    }
} // DeleteTokenStream



// Add a token to the end of a list for later playback or printout.
void RecordToken(TokenStream *pTok, int token, yystypepp * yylvalpp)
{
    const char *s;
    const char *str=NULL;

    if (token > 256)
        lAddByte(pTok, (unsigned char)((token & 0x7f) + 0x80));
    else
        lAddByte(pTok, (unsigned char)(token & 0x7f));
    switch (token) {
    case CPP_IDENTIFIER:
    case CPP_TYPEIDENTIFIER:
    case CPP_STRCONSTANT:
        s = GetAtomString(atable, yylvalpp->sc_ident);
        while (*s)
            lAddByte(pTok, (unsigned char) *s++);
        lAddByte(pTok, 0);
        break;
    case CPP_FLOATCONSTANT:
    case CPP_INTCONSTANT:
         str=yylvalpp->symbol_name;
         while (*str){
            lAddByte(pTok,(unsigned char) *str);
            str++;
         }
         lAddByte(pTok, 0);
         break;
    case '(':
        lAddByte(pTok, (unsigned char)(yylvalpp->sc_int ? 1 : 0));
    default:
        break;
    }
} // RecordToken



// Reset a token stream in preparation for reading.
void RewindTokenStream(TokenStream *pTok)
{
    if (pTok->head) {
        pTok->current = pTok->head;
        pTok->current->current = 0;
    }
} // RewindTokenStream



int PeekTokenType(TokenStream *pTok)
{
	int ltoken = lPeekByte(pTok);
	if (ltoken > 127)
		ltoken += 128;
	return ltoken;
}


int ReadToken(TokenStream *pTok, yystypepp * yylvalpp)
{
    char symbol_name[MAX_SYMBOL_NAME_LEN + 1];
    char string_val[MAX_STRING_LEN + 1];
    int ltoken, len;
    char ch;

    ltoken = lReadByte(pTok);
    if (ltoken >= 0) {
        if (ltoken > 127)
            ltoken += 128;
        switch (ltoken) {
        case CPP_IDENTIFIER:
        case CPP_TYPEIDENTIFIER:
            len = 0;
            ch = lReadByte(pTok);
            while ((ch >= 'a' && ch <= 'z') ||
                     (ch >= 'A' && ch <= 'Z') ||
                     (ch >= '0' && ch <= '9') ||
                     ch == '_')
            {
                if (len < MAX_SYMBOL_NAME_LEN) {
                    symbol_name[len] = ch;
                    len++;
                    ch = lReadByte(pTok);
                }
            }
            symbol_name[len] = '\0';
            assert(ch == '\0');
            yylvalpp->sc_ident = LookUpAddString(atable, symbol_name);
            return CPP_IDENTIFIER;
            break;
        case CPP_STRCONSTANT:
            len = 0;
            while ((ch = lReadByte(pTok)) != 0)
                if (len < MAX_STRING_LEN)
                    string_val[len++] = ch;
            string_val[len] = 0;
            yylvalpp->sc_ident = LookUpAddString(atable, string_val);
            break;
        case CPP_FLOATCONSTANT:
            len = 0;
            ch = lReadByte(pTok);
            while ((ch >= '0' && ch <= '9')||(ch=='e'||ch=='E'||ch=='.')||(ch=='+'||ch=='-') ||
                (ch=='f'||ch=='h'))
            {
                if (len < MAX_SYMBOL_NAME_LEN) {
                    symbol_name[len] = ch;
                    len++;
                    ch = lReadByte(pTok);
                }
            }
            symbol_name[len] = '\0';
            assert(ch == '\0');
            strcpy(yylvalpp->symbol_name,symbol_name);
            yylvalpp->sc_fval=(float)atof(yylvalpp->symbol_name);
            break;
        case CPP_INTCONSTANT:
            len = 0;
            ch = lReadByte(pTok);
            while ((ch >= '0' && ch <= '9'))
            {
                if (len < MAX_SYMBOL_NAME_LEN) {
                    symbol_name[len] = ch;
                    len++;
                    ch = lReadByte(pTok);
                }
            }
            symbol_name[len] = '\0';
            assert(ch == '\0');
            strcpy(yylvalpp->symbol_name,symbol_name);
            yylvalpp->sc_int=atoi(yylvalpp->symbol_name);
            break;
        case '(':
            yylvalpp->sc_int = lReadByte(pTok);
            break;
        }
        return ltoken;
    }
    return EOF_SY;
} // ReadToken


typedef struct TokenInputSrc {
    InputSrc            base;
    TokenStream         *tokens;
    int                 (*final)(CPPStruct *);
} TokenInputSrc;


static int scan_token(TokenInputSrc *in, yystypepp * yylvalpp)
{
    int token = ReadToken(in->tokens, yylvalpp);
    int (*final)(CPPStruct *);
    if (token == '\n') {
        in->base.line++;
        return token;
    }
    if (token > 0) return token;
    cpp->currentInput = in->base.prev;
    final = in->final;
    free(in);
    if (final && !final(cpp)) return -1;
    return cpp->currentInput->scan(cpp->currentInput, yylvalpp);
}

int ReadFromTokenStream(TokenStream *ts, int (*final)(CPPStruct *))
{
    TokenInputSrc *in = malloc(sizeof(TokenInputSrc));
    memset(in, 0, sizeof(TokenInputSrc));
    in->base.prev = cpp->currentInput;
    in->base.scan = (int (*)(InputSrc *, yystypepp *))scan_token;
    in->base.line = 1;
    in->tokens = ts;
    in->final = final;
    RewindTokenStream(ts);
    cpp->currentInput = &in->base;
    return 1;
}

typedef struct UngotToken {
    InputSrc    base;
    int         token;
    yystypepp     lval;
} UngotToken;

static int reget_token(UngotToken *t, yystypepp * yylvalpp)
{
    int token = t->token;
    *yylvalpp = t->lval;
    cpp->currentInput = t->base.prev;
    free(t);
    return token;
}

void UngetToken(int token, yystypepp * yylvalpp) {
    UngotToken *t = malloc(sizeof(UngotToken));
    memset(t, 0, sizeof(UngotToken));
    t->token = token;
    t->lval = *yylvalpp;
    t->base.scan = (void *)reget_token;
    t->base.prev = cpp->currentInput;
    t->base.line = cpp->currentInput->line;
    cpp->currentInput = &t->base;
}
