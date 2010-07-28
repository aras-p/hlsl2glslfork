#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <windows.h>
#include <gl/GL.h>

extern "C" {
typedef char GLcharARB;		/* native character */
typedef unsigned int GLhandleARB;	/* shader object handle */
#define GL_VERTEX_SHADER_ARB              0x8B31
#define GL_FRAGMENT_SHADER_ARB            0x8B30
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
typedef void (WINAPI * PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
typedef GLhandleARB (WINAPI * PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (WINAPI * PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef void (WINAPI * PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
typedef void (WINAPI * PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef void (WINAPI * PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
static PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
static PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
static PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
static PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
static PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
}


#else
#include <OpenGL/OpenGL.h>
#include <AGL/agl.h>
#include <dirent.h>
#endif
#include "../../include/hlsl2glsl.h"

bool EndsWith (const std::string& str, const std::string& sub)
{
	return (str.size() >= sub.size()) && (strncmp (str.c_str()+str.size()-sub.size(), sub.c_str(), sub.size())==0);
}


typedef std::vector<std::string> StringVector;

static StringVector GetFiles (const std::string& folder, const std::string& endsWith)
{
	StringVector res;

	#ifdef _MSC_VER
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA ((folder+"/*"+endsWith).c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return res;

	do {
		res.push_back (FindFileData.cFileName);
	} while (FindNextFileA (hFind, &FindFileData));

	FindClose (hFind);
	
	#else
	
	DIR *dirp;
	struct dirent *dp;

	if ((dirp = opendir(folder.c_str())) == NULL)
		return res;

	while ( (dp = readdir(dirp)) )
	{
		std::string fname = dp->d_name;
		if (fname == "." || fname == "..")
			continue;
		if (!EndsWith (fname, endsWith))
			continue;
		res.push_back (fname);
	}
	closedir(dirp);
	
	#endif

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

static bool InitializeOpenGL ()
{
	bool hasGLSL = false;

#ifdef _MSC_VER
	// setup minimal required GL
	HWND wnd = CreateWindowA(
		"STATIC",
		"GL",
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |	WS_CLIPCHILDREN,
		0, 0, 16, 16,
		NULL, NULL,
		GetModuleHandle(NULL), NULL );
	HDC dc = GetDC( wnd );

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
		PFD_TYPE_RGBA, 32,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		16, 0,
		0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	int fmt = ChoosePixelFormat( dc, &pfd );
	SetPixelFormat( dc, fmt, &pfd );

	HGLRC rc = wglCreateContext( dc );
	wglMakeCurrent( dc, rc );

#else
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

#endif
	
	// check if we have GLSL
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	hasGLSL = strstr(extensions, "GL_ARB_shader_objects") && strstr(extensions, "GL_ARB_vertex_shader") && strstr(extensions, "GL_ARB_fragment_shader");
	
#ifdef _MSC_VER
	if (hasGLSL)
	{
		glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
		glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
		glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
		glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
		glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
		glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
	}
#endif
	

	return hasGLSL;
}

static bool CheckGLSL (bool vertex, const char* source)
{
	GLhandleARB shader = glCreateShaderObjectARB (vertex ? GL_VERTEX_SHADER_ARB : GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB (shader, 1, &source, NULL);
	glCompileShaderARB (shader);
	GLint status;
	glGetObjectParameterivARB (shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	bool res = true;
	if (status == 0)
	{
		char log[4096];
		GLsizei logLength;
		glGetInfoLogARB (shader, sizeof(log), &logLength, log);
		printf ("  glsl compile error:\n%s\n", log);
		res = false;
	}
	glDeleteObjectARB (shader);
	return res;
}

static bool TestFile (bool vertex, const std::string& inputPath, const std::string& outputPath, const std::string& errPath, bool doCheckGLSL)
{
	std::string input;
	if (!ReadStringFromFile (inputPath.c_str(), input))
	{
		printf ("  failed to read input file\n");
		return false;
	}

	ShHandle parser = Hlsl2Glsl_ConstructCompiler (vertex ? EShLangVertex : EShLangFragment, 0);

	const char* sourceStr = input.c_str();

	bool res = true;

	if (Hlsl2Glsl_Parse (parser, sourceStr, 0))
	{
		static EAttribSemantic kAttribSemantic[] = {
			EAttrSemTangent,
		};
		static const char* kAttribString[] = {
			"TANGENT",
		};
		Hlsl2Glsl_SetUserAttributeNames (parser, kAttribSemantic, kAttribString, 1);
		if (Hlsl2Glsl_Translate (parser, "main"))
		{
			std::string text = Hlsl2Glsl_GetShader (parser);

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
			if (doCheckGLSL && !CheckGLSL (vertex, text.c_str()))
			{
				res = false;
			}
		}
		else
		{
			const char* infoLog = Hlsl2Glsl_GetInfoLog( parser );
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

	Hlsl2Glsl_DestructCompiler (parser);

	return res;
}


int main (int argc, const char** argv)
{
	if (argc < 2)
	{
		printf ("USAGE: hlsl2glsltest testfolder\n");
		return 1;
	}

	bool hasOpenGL = InitializeOpenGL ();
	Hlsl2Glsl_Initialize ();

	std::string baseFolder = argv[1];

	static const char* kTypeName[2] = { "vertex", "fragment" };
	size_t tests = 0;
	size_t errors = 0;
	for (int type = 0; type < 2; ++type)
	{
		printf ("testing %s...\n", kTypeName[type]);
		std::string testFolder = baseFolder + "/" + kTypeName[type];
		StringVector inputFiles = GetFiles (testFolder, "-in.txt");

		size_t n = inputFiles.size();
		tests += n;
		for (size_t i = 0; i < n; ++i)
		{
			std::string inname = inputFiles[i];
			printf ("test %s\n", inname.c_str());
			std::string outname = inname.substr (0,inname.size()-7) + "-out.txt";
			std::string errname = inname.substr (0,inname.size()-7) + "-res.txt";
			bool ok = TestFile (type==0, testFolder + "/" + inname, testFolder + "/" + outname, testFolder + "/" + errname, hasOpenGL);
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
