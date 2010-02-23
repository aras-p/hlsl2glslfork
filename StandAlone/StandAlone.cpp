
//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//

//
//Copyright (C) 2005-2006  ATI Research, Inc.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of ATI Research, Inc. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "./../hlslang/Public/HLSL2GLSL.h"

#ifdef _WIN32
   #include <windows.h>
   #include <psapi.h>
#else
   #include <stdlib.h>
#endif

extern "C" {
   SH_IMPORT_EXPORT void ShOutputHtml();
}

//
// Return codes from main.
//
enum TFailCode
{
   ESuccess = 0,
   EFailUsage,
   EFailParse,
   EFailTranslate,
   EFailParserCreate,
   EFailTranslatorCreate
};

//
// Function Prototypes
//
static EShLanguage FindLanguage(char *lang);
bool ParseFile(char *fileName, ShHandle, int);
void usage();
void FreeFileData(char *data);
char* ReadFileData(char *fileName);
void InfoLogMsg(char* msg, const char* name, const int num);
bool SaveShader( const char*, ShHandle, EShLanguage);
bool SaveUniforms( const char*, ShHandle);
bool LoadUserAttribConfigFile ( char* configFileName, EAttribSemantic attribSemanticEnums[], 
                                char* attribSemanticNames[], int* attribSemanticCount );


