#include <cstdio>
#include <string>
#include "../../include/hlsl2glsl.h"

bool ReadStringFromFile (const char* pathName, std::string& output)
{
	FILE* file = fopen( pathName, "rb" );
	if (file == NULL)
		return false;
	
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (length < 0)
	{
		fclose( file );
		return false;
	}
	
	output.resize(length);
	int readLength = fread(&*output.begin(), 1, length, file);
	
	fclose(file);
	
	if (readLength != length)
	{
		output.clear();
		return false;
	}
	
	return true;
}


int main (int argc, char * const argv[])
{
	if (argc < 4)
	{
		printf ("USAGE: hlsl2glsltest v|f input.txt output.txt\n");
		return 1;
	}
	std::string input;
	if (!ReadStringFromFile (argv[2], input))
	{
		printf ("ERROR: input file %s not found\n", argv[2]);
		return 1;
	}
	
	Hlsl2Glsl_Initialize();
	
	bool vertex = !strcmp(argv[1],"v");
	
	ShHandle parser = Hlsl2Glsl_ConstructParser (vertex ? EShLangVertex : EShLangFragment, 0);
	ShHandle translator = Hlsl2Glsl_ConstructTranslator( 0 );
	
	const char* sourceStr = input.c_str();
	FILE* fout = fopen (argv[3], "wb");
	if (!fout)
	{
		printf ("ERROR: failed to write to %s\n", argv[3]);
		return 1;
	}
	
	if (Hlsl2Glsl_Parse( parser, &sourceStr, 1, 0 ))
	{
		static EAttribSemantic kAttribSemantic[] = {
			EAttrSemTangent,
		};
		static const char* kAttribString[] = {
			"TANGENT",
		};
		Hlsl2Glsl_SetUserAttributeNames (translator, kAttribSemantic, kAttribString, 1);
		if (Hlsl2Glsl_Translate (translator, &parser, 1, vertex ? "main" : NULL, vertex ? NULL : "main", 0))
		{
			const char* text = Hlsl2Glsl_GetShader (translator, vertex ? EShLangVertex : EShLangFragment);
			fprintf (fout, "%s", text);
		}
		else
		{
			const char* infoLog = Hlsl2Glsl_GetInfoLog( translator );
			fprintf (fout, "Translate error: %s", infoLog);
		}
	}
	else
	{
		const char* infoLog = Hlsl2Glsl_GetInfoLog( parser );
		fprintf (fout, "Parse error: %s", infoLog);
	}
	fclose (fout);
    return 0;
}
