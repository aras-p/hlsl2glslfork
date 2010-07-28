#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <string>
#include <vector>
//#include <OpenGL/OpenGL.h>
//#include <AGL/agl.h>
#include <windows.h>
#include "../../include/hlsl2glsl.h"


typedef std::vector<std::string> StringVector;

static StringVector GetFiles (const std::string& path)
{
	StringVector res;

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA (path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return res;

	do {
		res.push_back (FindFileData.cFileName);
	} while (FindNextFileA (hFind, &FindFileData));

	FindClose (hFind);

	return res;
}

static bool ReadStringFromFile (const char* pathName, std::string& output)
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

/*
static void InitializeOpenGL ()
{	
	GLint attributes[16];
	int i = 0;
	attributes[i++]=AGL_RGBA;
	attributes[i++]=AGL_PIXEL_SIZE;
	attributes[i++]=32;
	attributes[i++]=AGL_NO_RECOVERY;
	attributes[i++]=AGL_NONE;
	
	AGLPixelFormat pixelFormat = aglChoosePixelFormat(NULL,0,attributes);
	AGLContext agl = aglCreateContext(pixelFormat, NULL);
	aglSetCurrentContext (agl);
}

bool CheckGLSL (bool vertex, const char* source, FILE* fout)
{
	//GLhandleARB prog = glCreateProgramObjectARB ();
	GLhandleARB shader = glCreateShaderObjectARB (vertex ? GL_VERTEX_SHADER_ARB : GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB (shader, 1, &source, NULL);
	glCompileShaderARB (shader);
	GLint status;
	glGetObjectParameterivARB (shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if (status == 0)
	{
		char log[4096];
		GLsizei logLength;
		glGetInfoLogARB (shader, sizeof(log), &logLength, log);
		fprintf (fout, "\n\n!!!! GLSL compile error: %s", log);
		return false;
	}
	
	return true;
}
*/

static bool TestFile (bool vertex, const std::string& inputPath, const std::string& outputPath, const std::string& errPath)
{
	std::string input;
	if (!ReadStringFromFile (inputPath.c_str(), input))
	{
		printf ("  failed to read input file\n");
		return false;
	}

	ShHandle parser = Hlsl2Glsl_ConstructParser (vertex ? EShLangVertex : EShLangFragment, 0);
	ShHandle translator = Hlsl2Glsl_ConstructTranslator (0);

	const char* sourceStr = input.c_str();

	bool res = true;

	if (Hlsl2Glsl_Parse (parser, &sourceStr, 1, 0))
	{
		static EAttribSemantic kAttribSemantic[] = {
			EAttrSemTangent,
		};
		static const char* kAttribString[] = {
			"TANGENT",
		};
		Hlsl2Glsl_SetUserAttributeNames (translator, kAttribSemantic, kAttribString, 1);
		if (Hlsl2Glsl_Translate (translator, &parser, 1, vertex ? "main" : NULL, vertex ? NULL : "main"))
		{
			std::string text = Hlsl2Glsl_GetShader (translator, vertex ? EShLangVertex : EShLangFragment);

			std::string output;
			ReadStringFromFile (outputPath.c_str(), output);

			if (text != output)
			{
				// write output
				FILE* f = fopen (errPath.c_str(), "wb");
				fwrite (text.c_str(), 1, text.size(), f);
				fclose (f);
				printf ("  does not match expected output\n");
				res = false;
			}
			//CheckGLSL (vertex, text, fout);
		}
		else
		{
			const char* infoLog = Hlsl2Glsl_GetInfoLog( translator );
			printf ("  translate error: %s\n", infoLog);
			res = false;
		}
	}
	else
	{
		const char* infoLog = Hlsl2Glsl_GetInfoLog( parser );
		printf ("  parse error: %s\n", infoLog);
		res = false;
	}

	Hlsl2Glsl_Destruct (parser);
	Hlsl2Glsl_Destruct (translator);

	return res;
}


int main (int argc, const char** argv)
{
	if (argc < 2)
	{
		printf ("USAGE: hlsl2glsltest testfolder\n");
		return 1;
	}

	Hlsl2Glsl_Initialize();

	std::string baseFolder = argv[1];

	static const char* kTypeName[2] = { "vertex", "fragment" };
	size_t tests = 0;
	size_t errors = 0;
	for (int type = 0; type < 2; ++type)
	{
		printf ("testing %s...\n", kTypeName[type]);
		std::string testFolder = baseFolder + "/" + kTypeName[type] + "/";
		StringVector inputFiles = GetFiles (testFolder + "*-in.txt");

		size_t n = inputFiles.size();
		tests += n;
		for (size_t i = 0; i < n; ++i)
		{
			std::string inname = inputFiles[i];
			printf ("test %s\n", inname.c_str());
			std::string outname = inname.substr (0,inname.size()-7) + "-out.txt";
			std::string errname = inname.substr (0,inname.size()-7) + "-res.txt";
			bool ok = TestFile (type==0, testFolder + inname, testFolder + outname, testFolder + errname);
			if (!ok)
			{
				++errors;
			}
		}
	}
	if (errors != 0)
		printf ("%i tests, %i FAILED\n", tests, errors);
	else
		printf ("%i tests succeeded\n", tests);

	return errors ? 1 : 0;
}
