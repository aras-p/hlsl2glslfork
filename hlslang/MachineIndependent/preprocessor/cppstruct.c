// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include <stdio.h>
#include <stdlib.h>

#include "slglobals.h"

CPPStruct  *cpp      = NULL;
static int  refCount = 0;

int InitPreprocessor(void);
int ResetPreprocessor(void);
int FreeCPPStruct(void);
int FinalizePreprocessor(void);



int InitCPPStruct(void)
{
    int len;
    char *p;

    cpp = (CPPStruct *) malloc(sizeof(CPPStruct));
    if (cpp == NULL)
        return 0;

    refCount++;

    // Initialize public members:
    cpp->pLastSourceLoc = &cpp->lastSourceLoc;
    
	p = (char *) &cpp->options;
    len = sizeof(cpp->options);
    while (--len >= 0)
        p[len] = 0;
     
    ResetPreprocessor();
    return 1;
} // InitCPPStruct


int ResetPreprocessor(void)
{
    // Initialize private members:

    cpp->lastSourceLoc.file = 0;
    cpp->lastSourceLoc.line = 0;
	cpp->pC=0;
    cpp->CompileError=0; 
	cpp->ifdepth=0;
    for(cpp->elsetracker=0; cpp->elsetracker<64; cpp->elsetracker++)
		cpp->elsedepth[cpp->elsetracker]=0; 
	cpp->elsetracker=0;
    cpp->tokensBeforeEOF = 0;
    return 1;
}


int InitPreprocessor(void)
{
   #  define CPP_STUFF true
        #  ifdef CPP_STUFF
            FreeCPPStruct();
            InitCPPStruct();
            cpp->options.Quiet = 1;
            if (!InitAtomTable(atable, 0))
                return 1;
            if (!InitScanner(cpp))
	            return 1;
       #  endif
  return 0; 
}


int FreeCPPStruct(void)
{
    if (refCount)
    {
       free(cpp);
       refCount--;
    }
    
    return 1;
}


int FinalizePreprocessor(void)
{
   #  define CPP_STUFF true
        #  ifdef CPP_STUFF
            FreeAtomTable(atable);
            FreeCPPStruct();
            FreeScanner();
       #  endif
  return 0; 
}
