// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "hlslLinker.h"

#include "glslFunction.h"
#include "hlslCrossCompiler.h"

#include "hlslSupportLib.h"
#include "osinclude.h"
#include <algorithm>
#include <string.h>
#include <set>

static const char* kTargetVersionStrings[ETargetVersionCount] = {
	"", // ES 1.00
	"", // 1.10
	"#version 120\n", // 1.20
};


// String table that maps attribute semantics to built-in GLSL attributes
static const char* attribString[EAttrSemCount] = {
	"",
	"gl_Vertex",
	"",
	"",
	"",
	"",
	"",
	"gl_Normal",
	"",
	"",
	"",
	"gl_Color",
	"gl_SecondaryColor",
	"",
	"",
	"gl_MultiTexCoord0",
	"gl_MultiTexCoord1",
	"gl_MultiTexCoord2",
	"gl_MultiTexCoord3",
	"gl_MultiTexCoord4",
	"gl_MultiTexCoord5",
	"gl_MultiTexCoord6",
	"gl_MultiTexCoord7",
	"",
	"",
	"xlat_attrib_tangent",
	"",
	"",
	"",
	"xlat_attrib_binorm",
	"",
	"",
	"",
	"xlat_attrib_blendweights",
	"",
	"",
	"",
	"xlat_attrib_blendindices",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"gl_VertexID",
	"gl_InstanceIDARB",
	"",
	""
};

// String table that maps attribute semantics to built-in GLSL output varyings
static const char* varOutString[EAttrSemCount] = {
	"",
	"gl_Position",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"gl_FrontColor",
	"gl_FrontSecondaryColor",
	"",
	"",
	"gl_TexCoord[0]",
	"gl_TexCoord[1]",
	"gl_TexCoord[2]",
	"gl_TexCoord[3]",
	"gl_TexCoord[4]",
	"gl_TexCoord[5]",
	"gl_TexCoord[6]",
	"gl_TexCoord[7]",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"gl_PointSize",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
};

// String table that maps attribute semantics to built-in GLSL input varyings
static const char* varInString[EAttrSemCount] = {
	"",
	"",
	"",
	"",
	"",
	"gl_FragCoord",
	"gl_FrontFacing",
	"",
	"",
	"",
	"",
	"gl_Color",
	"gl_SecondaryColor",
	"",
	"",
	"gl_TexCoord[0]",
	"gl_TexCoord[1]",
	"gl_TexCoord[2]",
	"gl_TexCoord[3]",
	"gl_TexCoord[4]",
	"gl_TexCoord[5]",
	"gl_TexCoord[6]",
	"gl_TexCoord[7]",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"gl_PrimitiveID",
	"",
};

// String table that maps attribute semantics to built-in GLSL fragment shader outputs
static const char* resultString[EAttrSemCount] = {
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"gl_FragData[0]",
	"gl_FragData[1]",
	"gl_FragData[2]",
	"gl_FragData[3]",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"gl_FragDepth",
	"gl_SampleMask[0]",
	"",
	"",
	"",
	"",
};

static const char* kUserVaryingPrefix = "xlv_";

static inline void AddToVaryings (std::stringstream& s, TPrecision prec, const std::string& type, const std::string& name)
{
	if (strstr (name.c_str(), kUserVaryingPrefix) == name.c_str())
		s << "varying " << getGLSLPrecisiontring(prec) << type << " " << name << ";\n";
}

HlslLinker::HlslLinker(TInfoSink& infoSink_) : infoSink(infoSink_)
{
	for ( int i = 0; i < EAttrSemCount; i++)
	{
		userAttribString[i][0] = 0;
	}

	// By default, use GL build-in varyings
	bUserVaryings = false;
}


HlslLinker::~HlslLinker()
{
	for ( std::vector<ShUniformInfo>::iterator it = uniforms.begin(); it != uniforms.end(); it++)
	{
		delete [] it->name;
		delete [] it->semantic;
		delete [] it->init;
	}
}



