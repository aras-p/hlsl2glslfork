/*-*-C++-*-

// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

*/
/* Based on
ANSI C grammar, Lex specification

In 1985, Jeff Lee published this Lex specification together with a Yacc 
grammar for the April 30, 1985 ANSI C draft.  Tom Stockfisch reposted 
both to net.sources in 1987; that original, as mentioned in the answer 
to question 17.25 of the comp.lang.c FAQ, can be ftp'ed from ftp.uu.net, 
file usenet/net.sources/ansi.c.grammar.Z. 

I intend to keep this version as close to the current C Standard grammar 
as possible; please let me know if you discover discrepancies. 

Jutta Degener, 1995 
*/

D           [0-9]
L           [a-zA-Z_]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+
O           [0-7]
F           [hHfF]
I           [uUlL]


%option nounput 
%{
#include <stdio.h>
#include <stdlib.h>
#include "ParseHelper.h"
#include "hlslang_tab.h"

/* windows only pragma */
#ifdef _MSC_VER
#pragma warning(disable : 4102)
#endif

int yy_input(char* buf, int max_size);
static TSourceLoc lexlineno = { 0, 0 };

#ifdef _WIN32
    extern int yyparse(TParseContext&);
    #define YY_DECL int yylex(YYSTYPE* pyylval, TParseContext& parseContext)    
#else
    extern int yyparse(void*);
    #define YY_DECL int yylex(YYSTYPE* pyylval, void* parseContextLocal)
    #define parseContext (*((TParseContext*)(parseContextLocal)))		
#endif
 
#define YY_INPUT(buf,result,max_size) (result = yy_input(buf, max_size))

%}

%option noyywrap
%option never-interactive
%option outfile="Gen_hlslang.cpp"
%x FIELDS


%%
<*>"//"[^\n]*"\n"     { /* ?? carriage and/or line-feed? */ };

"const"        {  pyylval->lex.line = lexlineno; return(CONST_QUAL); }
"static"       {  pyylval->lex.line = lexlineno; return(STATIC_QUAL); }
"uniform"      {  pyylval->lex.line = lexlineno; return(UNIFORM); }

"break"        {  pyylval->lex.line = lexlineno; return(BREAK); }
"continue"     {  pyylval->lex.line = lexlineno; return(CONTINUE); }
"do"           {  pyylval->lex.line = lexlineno; return(DO); }
"for"          {  pyylval->lex.line = lexlineno; return(FOR); }
"while"        {  pyylval->lex.line = lexlineno; return(WHILE); }

"if"           {  pyylval->lex.line = lexlineno; return(IF); }
"else"         {  pyylval->lex.line = lexlineno; return(ELSE); }

"in"           {  pyylval->lex.line = lexlineno; return(IN_QUAL); }
"out"          {  pyylval->lex.line = lexlineno; return(OUT_QUAL); }
"inout"        {  pyylval->lex.line = lexlineno; return(INOUT_QUAL); }

"float"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FLOAT_TYPE); }
"float1"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FLOAT_TYPE); }
"int"          {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(INT_TYPE); }
"int1"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(INT_TYPE); }
"void"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(VOID_TYPE); }
"bool"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(BOOL_TYPE); }
"bool1"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(BOOL_TYPE); }
"string"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(STRING_TYPE); }
"true"         {  pyylval->lex.line = lexlineno; pyylval->lex.b = true;  return(BOOLCONSTANT); }
"false"        {  pyylval->lex.line = lexlineno; pyylval->lex.b = false; return(BOOLCONSTANT); }

"discard"      {  pyylval->lex.line = lexlineno; return(DISCARD); }
"return"       {  pyylval->lex.line = lexlineno; return(RETURN); }

"float2x2"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX2x2); }
"float2x3"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX2x3); }
"float2x4"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX2x4); }

"float3x2"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX3x2); }
"float3x3"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX3x3); }
"float3x4"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX3x4); }

