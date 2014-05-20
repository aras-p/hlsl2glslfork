typedef union {
    struct {
        TSourceLoc line;
        union {
            TString *string;
            float f;
            int i;
            bool b;
        };
        TSymbol* symbol;
    } lex;
    struct {
        TSourceLoc line;
        TOperator op;
        union {
            TIntermNode* intermNode;
            TIntermNodePair nodePair;
            TIntermTyped* intermTypedNode;
            TIntermAggregate* intermAggregate;
			TIntermTyped* intermDeclaration;
        };
        union {
            TPublicType type;
            TQualifier qualifier;
            TFunction* function;
            TParameter param;
            TTypeLine typeLine;
            TTypeList* typeList;
	    TAnnotation* ann;
	    TTypeInfo* typeInfo;
        };
    } interm;
} YYSTYPE;
#define	CONST_QUAL	258
#define	STATIC_QUAL	259
#define	BOOL_TYPE	260
#define	FLOAT_TYPE	261
#define	INT_TYPE	262
#define	STRING_TYPE	263
#define	FIXED_TYPE	264
#define	HALF_TYPE	265
#define	BREAK	266
#define	CONTINUE	267
#define	DO	268
#define	ELSE	269
#define	FOR	270
#define	IF	271
#define	DISCARD	272
#define	RETURN	273
#define	BVEC2	274
#define	BVEC3	275
#define	BVEC4	276
#define	IVEC2	277
#define	IVEC3	278
#define	IVEC4	279
#define	VEC2	280
#define	VEC3	281
#define	VEC4	282
#define	HVEC2	283
#define	HVEC3	284
#define	HVEC4	285
#define	FVEC2	286
#define	FVEC3	287
#define	FVEC4	288
#define	MATRIX2x2	289
#define	MATRIX2x3	290
#define	MATRIX2x4	291
#define	MATRIX3x2	292
#define	MATRIX3x3	293
#define	MATRIX3x4	294
#define	MATRIX4x2	295
#define	MATRIX4x3	296
#define	MATRIX4x4	297
#define	HMATRIX2x2	298
#define	HMATRIX2x3	299
#define	HMATRIX2x4	300
#define	HMATRIX3x2	301
#define	HMATRIX3x3	302
#define	HMATRIX3x4	303
#define	HMATRIX4x2	304
#define	HMATRIX4x3	305
#define	HMATRIX4x4	306
#define	FMATRIX2x2	307
#define	FMATRIX2x3	308
#define	FMATRIX2x4	309
#define	FMATRIX3x2	310
#define	FMATRIX3x3	311
#define	FMATRIX3x4	312
#define	FMATRIX4x2	313
#define	FMATRIX4x3	314
#define	FMATRIX4x4	315
#define	IN_QUAL	316
#define	OUT_QUAL	317
#define	INOUT_QUAL	318
#define	UNIFORM	319
#define	STRUCT	320
#define	VOID_TYPE	321
#define	WHILE	322
#define	SAMPLER1D	323
#define	SAMPLER2D	324
#define	SAMPLER3D	325
#define	SAMPLERCUBE	326
#define	SAMPLER1DSHADOW	327
#define	SAMPLER2DSHADOW	328
#define	SAMPLERRECTSHADOW	329
#define	SAMPLERRECT	330
#define	SAMPLER2D_HALF	331
#define	SAMPLER2D_FLOAT	332
#define	SAMPLERCUBE_HALF	333
#define	SAMPLERCUBE_FLOAT	334
#define	SAMPLERGENERIC	335
#define	VECTOR	336
#define	MATRIX	337
#define	REGISTER	338
#define	TEXTURE	339
#define	SAMPLERSTATE	340
#define	IDENTIFIER	341
#define	TYPE_NAME	342
#define	FLOATCONSTANT	343
#define	INTCONSTANT	344
#define	BOOLCONSTANT	345
#define	STRINGCONSTANT	346
#define	FIELD_SELECTION	347
#define	LEFT_OP	348
#define	RIGHT_OP	349
#define	INC_OP	350
#define	DEC_OP	351
#define	LE_OP	352
#define	GE_OP	353
#define	EQ_OP	354
#define	NE_OP	355
#define	AND_OP	356
#define	OR_OP	357
#define	XOR_OP	358
#define	MUL_ASSIGN	359
#define	DIV_ASSIGN	360
#define	ADD_ASSIGN	361
#define	MOD_ASSIGN	362
#define	LEFT_ASSIGN	363
#define	RIGHT_ASSIGN	364
#define	AND_ASSIGN	365
#define	XOR_ASSIGN	366
#define	OR_ASSIGN	367
#define	SUB_ASSIGN	368
#define	LEFT_PAREN	369
#define	RIGHT_PAREN	370
#define	LEFT_BRACKET	371
#define	RIGHT_BRACKET	372
#define	LEFT_BRACE	373
#define	RIGHT_BRACE	374
#define	DOT	375
#define	COMMA	376
#define	COLON	377
#define	EQUAL	378
#define	SEMICOLON	379
#define	BANG	380
#define	DASH	381
#define	TILDE	382
#define	PLUS	383
#define	STAR	384
#define	SLASH	385
#define	PERCENT	386
#define	LEFT_ANGLE	387
#define	RIGHT_ANGLE	388
#define	VERTICAL_BAR	389
#define	CARET	390
#define	AMPERSAND	391
#define	QUESTION	392