bool HlslLinker::getArgumentData2( const std::string &name, const std::string &semantic, EGlslSymbolType type,
								 EClassifier c, std::string &outName, std::string &ctor, int &pad, int semanticOffset)
{
	int size;
	EGlslSymbolType base = EgstVoid;
	EAttribSemantic sem = parseAttributeSemantic( semantic );

	// Offset the semantic for the case of an array
	sem = static_cast<EAttribSemantic>( (int)sem + semanticOffset );

	//clear the return values
	outName = "";
	ctor = "";
	pad = 0;

	//compute the # of elements in the type
	switch (type)
	{
	case EgstBool:
	case EgstBool2:
	case EgstBool3:
	case EgstBool4:
		base = EgstBool;
		size = type - EgstBool + 1;
		break;

	case EgstInt:
	case EgstInt2:
	case EgstInt3:
	case EgstInt4:
		base = EgstInt;
		size = type - EgstInt + 1;
		break;

	case EgstFloat:
	case EgstFloat2:
	case EgstFloat3:
	case EgstFloat4:
		base = EgstFloat;
		size = type - EgstFloat + 1;
		break;

    default:
        return false;
	};

	if ( c != EClassUniform)
	{

		ctor = getTypeString( (EGlslSymbolType)((int)base + size - 1)); //default constructor
		pad = 0;

		switch (c)
		{
		case EClassNone:
			return false;

		case EClassAttrib:
			// If the user has specified a user attrib name, use a user attribute
			if ( userAttribString[sem][0] != '\0')
			{
				outName = userAttribString[sem];
			}
			// Otherwise, use the built-in attribute name
			else
			{
				outName = attribString[sem];
				if (sem == EAttrSemNormal && size == 4)
					pad = 1;
				else if ( sem == EAttrSemUnknown || outName[0] == '\0' )
				{
					//handle the blind data
					outName = "xlat_attrib_";
					outName += semantic;
				}
			}
			break;

		case EClassVarOut:
			// If using user varyings, create a user varying name
			if ( (bUserVaryings && sem != EAttrSemPosition && sem != EAttrSemPrimitiveID) || varOutString[sem][0] == 0 )
			{
				outName = kUserVaryingPrefix;
				outName += semantic;
				// If an array element, add the semantic offset to the name
				if ( semanticOffset != 0 )
				{
					outName += "_";
					outName += ( semanticOffset + '0' );
				}
			}
			else
			{
				// Use built-in varying name
				outName = varOutString[sem];

				// Always pad built-in varying outputs to 4 elements
				pad = 4 - size;
				ctor = "vec4";
			}
			break;

		case EClassVarIn:
			// If using user varyings, create a user varying name
			if ( (bUserVaryings && sem != EAttrSemVPos && sem != EAttrSemVFace && sem != EAttrSemPrimitiveID) || varInString[sem][0] == 0 )
			{
				outName = kUserVaryingPrefix;
				outName += stripSemanticModifier (semantic, false);
				// If an array element, add the semantic offset to the name
				if ( semanticOffset != 0 )
				{
					outName += "_";
					outName += ( semanticOffset + '0' );
				}
			}
			else
			{
				// Use built-in varying name
				outName = varInString[sem];
			}                
			break;

		case EClassRes:
			outName = resultString[sem];
			if ( sem == EAttrSemDepth)
				ctor = "float";
			else if ( sem == EAttrSemCoverage )
				ctor = "int";
			else
			{
				pad = 4 - size;
				ctor = "vec4";
			}
			break;

		case EClassUniform:
			assert(0); // this should have been stripped
			return false; 
		};


	}
	else
	{
		//these should always match exactly
		outName = "xlu_";
		outName += name;
	}

	return true;
}


bool HlslLinker::getArgumentData( GlslSymbol* sym, EClassifier c, std::string &outName,
								 std::string &ctor, int &pad)
{
	const std::string &name = sym->getName();
	const std::string &semantic = sym->getSemantic();
	EGlslSymbolType type = sym->getType();

	return getArgumentData2( name, semantic, type, c, outName, ctor, pad, 0);
}



bool HlslLinker::setUserAttribName ( EAttribSemantic eSemantic, const char *pName )
{
	if ( eSemantic >= EAttrSemPosition && eSemantic <= EAttrSemDepth )
	{
		if ( strlen ( pName ) > MAX_ATTRIB_NAME )
		{
			assert(0);
			infoSink.info << "Attribute name (" << pName << ") larger than max (" << MAX_ATTRIB_NAME << ")\n";
			return false;
		}
		strcpy ( userAttribString[eSemantic], pName );
		return true;
	}

	infoSink.info << "Semantic value " << eSemantic << " unknown \n";
	return false;
}



// Strip the semantic string of any modifiers
std::string HlslLinker::stripSemanticModifier(const std::string &semantic, bool warn)
{
	std::string newSemantic = semantic;

	size_t centroidLoc = semantic.find ("_centroid");
	if (centroidLoc != std::string::npos)
	{
		if (warn)
			infoSink.info << "Warning: '" << semantic << "' contains centroid modifier.  Modifier ignored because GLSL v1.10 does not support centroid\n";       
		newSemantic = semantic.substr (0, centroidLoc);
	}

	return newSemantic;
}


struct AttrSemanticMapping {
	const char* name;
	EAttribSemantic sem;
};