"float4x2"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX4x2); }
"float4x3"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX4x3); }
"float4x4"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(MATRIX4x4); }

"half2x2"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX2x2); }
"half2x3"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX2x3); }
"half2x4"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX2x4); }

"half3x2"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX3x2); }
"half3x3"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX3x3); }
"half3x4"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX3x4); }

"half4x2"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX4x2); }
"half4x3"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX4x3); }
"half4x4"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HMATRIX4x4); }

"fixed2x2"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX2x2); }
"fixed2x3"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX2x3); }
"fixed2x4"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX2x4); }

"fixed3x2"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX3x2); }
"fixed3x3"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX3x3); }
"fixed3x4"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX3x4); }

"fixed4x2"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX4x2); }
"fixed4x3"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX4x3); }
"fixed4x4"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FMATRIX4x4); }

"half"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HALF_TYPE); }
"half1"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(HALF_TYPE); }
"half2"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (HVEC2); }
"half3"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (HVEC3); }
"half4"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (HVEC4); }

"fixed"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FIXED_TYPE); }
"fixed1"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return(FIXED_TYPE); }
"fixed2"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (FVEC2); }
"fixed3"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (FVEC3); }
"fixed4"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (FVEC4); }

"float2"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (VEC2); }
"float3"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (VEC3); }
"float4"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (VEC4); }
"int2"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (IVEC2); }
"int3"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (IVEC3); }
"int4"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (IVEC4); }
"bool2"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (BVEC2); }
"bool3"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (BVEC3); }
"bool4"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (BVEC4); }

"vector"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (VECTOR); }
"matrix"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (MATRIX); }
"register"        {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return (REGISTER); }

"sampler1D"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLER1D; }
"sampler1DShadow" {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLER1DSHADOW; }
"sampler2D"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLER2D; }
"sampler2DShadow" {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLER2DSHADOW; }
"sampler2D_half"  {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLER2D_HALF; }
"sampler2D_float" {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLER2D_FLOAT; }
"sampler3D"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLER3D; }
"samplerRECT"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLERRECT; }
"samplerRECTShadow"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLERRECTSHADOW; }

"sampler"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLERGENERIC; }
"samplerCUBE"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLERCUBE; }
"samplerCUBE_half"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLERCUBE_HALF; }
"samplerCUBE_float"    {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLERCUBE_FLOAT; }

"texture"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return TEXTURE; }
"texture2D"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return TEXTURE; }
"texture3D"       {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return TEXTURE; }
"textureRECT"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return TEXTURE; }
"textureCUBE"     {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return TEXTURE; }
"sampler_state"   {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = true; return SAMPLERSTATE; }

"struct"       {  pyylval->lex.line = lexlineno; return(STRUCT); }

"asm"          {  PaReservedWord(); return 0; }

"class"        {  PaReservedWord(); return 0; }
"union"        {  PaReservedWord(); return 0; }
"enum"         {  PaReservedWord(); return 0; }
"typedef"      {  PaReservedWord(); return 0; }
"template"     {  PaReservedWord(); return 0; }
"this"         {  PaReservedWord(); return 0; }
"packed"       {  PaReservedWord(); return 0; }

"goto"         {  PaReservedWord(); return 0; }
"switch"       {  PaReservedWord(); return 0; }
"default"      {  PaReservedWord(); return 0; }

"inline"       {  /* just ignore it PaReservedWord(); return 0; */ }
"noinline"     {  /* just ignore it PaReservedWord(); return 0; */ }
"volatile"     {  PaReservedWord(); return 0; }
"public"       {  PaReservedWord(); return 0; }
"extern"       {  PaReservedWord(); return 0; }
"external"     {  PaReservedWord(); return 0; }
"interface"    {  PaReservedWord(); return 0; }

"long"         {  PaReservedWord(); return 0; }
"short"        {  PaReservedWord(); return 0; }
"double"       {  PaReservedWord(); return 0; }
"unsigned"     {  PaReservedWord(); return 0; }

