// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#if !defined(__TOKENS_H)
#define __TOKENS_H 1

#include "parser.h"

#define EOF_SY (-1)

typedef struct TokenBlock_Rec TokenBlockStruct;

typedef struct TokenStream_Rec {
    struct TokenStream_Rec *next;
    char *name;
    TokenBlockStruct *head;
    TokenBlockStruct *current;
} TokenStream;

struct TokenBlock_Rec {
    TokenBlockStruct *next;
    int current;
    int count;
    int max;
    unsigned char *data;
};

extern TokenStream stdlib_cpp_stream;


TokenStream *NewTokenStream(const char *name, MemoryPool *pool);
void DeleteTokenStream(TokenStream *pTok); 
void RecordToken(TokenStream *pTok, int token, yystypepp * yylvalpp);
void RewindTokenStream(TokenStream *pTok);
int ReadToken(TokenStream *pTok, yystypepp * yylvalpp);
int ReadFromTokenStream(TokenStream *pTok, int name, int (*final)(CPPStruct *));
void UngetToken(int, yystypepp * yylvalpp);

#if defined(CPPC_ENABLE_TOOLS)

void DumpTokenStream(FILE *, TokenStream *, yystypepp * yylvalpp);

#endif // defined(CPPC_ENABLE_TOOLS)

#endif // !defined(__TOKENS_H)