static AttrSemanticMapping kAttributeSemantic[] = {	
	{ "position", EAttrSemPosition },
	{ "position0", EAttrSemPosition },
	{ "position1", EAttrSemPosition1 },
	{ "position2", EAttrSemPosition2 },
	{ "position3", EAttrSemPosition3 },
	{ "vpos", EAttrSemVPos },
	{ "vface", EAttrSemVFace },
	{ "normal", EAttrSemNormal },
	{ "normal0", EAttrSemNormal },
	{ "normal1", EAttrSemNormal1 },
	{ "normal2", EAttrSemNormal2 },
	{ "normal3", EAttrSemNormal3 },
	{ "tangent", EAttrSemTangent },
	{ "tangent0", EAttrSemTangent },
	{ "tangent1", EAttrSemTangent1 },
	{ "tangent2", EAttrSemTangent2 },
	{ "tangent3", EAttrSemTangent3 },
	{ "binormal", EAttrSemBinormal },
	{ "binormal0", EAttrSemBinormal },
	{ "binormal1", EAttrSemBinormal1 },
	{ "binormal2", EAttrSemBinormal2 },
	{ "binormal3", EAttrSemBinormal3 },
	{ "blendweight", EAttrSemBlendWeight },
	{ "blendweight0", EAttrSemBlendWeight },
	{ "blendweight1", EAttrSemBlendWeight1 },
	{ "blendweight2", EAttrSemBlendWeight2 },
	{ "blendweight3", EAttrSemBlendWeight3 },
	{ "blendindices", EAttrSemBlendIndices },
	{ "blendindices0", EAttrSemBlendIndices },
	{ "blendindices1", EAttrSemBlendIndices1 },
	{ "blendindices2", EAttrSemBlendIndices2 },
	{ "blendindices3", EAttrSemBlendIndices3 },
	{ "psize", EAttrSemPSize },
	{ "psize0", EAttrSemPSize },
	{ "psize1", EAttrSemPSize1 },
	{ "psize2", EAttrSemPSize2 },
	{ "psize3", EAttrSemPSize3 },
	{ "color", EAttrSemColor0 },
	{ "color0", EAttrSemColor0 },
	{ "color1", EAttrSemColor1 },
	{ "color2", EAttrSemColor2 },
	{ "color3", EAttrSemColor3 },
	{ "texcoord", EAttrSemTex0 },
	{ "texcoord0", EAttrSemTex0 },
	{ "texcoord1", EAttrSemTex1 },
	{ "texcoord2", EAttrSemTex2 },
	{ "texcoord3", EAttrSemTex3 },
	{ "texcoord4", EAttrSemTex4 },
	{ "texcoord5", EAttrSemTex5 },
	{ "texcoord6", EAttrSemTex6 },
	{ "texcoord7", EAttrSemTex7 },
	{ "texcoord8", EAttrSemTex8 },
	{ "texcoord9", EAttrSemTex9 },
	{ "depth", EAttrSemDepth },
	{ "sv_vertexid", EAttrSemVertexID },
	{ "sv_primitiveid", EAttrSemPrimitiveID },
	{ "sv_instanceid", EAttrSemInstanceID },
	{ "sv_coverage", EAttrSemCoverage }
};

// Determine the GLSL attribute semantic for a given HLSL semantic
EAttribSemantic HlslLinker::parseAttributeSemantic (const std::string &semantic)
{
	std::string curSemantic = stripSemanticModifier (semantic, true);
	for (size_t i = 0; i < sizeof(kAttributeSemantic)/sizeof(kAttributeSemantic[0]); ++i)
		if (!_stricmp(curSemantic.c_str(), kAttributeSemantic[i].name))
			return kAttributeSemantic[i].sem;
	return EAttrSemUnknown;
}



/// Add the functions called by a function to the function set
/// \param func
///   The function for which all called functions will be added
/// \param funcSet
///   The set of currently called functions
/// \param funcList
///   The list of all functions
/// \return
///   True if all functions are found in the funcList, false otherwise.
bool HlslLinker::addCalledFunctions( GlslFunction *func, FunctionSet& funcSet, std::vector<GlslFunction*> &funcList )
{
	const std::set<std::string> &cf = func->getCalledFunctions();

	for (std::set<std::string>::const_iterator cit=cf.begin(); cit != cf.end(); cit++)
	{
		std::vector<GlslFunction*>::iterator it = funcList.begin();

		//This might be better as a more efficient search
		while (it != funcList.end())
		{
			if ( *cit == (*it)->getMangledName())
				break;
			it++;
		}

		//check to see if it really exists
		if ( it == funcList.end())
		{
			infoSink.info << "Failed to find function '" << *cit <<"'\n";
			return false;
		}

		//add the function (if it's not there already) and recurse
		if (std::find (funcSet.begin(), funcSet.end(), *it) == funcSet.end())
			funcSet.push_back (*it);
		addCalledFunctions( *it, funcSet, funcList); 
	}

	return true;
}

typedef std::vector<GlslFunction*> FunctionSet;

static void EmitCalledFunctions (std::stringstream& shader, const FunctionSet& functions)
{
	if (functions.empty())
		return;

	for (FunctionSet::const_reverse_iterator fit = functions.rbegin(); fit != functions.rend(); fit++) // emit backwards, will put least used ones in front
	{
		shader << (*fit)->getPrototype() << ";\n";
	}

	for (FunctionSet::const_reverse_iterator fit = functions.rbegin(); fit != functions.rend(); fit++) // emit backwards, will put least used ones in front
	{
		shader << "\n";
		OutputLineDirective(shader, (*fit)->getLine());
		shader << (*fit)->getPrototype() << " {\n";
		shader << (*fit)->getCode() << "\n"; //has embedded }
		shader << "\n";
	}
}