int C_DECL main(int argc, char* argv[])
{
   int numParsers = 0;
   bool parseFailed = false;
   bool translateFailed = false;
   int debugOptions = 0;
   int i;
   char *fEntry = 0;
   char *vEntry = 0;
   char *fOut = 0;
   char *vOut = 0;
   char *uOut = 0;
   char *shaderHeader = 0;

   ShHandle    translator = 0;
   ShHandle    uniformMap = 0;
   ShHandle    parsers[EShLangCount];
   bool        bUseUserVaryings = false;
   int             attribSemanticCount = 0;
   EAttribSemantic attribSemanticEnums[EAttrSemCount];
   char*           attribSemanticNames[EAttrSemCount];

   for ( int i = 0; i < EAttrSemCount; i++ )
   {
      attribSemanticEnums[i] = EAttrSemUnknown;
      attribSemanticNames[i] = NULL;
   }

   Hlsl2Glsl_Initialize();

#ifdef _WIN32
   __try {
#endif
      argc--;
      argv++;    
      for (; argc >= 1; argc--, argv++)
      {
         if (argv[0][0] == '-' || argv[0][0] == '/')
         {
            switch (argv[0][1])
            {
            case 'f': fEntry = (++argv)[0]; argc--;             break;
            case 'v': vEntry = (++argv)[0]; argc--;             break;
            case 'o':
               if (strlen(argv[0]) > 2)
               {
                  switch (argv[0][2])
                  {
                  case 'f':
                     fOut = (++argv)[0]; argc--;
                     break;
                  case 'v':
                     vOut = (++argv)[0]; argc--;
                     break;
                  case 'u':
                     uOut = (++argv)[0]; argc--;
                     break;
                  default: 
                     usage();
                     return EFailUsage;
                     break;
                  }
               }
               break;
            case 'i':
               if (strlen(argv[0]) > 2)
               {
                  EShLanguage language = EShLangFragment;

                  switch (argv[0][2])
                  {
                  case 'f':
                     language = EShLangFragment;
                     break;
                  case 'v':
                     language = EShLangVertex;
                     break;
                  default: 
                     usage();
                     return EFailUsage;
                     break;
                  }
                  argv++;
                  argc--;

                  parsers[numParsers] = Hlsl2Glsl_ConstructParser(language, debugOptions);
                  if (parsers[numParsers] == 0)
                     return EFailParserCreate;
                  ++numParsers;

                  if (! ParseFile(argv[0], parsers[numParsers-1], debugOptions))
                     parseFailed = true;
               }
               break;
            case 'u':
               bUseUserVaryings = true;
               break;
            case 'a':
               {
                  char *configFileName = (++argv)[0];
                  argc--;

                  // Load a configuration file specifying user attributes
                  if ( !LoadUserAttribConfigFile ( configFileName, attribSemanticEnums, attribSemanticNames, &attribSemanticCount ) )
                  {
                     printf ( "Error loading attribute configuration file '%s'.\n", configFileName );
                     return EFailUsage;
                  }                  
               }
               break;
            case 'h':
               {
                  char *shaderHeaderName = (++argv)[0];
                  argc--;

                  shaderHeader = ReadFileData ( shaderHeaderName );
               }
               break;

            default:  
               usage();                       
               return EFailUsage;
            }
         }
         else
         {
            parsers[numParsers] = Hlsl2Glsl_ConstructParser(FindLanguage(argv[0]), debugOptions);
            if (parsers[numParsers] == 0)
               return EFailParserCreate;
            ++numParsers;

            if (! ParseFile(argv[0], parsers[numParsers-1], debugOptions))
               parseFailed = true;
         }
      }

      if (!numParsers)
      {
         usage();
         return EFailUsage;
      }

      translator = Hlsl2Glsl_ConstructTranslator(debugOptions);
      if (translator == 0)
         return EFailTranslatorCreate;

      if ( bUseUserVaryings )
      {
         // Set the translator up to use user varyings
         Hlsl2Glsl_UseUserVaryings ( translator, bUseUserVaryings );
      }

      if ( shaderHeader != NULL )
      {
         // Set a shader header if requested
         Hlsl2Glsl_SetShaderHeader ( translator, true, shaderHeader );
      }


      if ( attribSemanticCount > 0 )
      {
         // Use user attributes if the user specified user attribute configuration file
         if ( !Hlsl2Glsl_SetUserAttributeNames ( translator, attribSemanticEnums, (const char**)&attribSemanticNames[0], attribSemanticCount ) )
         {
            printf ("ERROR: Setting user attribute names\n");                        
         }
      }

      

      if (numParsers > 0)
      {
         if (! Hlsl2Glsl_Translate(translator, parsers, numParsers, vEntry, fEntry))
            translateFailed = true;
      }

      for (i = 0; i < numParsers; ++i)
      {
         InfoLogMsg("BEGIN", "COMPILER", i);
         puts(Hlsl2Glsl_GetInfoLog(parsers[i]));
         InfoLogMsg("END", "COMPILER", i);
      }

      InfoLogMsg("BEGIN", "LINKER", -1);
      puts(Hlsl2Glsl_GetInfoLog(translator));
      InfoLogMsg("END", "LINKER", -1);

      if (!translateFailed )
      {
         if (fOut)
            SaveShader( fOut, translator, EShLangFragment);
         if (vOut)
            SaveShader( vOut, translator, EShLangVertex);
         if (uOut)
            SaveUniforms( uOut, translator);
      }

#ifdef _WIN32
   } __finally {    
#endif    
      for (i = 0; i < numParsers; ++i)
         Hlsl2Glsl_Destruct(parsers[i]);

      Hlsl2Glsl_Destruct(translator);
      Hlsl2Glsl_Destruct(uniformMap);

      for ( int i = 0; i < EAttrSemCount; i++ )
      {
         if ( attribSemanticNames[i] != NULL )
         {
            delete[] attribSemanticNames[i];
            attribSemanticNames[i] = NULL;
         }
      }

      if ( shaderHeader )
      {
         FreeFileData ( shaderHeader );
      }


#ifdef _WIN32        
   }
#endif

   if (parseFailed)
      return EFailParse;
   if (translateFailed)
      return EFailTranslate;

   return 0;
}

//
//   Deduce the language from the filename.  Files must end in one of the
//   following extensions:
//
//   .frag*    = fragment programs
//   .vert*    = vertex programs
//   .pack*    = pack programs
//   .unpa*    = unpack pragrams
//
static EShLanguage FindLanguage(char *name)
{
   if (!name)
      return EShLangFragment;

   char *ext = strrchr(name, '.');

   if (ext && strcmp(ext, ".sl") == 0)
      for (; ext > name && ext[0] != '.'; ext--);

   if (ext = strrchr(name, '.'))
   {
      if (strncmp(ext, ".frag", 4) == 0) return EShLangFragment;
      if (strncmp(ext, ".vert", 4) == 0) return EShLangVertex;
   }

   return EShLangFragment;
}


