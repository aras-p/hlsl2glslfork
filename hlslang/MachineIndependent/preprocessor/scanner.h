// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#if !defined(__SCANNER_H)
#define __SCANNER_H 1

#define MAX_SYMBOL_NAME_LEN 128
#define MAX_STRING_LEN 512

#include "parser.h"
#include "sourceloc.h"

// Not really atom table stuff but needed first...

typedef struct SourceLoc_Rec {
    unsigned short line;
} SourceLoc;

int yylex_CPP(char* buf, int maxSize);

typedef struct InputSrc {
    struct InputSrc	*prev;
    int			(*scan)(struct InputSrc *, yystypepp *);
    int			(*getch)(struct InputSrc *);
    void		(*ungetch)(struct InputSrc *, int);
    int			line;
} InputSrc;

int InitScanner(CPPStruct *cpp);   // Intialise the cpp scanner. 
int ScanFromString(char *);      // Start scanning the input from the string mentioned.
int check_EOF(int);              // check if we hit a EOF abruptly 
void CPPErrorToInfoLog(char *);   // sticking the msg,line into the Shader's.Info.log
void SetLineNumber(TSourceLoc line);
void IncLineNumber(void);
void DecLineNumber(void);
int FreeScanner(void);                 // Free the cpp scanner
#endif // !(defined(__SCANNER_H)

