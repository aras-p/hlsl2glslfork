// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#if !defined(__COMPILE_H)
#define __COMPILE_H 1

int InitCPPStruct(void);

typedef struct Options_Rec{
    int Quiet;
} Options;

struct CPPStruct_Rec {
    // Public members
    SourceLoc *pLastSourceLoc;  // Set at the start of each statement by the tree walkers
    Options options;            // Compile options and parameters

    // Private members
    SourceLoc lastSourceLoc;

    // Scanner data:

    SourceLoc *tokenLoc;        // Source location of most recent token seen by the scanner
    int mostRecentToken;        // Most recent token seen by the scanner
    InputSrc *currentInput;
    int previous_token;
    int notAVersionToken;      // used to make sure that #version is the first token seen in the file, if present
    
	void *pC;                   // storing the parseContext of the compile object in cpp.  
     
    // Private members:
    SourceLoc ltokenLoc;
	int ifdepth;                //current #if-#else-#endif nesting in the cpp.c file (pre-processor)    
    int elsedepth[64];          //Keep a track of #if depth..Max allowed is 64.   
    int elsetracker;            //#if-#else and #endif constructs...Counter.
    const char *ErrMsg;
    int CompileError;           //Indicate compile error when #error, #else,#elif mismatch.

    //
    // Globals used to communicate between PaParseStrings() and yy_input()and 
    // also across the files.(gen_glslang.cpp and scanner.c)
    //
    int    PaWhichStr;            // which string we're parsing
    int*   PaStrLen;              // array of lengths of the PaArgv strings
    int    PaArgc;                // count of strings in the array
    char** PaArgv;                // our array of strings to parse    
    unsigned int tokensBeforeEOF : 1;
};

#endif // !defined(__COMPILE_H)