static void EmitIfNotEmpty (std::stringstream& out, const std::stringstream& str)
{
	if (str.str().size())
		out << str.str() << "\n";
}

static const char* GetEntryName (const char* entryFunc)
{
	if (!entryFunc)
		return "";
	if (!strcmp(entryFunc, "main"))
		return "xlat_main";
	return entryFunc;
}

static const char* kShaderTypeNames[2] = { "Vertex", "Fragment" };

void HlslLinker::addRequiredExtensions(EAttribSemantic sem, ExtensionSet &extensions)
{
	if (sem == EAttrSemPrimitiveID || sem == EAttrSemVertexID)
		extensions.insert("GL_EXT_gpu_shader4");
	
	if (sem == EAttrSemInstanceID)
		extensions.insert("GL_ARB_draw_instanced");
}


bool HlslLinker::linkerSanityCheck(HlslCrossCompiler* compiler, const char* entryFunc)
{
	if (!compiler)
	{
		infoSink.info << "No shader compiler provided\n";
		return false;
	}
	if (!entryFunc)
	{
		infoSink.info << "No shader entry function provided\n";
		return false;
	}
	return true;
}


bool HlslLinker::buildFunctionLists(HlslCrossCompiler* comp, EShLanguage lang, const std::string& entryPoint, GlslFunction*& globalFunction, std::vector<GlslFunction*>& functionList, FunctionSet& calledFunctions, GlslFunction*& funcMain)
{
	// build the list of functions
	std::vector<GlslFunction*> &fl = comp->functionList;
	
	for (std::vector<GlslFunction*>::iterator fit = fl.begin(); fit < fl.end(); ++fit)
	{
		if ((*fit)->isGlobalScopeFunction())
		{
			assert(!globalFunction);
			globalFunction = *fit;
		}
		else
			functionList.push_back(*fit);
		
		if ((*fit)->getName() == entryPoint)
		{
			if (funcMain)
			{
				infoSink.info << kShaderTypeNames[lang] << " entry function cannot be overloaded\n";
				return false;
			}
			funcMain = *fit;
		}
	}
	
	// check to ensure that we found the entry function
	if (!funcMain)
	{
		infoSink.info << "Failed to find entry function: '" << entryPoint <<"'\n";
		return false;
	}
	
	//add all the called functions to the list
	calledFunctions.push_back (funcMain);
	if (!addCalledFunctions (funcMain, calledFunctions, functionList))
		infoSink.info << "Failed to resolve all called functions in the " << kShaderTypeNames[lang] << " shader\n";
	
	return true;
}


void HlslLinker::buildUniformsAndLibFunctions(const FunctionSet& calledFunctions, std::vector<GlslSymbol*>& constants, std::set<TOperator>& libFunctions)
{
	for (FunctionSet::const_iterator it = calledFunctions.begin(); it != calledFunctions.end(); ++it) {
		const std::vector<GlslSymbol*> &symbols = (*it)->getSymbols();
		
		unsigned n_symbols = symbols.size();
		for (unsigned i = 0; i != n_symbols; ++i) {
			GlslSymbol* s = symbols[i];
			if (s->getQualifier() == EqtUniform || s->getQualifier() == EqtMutableUniform)
				constants.push_back(s);
		}
		
		//take each referenced library function, and add it to the set
		const std::set<TOperator> &referencedFunctions = (*it)->getLibFunctions();
		libFunctions.insert( referencedFunctions.begin(), referencedFunctions.end());
	}
	
	// Remove duplicates
	constants.resize(std::unique(constants.begin(), constants.end()) - constants.begin());
}


void HlslLinker::emitLibraryFunctions(const std::set<TOperator>& libFunctions, EShLanguage lang, bool usePrecision)
{
	// library Functions & required extensions
	std::string shaderExtensions, shaderLibFunctions;
	if (!libFunctions.empty())
	{
		for (std::set<TOperator>::const_iterator it = libFunctions.begin(); it != libFunctions.end(); it++)
		{
			const std::string &func = getHLSLSupportCode(*it, shaderExtensions, lang==EShLangVertex, usePrecision);
			if (!func.empty())
			{
				shaderLibFunctions += func;
				shaderLibFunctions += '\n';
			}
		}
	}
	shader << shaderExtensions;
	shader << shaderLibFunctions;
}


