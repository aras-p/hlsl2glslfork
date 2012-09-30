#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <string>
#include <vector>
#include <time.h>


const bool kDumpShaderAST = false;


#ifdef _MSC_VER
#include <windows.h>
#include <gl/GL.h>
#include <cstdarg>

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

// forks stdio to debug console so when you hit a breakpoint you can more easily see what test is running:
static void logf(const char* format, ...)
{
    va_list args = NULL;
    va_start(args, format);
    vprintf(format, args);
    va_end(args); //lint !e1924: C-style cast
    
    char buffer[4096];
    const size_t bufferSize = sizeof(buffer) - 1;

    args = NULL;
    va_start(args, format);
    int rc = _vsnprintf(buffer, bufferSize, format, args);
    va_end(args); //lint !e1924: C-style cast
    
    size_t outputLen = (rc <= 0) ? 0 : static_cast<size_t>(rc);
    buffer[outputLen] = '\0';

    OutputDebugStringA(buffer);
}

#define printf logf

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

static void DeleteFile (const std::string& path)
{
	#ifdef _MSC_VER
	DeleteFileA (path.c_str());
	#else
	unlink (path.c_str());
	#endif
}

static bool ReadStringFromFile (const char* pathName, std::string& output)
{
	FILE* file = fopen( pathName, "rb" );
	if (file == NULL)
		return false;
	
	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (length < 0)
	{
		fclose( file );
		return false;
	}
	
	output.resize(length);
	size_t readLength = fread(&*output.begin(), 1, length, file);
	
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


static void replace_string (std::string& target, const std::string& search, const std::string& replace, size_t startPos)
{
	if (search.empty())
		return;
	
	std::string::size_type p = startPos;
	while ((p = target.find (search, p)) != std::string::npos)
	{
		target.replace (p, search.size (), replace);
		p += replace.size ();
	}
}


static bool CheckGLSL (bool vertex, ETargetVersion version, const std::string& source)
{
	const char* sourcePtr = source.c_str();
	std::string newSrc;
	if (version == ETargetGLSL_ES_100)
	{
		newSrc.reserve(source.size());
		newSrc += "#define lowp\n";
		newSrc += "#define mediump\n";
		newSrc += "#define highp\n";
		newSrc += "#define texture2DLodEXT texture2DLod\n";
		newSrc += "#define texture2DProjLodEXT texture2DProjLod\n";
		newSrc += "#define texture2DGradEXT texture2DGradARB\n";
		newSrc += "float shadow2DEXT (sampler2DShadow s, vec3 p) { return shadow2D(s,p).r; }\n";
		newSrc += "float shadow2DProjEXT (sampler2DShadow s, vec4 p) { return shadow2DProj(s,p).r; }\n";
		newSrc += source;
		replace_string (newSrc, "GL_EXT_shader_texture_lod", "GL_ARB_shader_texture_lod", 0);
		replace_string (newSrc, "#extension GL_OES_standard_derivatives : require", "", 0);
		replace_string (newSrc, "#extension GL_EXT_shadow_samplers : require", "", 0);
					   
		sourcePtr = newSrc.c_str();
	}
	
	GLhandleARB shader = glCreateShaderObjectARB (vertex ? GL_VERTEX_SHADER_ARB : GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB (shader, 1, &sourcePtr, NULL);
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

enum TestRun {
	VERTEX,
	FRAGMENT,
	BOTH,
	VERTEX_120,
	FRAGMENT_120,
	VERTEX_FAILURES,
	FRAGMENT_FAILURES,
	NUM_RUN_TYPES
};

const bool kIsVertexShader[NUM_RUN_TYPES] = {
	true,
	false,
	false,
	true,
	false,
	true,
	false,
};

static const char* kTypeName[NUM_RUN_TYPES] = {
	"vertex",
	"fragment",
	"combined",
	"vertex-120",
	"fragment-120",
	"vertex-failures",
	"fragment-failures",
};

static const EShLanguage kTypeLangs[NUM_RUN_TYPES] = {
	EShLangVertex,
	EShLangFragment,
	EShLangCount,
	EShLangVertex,
	EShLangFragment,
	EShLangVertex,
	EShLangFragment,
};

static const ETargetVersion kTargets1[NUM_RUN_TYPES] = {
	ETargetGLSL_110,
	ETargetGLSL_110,
	ETargetGLSL_120,
	ETargetGLSL_120,
	ETargetGLSL_120,
	ETargetGLSL_110,
	ETargetGLSL_110,
};
static const ETargetVersion kTargets2[NUM_RUN_TYPES] = {
	ETargetGLSL_ES_100,
	ETargetGLSL_ES_100,
	ETargetVersionCount,
	ETargetGLSL_120,
	ETargetGLSL_120,
	ETargetVersionCount,
	ETargetVersionCount,
};


static bool TestFile (TestRun type,
					  const std::string& inputPath,
					  const std::string& outputPath,
					  const char* entryPoint,
					  ETargetVersion version,
					  unsigned options,
					  bool doCheckGLSL)
{
	assert(version != ETargetVersionCount);
	
	std::string input;
	if (!ReadStringFromFile (inputPath.c_str(), input))
	{
		printf ("  failed to read input file\n");
		return false;
	}
	
	ShHandle parser = Hlsl2Glsl_ConstructCompiler (kTypeLangs[type]);

	const char* sourceStr = input.c_str();

	bool res = true;

	if (kDumpShaderAST)
		options |= ETranslateOpIntermediate;
		
	int parseOk = Hlsl2Glsl_Parse (parser, sourceStr, version, options);
	const char* infoLog = Hlsl2Glsl_GetInfoLog( parser );
	if (kDumpShaderAST)
	{
		// write output
		FILE* f = fopen ((outputPath+"-ir.txt").c_str(), "wb");
		fwrite (infoLog, 1, strlen(infoLog), f);
		fclose (f);
	}
	if (parseOk)
	{
		static EAttribSemantic kAttribSemantic[] = {
			EAttrSemTangent,
		};
		static const char* kAttribString[] = {
			"TANGENT",
		};
		Hlsl2Glsl_SetUserAttributeNames (parser, kAttribSemantic, kAttribString, 1);
		Hlsl2Glsl_UseUserVaryings (parser, true);
		
		int translateOk = Hlsl2Glsl_Translate (parser, entryPoint, version, options);
		const char* infoLog = Hlsl2Glsl_GetInfoLog( parser );
		if (translateOk)
		{
			std::string text = Hlsl2Glsl_GetShader (parser);
			
			for (size_t i = 0, n = text.size(); i != n; ++i)
			{
			   char c = text[i];
			   
			   if (!isascii(c))
			   {
				   printf ("  contains non-ascii '%c' (0x%02X)\n", c, c);
				   res = false;
			   }
			}

			std::string output;
			ReadStringFromFile (outputPath.c_str(), output);

			if (text != output)
			{
				// write output
				FILE* f = fopen (outputPath.c_str(), "wb");
				fwrite (text.c_str(), 1, text.size(), f);
				fclose (f);
				printf ("  does not match expected output\n");
				res = false;
			}
			if (doCheckGLSL && !CheckGLSL (kIsVertexShader[type], version, text))
			{
				res = false;
			}
		}
		else
		{
			printf ("  translate error: %s\n", infoLog);
			res = false;
		}
	}
	else
	{
		printf ("  parse error: %s\n", infoLog);
		res = false;
	}

	Hlsl2Glsl_DestructCompiler (parser);

	return res;
}


static bool TestFileFailure (TestRun type,
	const std::string& inputPath,
	const std::string& outputPath)
{
	std::string input;
	if (!ReadStringFromFile (inputPath.c_str(), input))
	{
		printf ("  failed to read input file\n");
		return false;
	}

	ShHandle parser = Hlsl2Glsl_ConstructCompiler (kTypeLangs[type]);
	const ETargetVersion version = kTargets1[type];

	const char* sourceStr = input.c_str();

	bool res = true;

	unsigned options = 0;
	if (kDumpShaderAST)
		options |= ETranslateOpIntermediate;
	int parseOk = Hlsl2Glsl_Parse (parser, sourceStr, version, options);
	
	if (parseOk)
	{
		int translateOk = Hlsl2Glsl_Translate (parser, "main", version, options);
		
		if (translateOk) 
		{
			printf ("  translation was expected to fail\n");
		    res = false;
		}
    }
    
	std::string text = Hlsl2Glsl_GetInfoLog( parser );
	std::string output;
	
	if (res)
	{
	    ReadStringFromFile (outputPath.c_str(), output);
    }
    
	if (!res || (text != output))
	{
		// write output
		FILE* f = fopen (outputPath.c_str(), "wb");
		fwrite (text.c_str(), 1, text.size(), f);
		fclose (f);
		printf ("  does not match expected output\n");
		res = false;
	}

	Hlsl2Glsl_DestructCompiler (parser);

	return res;
}

static bool TestCombinedFile(const std::string& inputPath, ETargetVersion version, bool checkGL)
{
	std::string outname = inputPath.substr (0,inputPath.size()-7);
	std::string frag_out, vert_out;
	
	if (version == ETargetGLSL_ES_100) {
		vert_out = outname + "-vertex-outES.txt";
		frag_out = outname + "-fragment-outES.txt";
	} else {
		vert_out = outname + "-vertex-out.txt";
		frag_out = outname + "-fragment-out.txt";
	}
	
	bool res = TestFile(VERTEX, inputPath, vert_out, "vs_main", version, 0, checkGL);
	return res & TestFile(FRAGMENT, inputPath, frag_out, "ps_main", version, 0, checkGL);
}


static bool TestFile (TestRun type,
					  const std::string& inputPath,
					  ETargetVersion version,
					  unsigned options,
					  bool checkGL)
{
	std::string outname = inputPath.substr (0,inputPath.size()-7);

	const char* suffix = "-out.txt";
	if (version == ETargetGLSL_ES_100)
		suffix = "-outES.txt";
	else if (options & ETranslateOpEmitGLSL120ArrayInitWorkaround)
		suffix = "-out120arr.txt";
	
	if (type == VERTEX_FAILURES || type == FRAGMENT_FAILURES) {
		return TestFileFailure(type, inputPath, outname + suffix);
	} else {
		return TestFile(type, inputPath, outname + suffix, "main", version, options, checkGL);
	}
	return false;
}

void Delete(void* p, void* ud);
void* Allocate(unsigned size, void* ud);

void* Allocate(unsigned size, void* ud)
{
	return malloc(size);
}

void Delete(void* p, void* ud)
{
	free(p);
}



int main (int argc, const char** argv)
{
	if (argc < 2)
	{
		printf ("USAGE: hlsl2glsltest testfolder\n");
		return 1;
	}

	bool hasOpenGL = InitializeOpenGL ();
	
	clock_t time0 = clock();
	
	Hlsl2Glsl_Initialize (Allocate, Delete, NULL);

	std::string baseFolder = argv[1];

	size_t tests = 0;
	size_t errors = 0;
	for (int type = 0; type < NUM_RUN_TYPES; ++type)
	{
		printf ("TESTING %s...\n", kTypeName[type]);
		const ETargetVersion version1 = kTargets1[type];
		const ETargetVersion version2 = kTargets2[type];
		std::string testFolder = baseFolder + "/" + kTypeName[type];
		StringVector inputFiles = GetFiles (testFolder, "-in.txt");

		size_t n = inputFiles.size();
		tests += n;
		for (size_t i = 0; i < n; ++i)
		{
			std::string inname = inputFiles[i];
			//if (inname != "non-matching-type-init-in.txt")
			//	continue;
			bool ok = true;
			
			printf ("test %s\n", inname.c_str());
			if (type == BOTH) {
				ok = TestCombinedFile(testFolder + "/" + inname, version1, hasOpenGL);
				if (ok && version2 != ETargetVersionCount)
					ok = TestCombinedFile(testFolder + "/" + inname, version2, hasOpenGL);
			} else {
				ok = TestFile(TestRun(type), testFolder + "/" + inname, version1, 0, hasOpenGL);
				if (ok && version2 != ETargetVersionCount)
					ok = TestFile(TestRun(type), testFolder + "/" + inname, version2, ETranslateOpEmitGLSL120ArrayInitWorkaround, hasOpenGL);
			}
			
			if (!ok)
				++errors;
		}
	}

	clock_t time1 = clock();
	float t = float(time1-time0) / float(CLOCKS_PER_SEC);
	if (errors != 0)
		printf ("%i tests, %i FAILED, %.2fs\n", tests, errors, t);
	else
		printf ("%i tests succeeded, %.2fs\n", tests, t);

	return errors ? 1 : 0;
}