"sampler3DRect"        {  PaReservedWord(); return 0; }

"sizeof"       {  PaReservedWord(); return 0; }
"cast"         {  PaReservedWord(); return 0; }

"namespace"    {  PaReservedWord(); return 0; }
"using"        {  PaReservedWord(); return 0; }

{L}({L}|{D})*       {  
   pyylval->lex.line = lexlineno; 
   pyylval->lex.string = NewPoolTString(yytext); 
   return PaIdentOrType(*pyylval->lex.string, parseContext, pyylval->lex.symbol); 
}

{D}+{E}{F}?           { pyylval->lex.line = lexlineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }
{D}+"."{D}*({E})?{F}? { pyylval->lex.line = lexlineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }
"."{D}+({E})?{F}?     { pyylval->lex.line = lexlineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }
{D}+{F}               { pyylval->lex.line = lexlineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }

0[xX]{H}+{I}?         { pyylval->lex.line = lexlineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }
0{O}+{I}?             { pyylval->lex.line = lexlineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }
0{D}+{I}?             { pyylval->lex.line = lexlineno; parseContext.error(lexlineno, "Invalid Octal number.", yytext, "", ""); parseContext.recover(); return 0;}
{D}+{I}?              { pyylval->lex.line = lexlineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }





"/*"            {  int ret = PaParseComment(pyylval->lex.line, parseContext); if (!ret) return ret; }   

"+="            {  pyylval->lex.line = lexlineno; return(ADD_ASSIGN); }
"-="            {  pyylval->lex.line = lexlineno; return(SUB_ASSIGN); }
"*="            {  pyylval->lex.line = lexlineno; return(MUL_ASSIGN); }
"/="            {  pyylval->lex.line = lexlineno; return(DIV_ASSIGN); }
"%="            {  pyylval->lex.line = lexlineno; return(MOD_ASSIGN); }
"<<="           {  pyylval->lex.line = lexlineno; return(LEFT_ASSIGN); }
">>="           {  pyylval->lex.line = lexlineno; return(RIGHT_ASSIGN); }
"&="            {  pyylval->lex.line = lexlineno; return(AND_ASSIGN); }
"^="            {  pyylval->lex.line = lexlineno; return(XOR_ASSIGN); }
"|="            {  pyylval->lex.line = lexlineno; return(OR_ASSIGN); }

"++"            {  pyylval->lex.line = lexlineno; return(INC_OP); }
"--"            {  pyylval->lex.line = lexlineno; return(DEC_OP); }
"&&"            {  pyylval->lex.line = lexlineno; return(AND_OP); }
"||"            {  pyylval->lex.line = lexlineno; return(OR_OP); }
"^^"            {  pyylval->lex.line = lexlineno; return(XOR_OP); }
"<="            {  pyylval->lex.line = lexlineno; return(LE_OP); }
">="            {  pyylval->lex.line = lexlineno; return(GE_OP); }
"=="            {  pyylval->lex.line = lexlineno; return(EQ_OP); }
"!="            {  pyylval->lex.line = lexlineno; return(NE_OP); }
"<<"            {  pyylval->lex.line = lexlineno; return(LEFT_OP); }
">>"            {  pyylval->lex.line = lexlineno; return(RIGHT_OP); }
";"         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = false; return(SEMICOLON); }
"{"      {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = false; return(LEFT_BRACE); }
"}"      {  pyylval->lex.line = lexlineno; return(RIGHT_BRACE); }
","         {  pyylval->lex.line = lexlineno; if (parseContext.inTypeParen) parseContext.lexAfterType = false; return(COMMA); }
":"         {  pyylval->lex.line = lexlineno; return(COLON); }
"="         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = false; return(EQUAL); }
"("         {  pyylval->lex.line = lexlineno; parseContext.lexAfterType = false; parseContext.inTypeParen = true; return(LEFT_PAREN); }
")"         {  pyylval->lex.line = lexlineno; parseContext.inTypeParen = false; return(RIGHT_PAREN); }
"["      {  pyylval->lex.line = lexlineno; return(LEFT_BRACKET); }
"]"      {  pyylval->lex.line = lexlineno; return(RIGHT_BRACKET); }
"."         { BEGIN(FIELDS);  return(DOT); }
"!"         {  pyylval->lex.line = lexlineno; return(BANG); }
"-"         {  pyylval->lex.line = lexlineno; return(DASH); }
"~"         {  pyylval->lex.line = lexlineno; return(TILDE); }
"+"         {  pyylval->lex.line = lexlineno; return(PLUS); }
"*"         {  pyylval->lex.line = lexlineno; return(STAR); }
"/"         {  pyylval->lex.line = lexlineno; return(SLASH); }
"%"         {  pyylval->lex.line = lexlineno; return(PERCENT); }
"<"         {  pyylval->lex.line = lexlineno; return(LEFT_ANGLE); }
">"         {  pyylval->lex.line = lexlineno; return(RIGHT_ANGLE); }
"|"         {  pyylval->lex.line = lexlineno; return(VERTICAL_BAR); }
"^"         {  pyylval->lex.line = lexlineno; return(CARET); }
"&"         {  pyylval->lex.line = lexlineno; return(AMPERSAND); }
"?"         {  pyylval->lex.line = lexlineno; return(QUESTION); }

