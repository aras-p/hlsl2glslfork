// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef BISON_PARSER_H
# define BISON_PARSER_H

#ifndef yystypepp
typedef struct {
    int    sc_int;
    float  sc_fval;
    int    sc_ident;
	char   symbol_name[MAX_SYMBOL_NAME_LEN+1];
} yystypepp;

# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	CPP_AND_OP	        257
# define	CPP_SUB_ASSIGN	    259
# define	CPP_MOD_ASSIGN	    260
# define	CPP_ADD_ASSIGN	261
# define	CPP_DIV_ASSIGN	262
# define	CPP_MUL_ASSIGN	263
# define	CPP_EQ_OP	        264
# define    CPP_XOR_OP         265 
# define	ERROR_SY	    266
# define	CPP_FLOATCONSTANT	267
# define	CPP_GE_OP	        268
# define	CPP_RIGHT_OP        269
# define	CPP_IDENTIFIER	    270
# define	CPP_INTCONSTANT	    271
# define	CPP_LE_OP	        272
# define	CPP_LEFT_OP	        273
# define	CPP_DEC_OP	274
# define	CPP_NE_OP	        275
# define	CPP_OR_OP	        276
# define	CPP_INC_OP	    277
# define	CPP_STRCONSTANT	    278
# define	CPP_TYPEIDENTIFIER	279

# define	CPP_RIGHT_ASSIGN	    280
# define	CPP_LEFT_ASSIGN	    281
# define	CPP_AND_ASSIGN	282
# define	CPP_OR_ASSIGN  	283
# define	CPP_XOR_ASSIGN	284
# define	CPP_LEFT_BRACKET	285
# define	CPP_RIGHT_BRACKET	286
# define	CPP_LEFT_BRACE	287
# define	CPP_RIGHT_BRACE	288

# define	CPP_TOKENPASTE 	289

# define	FIRST_USER_TOKEN_SY	290

#endif /* not BISON_PARSER_H */