//
//   Read a file's data into a string, and parse it using Hlsl2Glsl_Parse
//
bool ParseFile(char *fileName, ShHandle parser, int debugOptions)
{
   int ret;
   char *data = ReadFileData(fileName);

   if (!data)
      return false;

   const char *dataPtr[1] = { data};
   ret = Hlsl2Glsl_Parse(parser, dataPtr, 1, debugOptions);

   FreeFileData(data);

   return ret ? true : false;
}

//
//   Load an (optional) configuration file specifying which user attribute names to use
//
bool LoadUserAttribConfigFile ( char* configFileName, EAttribSemantic attribSemanticEnums[], 
                                char* attribSemanticNames[], int* attribSemanticCount )
{
   static char *attribString[EAttrSemCount] =
   {
      "",
      "EAttrSemPosition",
      "EAttrSemNormal",
      "EAttrSemColor0",
      "EAttrSemColor1",
      "EAttrSemColor2",
      "EAttrSemColor3",
      "EAttrSemTex0",
      "EAttrSemTex1",
      "EAttrSemTex2",
      "EAttrSemTex3",
      "EAttrSemTex4",
      "EAttrSemTex5",
      "EAttrSemTex6",
      "EAttrSemTex7",
      "EAttrSemTex8",
      "EAttrSemTex9",
      "EAttrSemTangent",
      "EAttrSemBinormal",
      "EAttrSemBlendWeight",
      "EAttrSemBlendIndices",
      "EAttrSemDepth",
      "EAttrSemUnknown",   
   };
   char *data = ReadFileData(configFileName);
   int semanticCount = 0;

   if (!data)
      return false;

   char *semToken = strtok ( data, " \n" );
   while ( semToken != NULL )
   {      
      char *attrToken = strtok ( NULL, " \n" );

      if ( attrToken == NULL )
      {
         printf ("ERROR: Encountered invalid token in file '%s'\n", configFileName );
         FreeFileData(data);
         return false;
      }

      bool bAttribSemFound = false;
      EAttribSemantic semantic;

      for ( int i = 0; i < EAttrSemCount; i++ )
      {
         if ( !strcmp ( semToken, attribString[i] ) )
         {
            bAttribSemFound = true;
            semantic = (EAttribSemantic) i;
            break;
         }
      }

      if ( !bAttribSemFound )
      {
         printf ("ERROR: Attribute semantic '%s' in file '%s' not found. \n", semToken, configFileName );
      }
      else
      {
         if ( semanticCount < EAttrSemCount )
         {
            attribSemanticEnums[semanticCount] = semantic;
            attribSemanticNames[semanticCount] = new char[strlen(attrToken) + 1];
            strcpy ( attribSemanticNames[semanticCount], attrToken );
            semanticCount++;
         }
         else
         {
            printf ( "WARNING: Ignoring attribute semantic '%s' in file '%s' because there are too many semantics\n", semToken, configFileName );
         }
      }

      semToken = strtok ( NULL, " \n" );
   }

   *attribSemanticCount = semanticCount;
   FreeFileData(data);

   return true;
}


//
//   print usage to stdout
//
void usage()
{
   printf("USAGE:\n");
   printf("  HLSL2GLSL [-f -v -of -ov -ou -if -iv -u -a -h] file1 file2 ...\n\n");
   printf("OPTIONS:\n");
   printf("  -f function  : Identifies the name of the fragment shader main entry point\n");
   printf("  -v function  : Identifies the name of the vertex shader main entry point\n");
   printf("  -if filename : Specifies the input filename for the HLSL fragment shader.\n");
   printf("  -iv filename : Specifies the input filename for the HLSL vertex shader.\n");
   printf("  -ov filename : Specifies the GLSL output filename for the vertex shader.\n");
   printf("  -of filename : Specifies the GLSL output filename for the fragment shader.\n");
   printf("  -ou filename : Specifies the output filename for the list of uniforms.\n");
   printf("  -u           : Specifies that the translator will use user varyings.\n");
   printf("  -a filename  : Specifies a user attribute config file (see documentation)\n");   
   printf("  -h filename  : Specifies a file whose contents will be preprended to the GLSL\n\n");
   printf("EXAMPLE:\n");
   printf("  HLSL2GLSL -v VSMain -f FSMain -of fs.glsl -ov vs.glsl -if fs.hlsl -iv vs.hlsl\n");
}


