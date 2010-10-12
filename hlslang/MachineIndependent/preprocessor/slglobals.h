// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#if !defined(__SLGLOBALS_H)
#define __SLGLOBALS_H 1

typedef struct CPPStruct_Rec CPPStruct;

extern CPPStruct *cpp;

#undef  CPPC_DEBUG_THE_COMPILER
#if defined(_DEBUG)
#define CPPC_DEBUG_THE_COMPILER 1
#endif

#include "memory.h"
#include "atom.h"
#include "scanner.h"
#include "cpp.h"
#include "tokens.h"
#include "symbols.h"
#include "compile.h"
#if !defined(NO_PARSER)
#include "parser.h"
#endif

#if !defined(NULL)
#define NULL 0
#endif

#endif // !(defined(__SLGLOBALS_H)


    

