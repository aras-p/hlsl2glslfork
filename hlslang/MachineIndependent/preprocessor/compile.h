// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#if !defined(__COMPILE_H)
#define __COMPILE_H 1


struct CPPStruct_Rec {
    // Scanner data:

    InputSrc *currentInput;
    int previous_token;
    
	void *pC;                   // storing the parseContext of the compile object in cpp.  
     
    // Private members:
	int ifdepth;                //current #if-#else-#endif nesting in the cpp.c file (pre-processor)    
    int elsedepth[64];          //Keep a track of #if depth..Max allowed is 64.   
    int elsetracker;            //#if-#else and #endif constructs...Counter.
    const char *ErrMsg;
    int CompileError;           //Indicate compile error when #error, #else,#elif mismatch.

    //
    // Globals used to communicate between PaParseStrings() and yy_input()and 
    // also across the files.(gen_glslang.cpp and scanner.c)
    //
    unsigned int tokensBeforeEOF : 1;
};

#endif // !defined(__COMPILE_H)