//
//   Malloc a string of sufficient size and read a string into it.
//
char* ReadFileData(char *fileName) 
{
   FILE *in    = fopen(fileName, "r");
   char *fdata;
   int count = 0;

   if (!in)
   {
      printf("Error: unable to open input file: %s\n", fileName);
      return 0;
   }

   while (fgetc(in) != EOF)
      count++;

   fseek(in, 0, SEEK_SET);


   if (!(fdata = (char *)malloc(count+2)))
   {
      printf("Error allocating memory\n");
      return 0;
   }
   if (fread(fdata,1,count, in)!=count)
   {
      printf("Error reading input file: %s\n", fileName);
      return 0;
   }
   fdata[count] = '\0';
   fclose(in);
   return fdata;
}


//
// Free the file data
//
void FreeFileData(char *data)
{
   free(data);
}



void InfoLogMsg(char* msg, const char* name, const int num)
{
   printf(num >= 0 ? "#### %s %s %d INFO LOG ####\n" :
          "#### %s %s INFO LOG ####\n", msg, name, num);
}

//
// Save the shader to the file
//
bool SaveShader( const char* fn, ShHandle handle, EShLanguage lang)
{
   FILE* fp = fopen( fn, "w");

   //__asm int 3;

   if (!fp)
      return false; // failed to open file

   const char* text = Hlsl2Glsl_GetShader( handle, lang);

   if (text)
      fprintf( fp, "%s", text);

   fclose(fp);
   return !text;
}


//
// Save the uniform info to a file
//
bool SaveUniforms( const char* fn, ShHandle handle)
{
   //
   // Uniform type conversion string
   //
   const char typeStrings[][32] = 
   {
      "void",
      "bool",
      "bvec2",
      "bvec3",
      "bvec4",
      "int",
      "ivec2",
      "ivec3",
      "ivec4",
      "float",
      "vec2",
      "vec3",
      "vec4",
      "mat2",
      "mat3",
      "mat4",
      "sampler",
      "sampler1D",
      "sampler2D",
      "sampler3D",
      "samplerCube",
      "struct"
   };

   //
   // Uniform element count lookup table by uniform type
   //
   const int elementCount[] = 
   {
      0,
      1,
      2,
      3,
      4,
      1,
      2,
      3,
      4,
      1,
      2,
      3,
      4,
      4,
      9,
      16,
      0,
      0,
      0,
      0,
      0,
      0
   };

   FILE* fp = fopen( fn, "w");

   if (!fp)
      return false; // failed to open file

   const ShUniformInfo* uniforms = Hlsl2Glsl_GetUniformInfo( handle);
   const int nUniforms = Hlsl2Glsl_GetUniformCount( handle);

   for (int ii=0; ii<nUniforms; ii++)
   {
      fprintf( fp, "%s %s", typeStrings[uniforms[ii].type], uniforms[ii].name);
      if ( uniforms[ii].arraySize)
         fprintf( fp, "[%d]", uniforms[ii].arraySize);
      if (uniforms[ii].semantic)
         fprintf( fp, " : %s", uniforms[ii].semantic);

      if (uniforms[ii].init)
      {
         int as = uniforms[ii].arraySize;
         as = (as) ? as : 1;
         fprintf( fp, " =\n  {");
         for ( int jj=0; jj < elementCount[uniforms[ii].type]*as; jj++)
         {
            fprintf( fp, " %f,", uniforms[ii].init[jj]);
         }
         fprintf( fp, "}\n");
      }
      else
         fprintf( fp, "\n");
   }

   fclose(fp);
   return true;
}