void HlslLinker::emitStructs(HlslCrossCompiler* comp)
{
	// Presently, structures are not tracked per function, just dump them all
	// This could be improved by building a complete list of structures for the
	// shaders based on the variables in each function.
	
	std::vector<GlslStruct*> &sList = comp->structList;
	if (!sList.empty())
	{
		for (std::vector<GlslStruct*>::iterator it = sList.begin(); it < sList.end(); it++)
		{
			shader << "\n";
			OutputLineDirective(shader, (*it)->getLine());
			shader << (*it)->getDecl() << "\n";
		}
	}
}


void HlslLinker::emitGlobals(const GlslFunction* globalFunction, const std::vector<GlslSymbol*>& constants)
{
	// write global scope declarations (represented as a fake function)
	assert(globalFunction);
	shader << globalFunction->getCode();
	
	// write mutable uniform declarations
	const unsigned n_constants = constants.size();
	for (unsigned i = 0; i != n_constants; ++i) {
		GlslSymbol* s = constants[i];
		if (s->getIsMutable()) {
			s->writeDecl(shader, GlslSymbol::WRITE_DECL_MUTABLE_UNIFORMS);
			shader << ";\n";	
		}
	}	
}


void HlslLinker::buildUniformReflection(const std::vector<GlslSymbol*>& constants)
{
	const unsigned n_constants = constants.size();
	for (unsigned i = 0; i != n_constants; ++i) {
		GlslSymbol* s = constants[i];
		
		ShUniformInfo info;
		const std::string& name = s->getName(false);
		info.name = new char[name.size()+1];
		strcpy(info.name, name.c_str());
		
		if (s->getSemantic() != "") {
			info.semantic = new char[s->getSemantic().size()+1];
			strcpy(info.semantic, s->getSemantic().c_str());
		}
		else
			info.semantic = 0;
		
		info.type = (EShType)s->getType();
		info.arraySize = s->getArraySize();
		info.init = 0;
		uniforms.push_back(info);
	}
}


static void emitSymbolWithPad (std::stringstream& str, const std::string& ctor, const std::string& name, int pad)
{
	str << ctor << "(" << name;
	for (int i = 0; i < pad; ++i)
		str << ", 0.0";
	str << ")";
}


static void emitSingleInputVariable (EShLanguage lang, const std::string& name, const std::string& ctor, EGlslSymbolType type, TPrecision prec, std::stringstream& attrib, std::stringstream& varying)
{
	// vertex shader: emit custom attributes
	if (lang == EShLangVertex && strncmp(name.c_str(), "gl_", 3) != 0)
	{
		int typeOffset = 0;
		
		// If the type is integer or bool based, we must convert to a float based
		// type. This is because GLSL does not allow int or bool based vertex attributes.
		//
		// NOTE: will need to be updated for more modern GLSL versions to allow this!
		if (type >= EgstInt && type <= EgstInt4)
			typeOffset += 4;
		if (type >= EgstBool && type <= EgstBool4)
			typeOffset += 8;
		
		attrib << "attribute " << getTypeString((EGlslSymbolType)(type + typeOffset)) << " " << name << ";\n";
	}
	
	// fragment shader: emit varying
	if (lang == EShLangFragment)
	{
		AddToVaryings (varying, prec, ctor, name);
	}
}
	

void HlslLinker::emitInputNonStructParam(GlslSymbol* sym, EShLanguage lang, bool usePrecision, EAttribSemantic attrSem, std::stringstream& attrib, std::stringstream& varying, std::stringstream& preamble, std::stringstream& call)
{
	std::string name, ctor;
	int pad;
	
	if (!getArgumentData (sym, lang==EShLangVertex ? EClassAttrib : EClassVarIn, name, ctor, pad))
	{
		// should deal with fall through cases here
		assert(0);
		infoSink.info << "Unsupported type for shader entry parameter (";
		infoSink.info << getTypeString(sym->getType()) << ")\n";
		return;
	}
	
	
	// In fragment shader, pass zero for POSITION inputs
	if (lang == EShLangFragment && attrSem == EAttrSemPosition)
	{
		call << ctor << "(0.0)";
		return; // noting more to do
	}
	// For "in" parameters, just call directly to the main
	else if ( sym->getQualifier() != EqtInOut )
	{
		emitSymbolWithPad (call, ctor, name, pad);
	}
	// For "inout" parameters, declare a temp and initialize it
	else
	{
		preamble << "    ";
		writeType (preamble, sym->getType(), NULL, usePrecision?sym->getPrecision():EbpUndefined);
		preamble << " xlt_" << sym->getName() << " = ";
		emitSymbolWithPad (preamble, ctor, name, pad);
		preamble << ";\n";
	}
	
	emitSingleInputVariable (lang, name, ctor, sym->getType(), sym->getPrecision(), attrib, varying);
}