\"[^\"]*\"  {  pyylval->lex.line = lexlineno; return(STRINGCONSTANT); }

<FIELDS>{L}({L}|{D})* { 
BEGIN(INITIAL);      
    pyylval->lex.line = lexlineno;     
    pyylval->lex.string = NewPoolTString(yytext); 
    return FIELD_SELECTION; }
<FIELDS>[ \t\v\f\r] {}


[ \t\v\n\f\r]   {  }
<*><<EOF>> { (&parseContext)->AfterEOF = true; yy_delete_buffer(YY_CURRENT_BUFFER); yyterminate();}
<*>.    { parseContext.infoSink.info << "FLEX: Unknown char " << yytext << "\n";
          return 0; }

%%



//Including Pre-processor.
extern "C" {
  #include "./preprocessor/preprocess.h"
} 

//
// The YY_INPUT macro just calls this.  Maybe this could be just put into
// the macro directly.
//

int yy_input(char* buf, int max_size)
{
	int len;

    if ((len = yylex_CPP(buf, max_size)) == 0)
        return 0;
    if (len >= max_size) 
        YY_FATAL_ERROR( "input buffer overflow, can't enlarge buffer because scanner uses REJECT" );

    //debug code to dump the token stream to stdout
    //buf[len] = '\0';
    //printf( ":: %s\n", buf);

    buf[len] = ' ';
	return len+1;
}


//
// Parse an array of strings using yyparse.  We set up globals used by
// yywrap.
//
// Returns 0 for success, as per yyparse().
//
int PaParseString(char* source, TParseContext& parseContextLocal)
{
    int sourceLen;
    
    ScanFromString(source);
    
    //Storing the Current Compiler Parse context into the cpp structure.
	cpp->pC = (void*)&parseContextLocal;
	
    if (!source) {
        parseContextLocal.error(gNullSourceLoc, "Null shader source string", "", "");
        parseContextLocal.recover();
        return 1;
    }
    
    sourceLen = (int) strlen(source);
    
    yyrestart(0);
    (&parseContextLocal)->AfterEOF = false;
	lexlineno.file = NULL;
    lexlineno.line = 1;
   
    if (sourceLen >= 0) {
        #ifdef _WIN32
            yyparse(parseContextLocal);
        #else
            yyparse((void*)(&parseContextLocal));
        #endif
        if (cpp->CompileError == 1 || parseContextLocal.recoveredFromError || parseContextLocal.numErrors > 0)
             return 1;
        else
             return 0;
    }
    else
        return 0;
}

