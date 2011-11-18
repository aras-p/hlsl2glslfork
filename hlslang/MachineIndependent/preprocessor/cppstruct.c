// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include <stdio.h>
#include <stdlib.h>

#include "slglobals.h"

CPPStruct  *cpp      = NULL;

int InitPreprocessor(void);
int FinalizePreprocessor(void);


static void InitCPPStruct(void)
{
	if (cpp)
		free (cpp);
    cpp = (CPPStruct*)malloc(sizeof(CPPStruct));
    if (cpp == NULL)
        return;

	cpp->pC=0;
    cpp->CompileError=0; 
	cpp->ifdepth=0;
    for(cpp->elsetracker=0; cpp->elsetracker<64; cpp->elsetracker++)
		cpp->elsedepth[cpp->elsetracker]=0; 
	cpp->elsetracker=0;
    cpp->tokensBeforeEOF = 0;
} // InitCPPStruct



int InitPreprocessor(void)
{
	InitCPPStruct();
	if (!InitAtomTable(atable, 0))
		return 1;
	if (!InitScanner(cpp))
		return 1;
	return 0; 
}


int FinalizePreprocessor(void)
{
	FreeAtomTable(atable);
	if (cpp)
		free(cpp);
	cpp = NULL;    
	FreeScanner();
	return 0;
}