void HlslLinker::emitInputStructParam(GlslSymbol* sym, EShLanguage lang, ExtensionSet& extensions, std::stringstream& attrib, std::stringstream& varying, std::stringstream& preamble, std::stringstream& call)
{
	GlslStruct* str = sym->getStruct();
	assert(str);

	// temporary variable for the struct
	const std::string tempVar = "xlt_" + sym->getName();
	preamble << "    " << str->getName() << " ";
	preamble << tempVar <<";\n";
	call << tempVar;
	
	// process struct members
	const int elem = str->memberCount();
	for (int jj=0; jj<elem; jj++)
	{
		const GlslStruct::StructMember &current = str->getMember(jj);
		EAttribSemantic memberSem = parseAttributeSemantic (current.semantic);
		
		addRequiredExtensions(memberSem, extensions);
		
		// if member of the struct is an array, we have to loop over all array elements
		int arraySize = 1;
		bool isArray = false;
		if (current.arraySize > 0)
		{
			arraySize = current.arraySize;
			isArray = true;
		}
		
		std::string name, ctor;
		for (int idx = 0; idx < arraySize; ++idx)
		{
			int pad;
			if (!getArgumentData2 (current.name, current.semantic, current.type,
								  lang==EShLangVertex ? EClassAttrib : EClassVarIn, name, ctor, pad, idx))
			{
				//should deal with fall through cases here
				assert(0);
				infoSink.info << "Unsupported type for struct element in shader entry parameter (";
				infoSink.info << getTypeString(current.type) << ")\n";
				continue;
			}
			
			preamble << "    ";
			preamble << tempVar << "." << current.name;
			
			if (isArray)
				preamble << "[" << idx << "]";
			
			// In fragment shader, pass zero for POSITION inputs
			if (lang == EShLangFragment && memberSem == EAttrSemPosition)
			{
				preamble << " = " << ctor << "(0.0);\n";
				continue; // nothing more to do
			}
			else
			{
				preamble << " = ";
				emitSymbolWithPad (preamble, ctor, name, pad);
				preamble << ";\n";
			}

			
			emitSingleInputVariable (lang, name, ctor, current.type, current.precision, attrib, varying);
		}
	}
}


void HlslLinker::emitOutputNonStructParam(GlslSymbol* sym, EShLanguage lang, bool usePrecision, EAttribSemantic attrSem, std::stringstream& varying, std::stringstream& preamble, std::stringstream& postamble, std::stringstream& call)
{
	std::string name, ctor;
	int pad;
	
	if (!getArgumentData( sym, lang==EShLangVertex ? EClassVarOut : EClassRes, name, ctor, pad))
	{
		//should deal with fall through cases here
		assert(0);
		infoSink.info << "Unsupported type for shader entry parameter (";
		infoSink.info << getTypeString(sym->getType()) << ")\n";
		return;
	}
	
	// For "inout" parameters, the preamble was already written so no need to do it here.
	if (sym->getQualifier() != EqtInOut)
	{
		preamble << "    ";
		writeType (preamble, sym->getType(), NULL, usePrecision?sym->getPrecision():EbpUndefined);
		preamble << " xlt_" << sym->getName() << ";\n";                     
	}
	
	// In vertex shader, add to varyings
	if (lang == EShLangVertex)
		AddToVaryings (varying, sym->getPrecision(), ctor, name);
	
	call << "xlt_" << sym->getName();
	
	postamble << "    ";
	postamble << name << " = ";
	emitSymbolWithPad (postamble, ctor, "xlt_"+sym->getName(), pad);
	postamble << ";\n";
}


void HlslLinker::emitOutputStructParam(GlslSymbol* sym, EShLanguage lang, bool usePrecision, EAttribSemantic attrSem, std::stringstream& varying, std::stringstream& preamble, std::stringstream& postamble, std::stringstream& call)
{
	//structs must pass the struct, then process per element
	GlslStruct *Struct = sym->getStruct();
	assert(Struct);
	
	//first create the temp
	std::string tempVar = "xlt_" + sym->getName();
	
	// For "inout" parmaeters the preamble and call were already written, no need to do it here
	if ( sym->getQualifier() != EqtInOut )
	{
		preamble << "    " << Struct->getName() << " ";
		preamble << tempVar <<";\n";
		call << tempVar;
	}
	
	const int elem = Struct->memberCount();
	for (int ii=0; ii<elem; ii++)
	{
		const GlslStruct::StructMember &current = Struct->getMember(ii);
		std::string name, ctor;
		int pad;
		
		if (!getArgumentData2( current.name, current.semantic, current.type, lang==EShLangVertex ? EClassVarOut : EClassRes, name, ctor, pad, 0))
		{
			//should deal with fall through cases here
			assert(0);
			infoSink.info << "Unsupported type in struct element for shader entry parameter (";
			infoSink.info << getTypeString(current.type) << ")\n";
			continue;
		}
		postamble << "    ";
		postamble << name << " = ";
		emitSymbolWithPad (postamble, ctor, tempVar+"."+current.name, pad);		
		postamble << ";\n";

		// In vertex shader, add to varyings
		if (lang == EShLangVertex)
			AddToVaryings (varying, current.precision, ctor, name);
	}
}


