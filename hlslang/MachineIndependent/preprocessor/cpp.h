// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#if !defined(__CPP_H)
#define __CPP_H 1

#include "parser.h"
#include "tokens.h"

int InitCPP(void);
int FinalCPP(void);
int  readCPPline(yystypepp * yylvalpp);
int MacroExpand(int atom, yystypepp * yylvalpp);
int ChkCorrectElseNesting(void);

typedef struct MacroSymbol {
    int argc;
    int *args;
    TokenStream *body;
    unsigned busy:1;
    unsigned undef:1;
} MacroSymbol;

void FreeMacro(MacroSymbol *);
int PredefineMacro(char *);

void  CPPShInfoLogMsg(const char*);         // Store cpp Err Msg into Sh.Info.Log
void  CPPWarningToInfoLog(const char *msg); // Prints warning messages into info log
void  ResetTString(void);                   // #error Message as TString.
void  CPPErrorToInfoLog(char*);             // Stick all cpp errors into Sh.Info.log   .
void  StoreStr(char*);                      // Store the TString in Parse Context.
void  SetLineNumber(int);                   // Set line number.  
void  SetStringNumber(int);                 // Set string number.    
int   GetLineNumber(void);                  // Get the current String Number. 
int   GetStringNumber(void);                // Get the current String Number. 
const char* GetStrfromTStr(void);           // Convert TString to String.  
int   FreeCPP(void);

#endif // !(defined(__CPP_H)
