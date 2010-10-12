// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

# include "slglobals.h"
extern CPPStruct *cpp;
int InitCPPStruct(void);
int InitScanner(CPPStruct *cpp);
int InitAtomTable(AtomTable *atable, int htsize);
int ScanFromString(char *s);
char* GetStringOfAtom(AtomTable *atable, int atom);