void HlslLinker::emitMainStart(const HlslCrossCompiler* compiler, const EGlslSymbolType retType, GlslFunction* funcMain, ETargetVersion version, unsigned options, bool usePrecision, std::stringstream& preamble)
{
	preamble << "void main() {\n";
	
	std::string arrayInit = compiler->m_DeferredArrayInit.str();
	if (!arrayInit.empty())
	{
		const bool emit_120_arrays = (version >= ETargetGLSL_120);
		const bool emit_old_arrays = !emit_120_arrays || (options & ETranslateOpEmitGLSL120ArrayInitWorkaround);
		const bool emit_both = emit_120_arrays && emit_old_arrays;
		
		if (emit_both)
			preamble << "#if defined(HLSL2GLSL_ENABLE_ARRAY_120_WORKAROUND)" << std::endl;
		preamble << arrayInit;
		if (emit_both)
			preamble << "\n#endif" << std::endl;
	}
	
	if (retType == EgstStruct)
	{
		GlslStruct* retStruct = funcMain->getStruct();
		assert(retStruct);
		preamble << "    " << retStruct->getName() << " xl_retval;\n";
	}
	else if (retType != EgstVoid)
	{
		preamble << "    ";
		writeType (preamble, retType, NULL, usePrecision?funcMain->getPrecision():EbpUndefined);
		preamble << " xl_retval;\n";
	}
}


bool HlslLinker::emitReturnValue(const EGlslSymbolType retType, GlslFunction* funcMain, EShLanguage lang, std::stringstream& varying, std::stringstream& postamble)
{
	// void return type
	if (retType == EgstVoid)
	{
		if (lang == EShLangFragment) // fragment shader
		{
			// If no return type, close off the output
			postamble << ";\n";
		}
		return true;
	}
	
	// non-struct return type
	assert (retType != EgstVoid);
	if (retType != EgstStruct)
	{
		std::string name, ctor;
		int pad;
		
		if (!getArgumentData2("", funcMain->getSemantic(), retType, lang==EShLangVertex ? EClassVarOut : EClassRes,
							  name, ctor, pad, 0))
		{
			assert(0);
			infoSink.info << (lang==EShLangVertex ? "Unsupported type for shader return value (" : "Unsupported return type for shader entry function (");
			infoSink.info << getTypeString(retType) << ")\n";
			return true; //@TODO: real error and return false?
		}
		
		postamble << "    ";
		postamble << name << " = ";
		emitSymbolWithPad (postamble, ctor, "xl_retval", pad);		
		postamble << ";\n";
		
		// In vertex shader, add to varyings
		if (lang == EShLangVertex)
			AddToVaryings (varying, funcMain->getPrecision(), ctor, name);
		
		return true;
	}
	
	// struct return type
	assert (retType == EgstStruct);
	GlslStruct *retStruct = funcMain->getStruct();
	assert (retStruct);
	
	const int elem = retStruct->memberCount();
	for (int ii=0; ii<elem; ii++)
	{
		const GlslStruct::StructMember &current = retStruct->getMember(ii);
		std::string name, ctor;
		int pad;
		int arraySize = 1;
		bool isArray = false;
		
		if (lang == EShLangVertex) // vertex shader
		{
			// If it is an array, loop over each member
			if ( current.arraySize > 0 )
			{
				arraySize = current.arraySize;
				isArray = true;
			}
		}
		
		for (int idx = 0; idx < arraySize; ++idx)
		{
			if (!getArgumentData2( current.name, current.semantic, current.type, lang==EShLangVertex ? EClassVarOut : EClassRes, name, ctor, pad, idx))
			{
				infoSink.info << (lang==EShLangVertex ? "Unsupported element type in struct for shader return value (" : "Unsupported struct element type in return type for shader entry function (");
				infoSink.info << getTypeString(current.type) << ")\n";
				return false;
			}
			postamble << "    ";
			postamble << name;                                                            
			postamble << " = " << ctor;
			postamble << "( xl_retval." << current.name;
			if (isArray)
			{
				postamble << "[" << idx << "]";
			}
			for (int ii = 0; ii<pad; ii++)
				postamble << ", 0.0";
			
			postamble << ");\n";
			
			// In vertex shader, add to varyings
			if (lang == EShLangVertex)
				AddToVaryings (varying, current.precision, ctor, name);
		}
	}
	
	return true;
}