void yyerror(const char *s)
{
    if (((TParseContext *)cpp->pC)->AfterEOF) {
        if (cpp->tokensBeforeEOF == 1) {
            GlobalParseContext->error(lexlineno, "syntax error", "pre-mature EOF", s, "");
            GlobalParseContext->recover();
        }
    } else {
        GlobalParseContext->error(lexlineno, "syntax error", yytext, s, "");
        GlobalParseContext->recover();
    }            
}

void PaReservedWord()
{
    GlobalParseContext->error(lexlineno, "Reserved word.", yytext, "", "");
    GlobalParseContext->recover();
}

int PaIdentOrType(TString& id, TParseContext& parseContextLocal, TSymbol*& symbol)
{
    symbol = parseContextLocal.symbolTable.find(id);
    if (parseContextLocal.lexAfterType == false && symbol && symbol->isVariable()) {
        TVariable* variable = static_cast<TVariable*>(symbol);
        if (variable->isUserType()) {
            parseContextLocal.lexAfterType = true;
            return TYPE_NAME;
        }
    }
    
    return IDENTIFIER;
}

int PaParseComment(TSourceLoc &lineno, TParseContext& parseContextLocal)
{
    int transitionFlag = 0;
    int nextChar;
    
    while (transitionFlag != 2) {
        nextChar = yyinput();
        if (nextChar == '\n')
             lineno.line++;
        switch (nextChar) {
        case '*' :
            transitionFlag = 1;
            break;
        case '/' :  /* if star is the previous character, then it is the end of comment */
            if (transitionFlag == 1) {
                return 1 ;
            }
            break;
        case EOF :
            /* Raise error message here */
            parseContextLocal.error(lexlineno, "End of shader found before end of comment.", "", "", "");
            GlobalParseContext->recover();
            return YY_NULL; 
        default :  /* Any other character will be a part of the comment */
            transitionFlag = 0;
        }
    }
    return 1;
}

extern "C" {

void CPPWarningToInfoLog(const char *msg)
{
    ((TParseContext *)cpp->pC)->infoSink.info.message(EPrefixWarning, msg, lexlineno); 
}

void CPPShInfoLogMsg(const char *msg)
{
    ((TParseContext *)cpp->pC)->error(lexlineno,"", "",msg,"");
    GlobalParseContext->recover();
}

void CPPErrorToInfoLog(char *msg)
{
    ((TParseContext *)cpp->pC)->error(lexlineno,"syntax error", "",msg,"");
    GlobalParseContext->recover();
}

void SetLineNumber(TSourceLoc line)
{
    lexlineno.file = NULL;
    lexlineno.line = line.line;
    
    if(line.file && line.file[0])
    {
	    // GLSL does not permit quoted strings in #line directives
    
	    if(line.file[0] == '"')
	    {
	        TString stripped(line.file + 1);
	        size_t len = stripped.size();
	        if(stripped[len - 1] == '"')
	        {
	            stripped.resize(len - 1);
	        }
	        
	        lexlineno.file = NewPoolTString(stripped.c_str())->c_str();
	    }
	    else
	    {
	        lexlineno.file = NewPoolTString(line.file)->c_str();
	    }
	}
}

TSourceLoc GetLineNumber(void)
{
    return lexlineno;
}

void IncLineNumber(void)
{
    ++lexlineno.line;
}

void DecLineNumber(void)
{
    if (lexlineno.line > 0)
        --lexlineno.line;
}

void StoreStr(char *string)
{
    TString strSrc;
    strSrc = TString(string);

    ((TParseContext *)cpp->pC)->HashErrMsg = ((TParseContext *)cpp->pC)->HashErrMsg + " " + strSrc;
}

const char* GetStrfromTStr(void)
{
    cpp->ErrMsg = (((TParseContext *)cpp->pC)->HashErrMsg).c_str();
    return cpp->ErrMsg;
}

void ResetTString(void)
{
    ((TParseContext *)cpp->pC)->HashErrMsg = "";
}

}  // extern "C"

void setInitialState()
{
    yy_start = 1;
}