bool HlslLinker::link(HlslCrossCompiler* compiler, const char* entryFunc, ETargetVersion targetVersion, unsigned options)
{
	if (!linkerSanityCheck(compiler, entryFunc))
		return false;
	
	const bool usePrecision = Hlsl2Glsl_VersionUsesPrecision(targetVersion);
	
	EShLanguage lang = compiler->getLanguage();
	std::string entryPoint = GetEntryName (entryFunc);
	
	
	// figure out all relevant functions
	GlslFunction* globalFunction = NULL;
	std::vector<GlslFunction*> functionList;
	FunctionSet calledFunctions;
	GlslFunction* funcMain = NULL;
	if (!buildFunctionLists(compiler, lang, entryPoint, globalFunction, functionList, calledFunctions, funcMain))
		return false;
	assert(globalFunction);
	assert(funcMain);
	
	
	// uniforms and used built-in functions
	std::vector<GlslSymbol*> constants;
	std::set<TOperator> libFunctions;
	buildUniformsAndLibFunctions(calledFunctions, constants, libFunctions);
	buildUniformReflection (constants);


	// print all the components collected above.

	emitLibraryFunctions (libFunctions, lang, usePrecision);
	emitStructs(compiler);
	emitGlobals (globalFunction, constants);
	EmitCalledFunctions (shader, calledFunctions);

	
	// Generate a main function that calls the specified entrypoint.
	// That main function uses semantics on the arguments and return values to
	// connect items appropriately.	
	
	ExtensionSet extensions;
	std::stringstream attrib;
	std::stringstream uniform;
	std::stringstream preamble;
	std::stringstream postamble;
	std::stringstream varying;
	std::stringstream call;

	// Declare return value
	const EGlslSymbolType retType = funcMain->getReturnType();
	emitMainStart(compiler, retType, funcMain, targetVersion, options, usePrecision, preamble);
	

	// Call the entry point
	call << "    ";
	if (retType != EgstVoid)
		call << "xl_retval = ";
	call << funcMain->getName() << "( ";
	

	// Entry point parameters
	const int pCount = funcMain->getParameterCount();
	for (int ii=0; ii<pCount; ii++)
	{
		GlslSymbol *sym = funcMain->getParameter(ii);
		EAttribSemantic attrSem = parseAttributeSemantic( sym->getSemantic());
		
		addRequiredExtensions(attrSem, extensions);

		switch (sym->getQualifier())
		{

		// -------- IN & OUT parameters
		case EqtIn:
		case EqtInOut:
		case EqtConst:
			if (sym->getType() != EgstStruct)
			{
				emitInputNonStructParam(sym, lang, usePrecision, attrSem, attrib, varying, preamble, call);
			}
			else
			{
				emitInputStructParam(sym, lang, extensions, attrib, varying, preamble, call);
			}

			// NOTE: for "inout" parameters need to fallthrough to the next case
			if (sym->getQualifier() != EqtInOut)
			{
				break;
			}


		// -------- OUT parameters; also fall-through for INOUT (see the check above)
		case EqtOut:

			if ( sym->getType() != EgstStruct)
			{
				emitOutputNonStructParam(sym, lang, usePrecision, attrSem, varying, preamble, postamble, call);
			}
			else
			{
				emitOutputStructParam(sym, lang, usePrecision, attrSem, varying, preamble, postamble, call);
			}
			break;

		case EqtUniform:
			uniform << "uniform ";
			writeType (uniform, sym->getType(), NULL, usePrecision?sym->getPrecision():EbpUndefined);
			uniform << " xlu_" << sym->getName();
			if(sym->getArraySize())
				uniform << "[" << sym->getArraySize() << "]";
			uniform << ";\n";
			call << "xlu_" << sym->getName();
			break;

		default:
			assert(0);
		};
		if (ii != pCount -1)
			call << ", ";
	}

	call << ");\n";


	// Entry point return value
	if (!emitReturnValue(retType, funcMain, lang, varying, postamble))
		return false;

	postamble << "}\n\n";
	
	
	// Generate final code of the pieces above.
	{
		shaderPrefix << kTargetVersionStrings[targetVersion];
		std::set<const char*>::iterator it = extensions.begin(), end = extensions.end();
		for (; it != end; ++it)
			shaderPrefix << "#extension " << *it << " : require" << std::endl;
	}

	EmitIfNotEmpty (shader, uniform);
	EmitIfNotEmpty (shader, attrib);
	EmitIfNotEmpty (shader, varying);

	shader << preamble.str() << "\n";
	shader << call.str() << "\n";
	shader << postamble.str() << "\n";

	return true;
}


static std::string CleanupShaderText (const std::string& prefix, const std::string& str)
{
	std::string res = prefix;
	size_t n = str.size();
	res.reserve (n + prefix.size());
	char cc = 0;
	for (size_t i = 0; i < n; ++i)
	{
		char c = str[i];
		// Next line used to have str[i-1] instead of cc, but that produces some bug on OSX Lion
		// with Xcode 4.3 (i686-apple-darwin11-llvm-gcc-4.2 (GCC) 4.2.1) in release config; str[i-1]
		// always returns zero.
		if (c != '\n' || i==0 || cc != '\n')
			res.push_back(c);
		cc = c;
	}
	return res;
}



const char* HlslLinker::getShaderText() const 
{
	bs = CleanupShaderText (shaderPrefix.str(), shader.str());
	return bs.c_str();
}
