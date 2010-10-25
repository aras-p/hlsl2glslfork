// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "hlslLinker.h"

#include "glslFunction.h"
#include "hlslCrossCompiler.h"

#include "hlslSupportLib.h"
#include "osinclude.h"
#include <algorithm>

// String table that maps attribute semantics to built-in GLSL attributes
static const char attribString[EAttrSemCount][32] = {
	"",
	"gl_Vertex",
	"",
	"",
	"gl_Normal",
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
	"xlat_attrib_binorm",
	"xlat_attrib_blendweights",
	"xlat_attrib_blendindices",
	"",
	"",
};

// String table that maps attribute semantics to built-in GLSL output varyings
static const char varOutString[EAttrSemCount][32] = {
	"",
	"gl_Position",
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
};

// String table that maps attribute semantics to built-in GLSL input varyings
static const char varInString[EAttrSemCount][32] = {
	"",
	"",
	"gl_FragCoord",
	"gl_FrontFacing",
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
};

// String table that maps attribute semantics to built-in GLSL fragment shader outputs
static const char resultString[EAttrSemCount][32] = {
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
	"gl_FragDepth",
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
			if ( (bUserVaryings && sem != EAttrSemPosition) || varOutString[sem][0] == 0 )
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
			}
			break;

		case EClassVarIn:
			// If using user varyings, create a user varying name
			if ( (bUserVaryings && sem != EAttrSemVPos && sem != EAttrSemVFace) || varInString[sem][0] == 0 )
			{
				outName = kUserVaryingPrefix;
				outName += stripSemanticModifier ( semantic, false );
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
			if ( sem != EAttrSemDepth)
			{
				pad = 4 - size;
				ctor = "vec4";
			}
			else
			{
				ctor = "float";
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



// Strip the semantic string of any modifiers (e.g. _centroid)
std::string HlslLinker::stripSemanticModifier( const std::string &semantic, bool bWarn ) 
{
	std::string newSemantic = semantic;

	int nCentroidLoc = semantic.find ( "_centroid" );
	if ( nCentroidLoc != -1 )
	{
		if ( bWarn )
		{
			infoSink.info << "Warning: '" << semantic << "' contains centroid modifier.  Modifier ignored because GLSL v1.10 does not support centroid\n";       
		}
		newSemantic = semantic.substr ( 0, nCentroidLoc );
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
	{ "vpos", EAttrSemVPos },
	{ "vface", EAttrSemVFace },
	{ "normal", EAttrSemNormal },
	{ "normal0", EAttrSemNormal },
	{ "tangent", EAttrSemTangent },
	{ "tangent0", EAttrSemTangent },
	{ "binormal", EAttrSemBinormal },
	{ "binormal0", EAttrSemBinormal },
	{ "blendweight", EAttrSemBlendWeight },
	{ "blendweight0", EAttrSemBlendWeight },
	{ "blendindices", EAttrSemBlendIndices },
	{ "blendindices0", EAttrSemBlendIndices },
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
};

// Determine the GLSL attribute semantic for a given HLSL semantic
EAttribSemantic HlslLinker::parseAttributeSemantic( const std::string &semantic )
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
		shader << (*fit)->getPrototype() << " {\n";
		shader << (*fit)->getLocalDecls(1) << "\n";
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



bool HlslLinker::link(HlslCrossCompiler* compiler, const char* entryFunc, bool usePrecision)
{
	std::vector<GlslFunction*> globalList;
	std::vector<GlslFunction*> functionList;
	std::string entryPoint;
	GlslFunction* funcMain = NULL;
	FunctionSet calledFunctions;
	std::set<TOperator> libFunctions;
	std::map<std::string,GlslSymbol*> globalSymMap;
	std::map<std::string,GlslStruct*> structMap;

	if (!compiler)
	{
		infoSink.info << "No shader compiler provided\n";
		return false;
	}

	EShLanguage lang = compiler->getLanguage();

	if (!entryFunc)
	{
		infoSink.info << "No shader entry function provided\n";
		return false;
	}

	entryPoint = GetEntryName (entryFunc);

	//build the list of functions
	HlslCrossCompiler *comp = static_cast<HlslCrossCompiler*>(compiler);

	std::vector<GlslFunction*> &fl = comp->functionList;

	for ( std::vector<GlslFunction*>::iterator fit = fl.begin(); fit < fl.end(); fit++)
	{
		if ( (*fit)->getName() == "__global__")
			globalList.push_back( *fit);
		else
			functionList.push_back( *fit);

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
	{
		infoSink.info << "Failed to resolve all called functions in the " << kShaderTypeNames[lang] << " shader\n";
	}

	//iterate over the functions, building a global list of structure declaractions and symbols
	// assume a single compilation unit for expediency (eliminates name clashes, as type checking
	// withing a single compilation unit has been performed)
	for (FunctionSet::iterator it=calledFunctions.begin(); it != calledFunctions.end(); it++)
	{
		//get each symbol and each structure, and add them to the map
		// checking that any previous entries are equivalent
		const std::vector<GlslSymbol*> &symList = (*it)->getSymbols();

		for (std::vector<GlslSymbol*>::const_iterator cit = symList.begin(); cit < symList.end(); cit++)
		{
			if ( (*cit)->getIsGlobal())
			{
				//should check for already added ones here
				globalSymMap[(*cit)->getName()] = *cit;
			}
		}

		//take each referenced library function, and add it to the set
		const std::set<TOperator> &libSet = (*it)->getLibFunctions();

		libFunctions.insert( libSet.begin(), libSet.end());
	}


	// The following code is what is used to generate the actual shader and "main"
	// function. The process is to take all the components collected above, and
	// write them to the appropriate code stream. Finally, a main function is
	// generated that calls the specified entrypoint. That main function uses
	// semantics on the arguments and return values to connect items appropriately.

	//
	// Write Library Functions
	if (!libFunctions.empty())
	{
		for (std::set<TOperator>::iterator it = libFunctions.begin(); it != libFunctions.end(); it++)
		{
			const std::string &func = getHLSLSupportCode(*it);
			if (func.size())
				shader << func << "\n";
		}
	}

	//
	//Structure addition hack
	// Presently, structures are not tracked per function, just dump them all
	// This could be improved by building a complete list of structures for the
	// shaders based on the variables in each function
	//
	{
		HlslCrossCompiler *comp = static_cast<HlslCrossCompiler*>(compiler);
		std::vector<GlslStruct*> &sList = comp->structList;

		if (!sList.empty())
		{
			for (std::vector<GlslStruct*>::iterator it = sList.begin(); it < sList.end(); it++)
			{
				shader << (*it)->getDecl() << "\n";
			}
		}
	}

	//
	// Write global variables
	//

	if (!globalSymMap.empty())
	{
		for (std::map<std::string,GlslSymbol*>::iterator sit = globalSymMap.begin(); sit != globalSymMap.end(); sit++)
		{
			sit->second->writeDecl(shader);
			shader << ";\n";

			if ( sit->second->getIsMutable() )
			{
				sit->second->writeDecl(shader, true);
				shader << ";\n";
			}         
		}
	}

	//
	// Write function declarations and definitions
	//
	EmitCalledFunctions (shader, calledFunctions);

	// 
	// Gather the uniforms into the uniform list
	//
	for (std::map<std::string, GlslSymbol*>::iterator it = globalSymMap.begin(); it != globalSymMap.end(); it++)
	{
		if (it->second->getQualifier() != EqtUniform)
			continue;

		ShUniformInfo infoStruct;
		infoStruct.name = new char[it->first.size()+1];
		strcpy( infoStruct.name, it->first.c_str());
		if (it->second->getSemantic() != "")
		{
			infoStruct.semantic = new char[it->second->getSemantic().size()+1];
			strcpy( infoStruct.semantic, it->second->getSemantic().c_str());
		}
		else
			infoStruct.semantic = 0;

		//gigantic hack, the enumerations are kept in alignment
		infoStruct.type = (EShType)it->second->getType();
		infoStruct.arraySize = it->second->getArraySize();

		if ( it->second->hasInitializer() )
		{
			int initSize = it->second->initializerSize();
			infoStruct.init = new float[initSize];
			memcpy( infoStruct.init, it->second->getInitializer(), sizeof(float) * initSize);
		}
		else
			infoStruct.init = 0;

		//TODO: need to add annotation

		uniforms.push_back( infoStruct);
	}

	//
	// Generate the main function
	//

		std::stringstream attrib;
		std::stringstream uniform;
		std::stringstream preamble;
		std::stringstream postamble;
		std::stringstream varying;
		std::stringstream call;
		const int pCount = funcMain->getParameterCount();

		preamble << "void main() {\n";
		const EGlslSymbolType retType = funcMain->getReturnType();
		GlslStruct *retStruct = funcMain->getStruct();
		if (  retType == EgstStruct)
		{
			assert(retStruct);
			preamble << "    " << retStruct->getName() << " xl_retval;\n";
		}
		else
		{
			if ( retType != EgstVoid)
			{
				preamble << "    ";
				writeType (preamble, retType, NULL, usePrecision?funcMain->getPrecision():EbpUndefined);
				preamble << " xl_retval;\n";
			}
		}

		// Write all mutable initializations
		if ( calledFunctions.size() > 0 )
		{
			for (FunctionSet::iterator fit = calledFunctions.begin(); fit != calledFunctions.end(); fit++)
			{
				std::string mutableDecls = (*fit)->getMutableDecls(1, calledFunctions.begin(), fit);

				if ( mutableDecls.size() > 0 )
				{
					preamble << mutableDecls;
				}
			}
		}

		call << "    ";
		if (retType != EgstVoid)
			call << "xl_retval = " << funcMain->getName() << "( ";
		else
			call << funcMain->getName() << "( ";

		// pass main function parameters
		for (int ii=0; ii<pCount; ii++)
		{
			GlslSymbol *sym = funcMain->getParameter(ii);
			EAttribSemantic attrSem = parseAttributeSemantic( sym->getSemantic());

			switch (sym->getQualifier())
			{

			// -------- IN & OUT parameters
			case EqtIn:
			case EqtInOut:
				if ( sym->getType() != EgstStruct)
				{
					std::string name, ctor;
					int pad;

					if ( getArgumentData( sym, lang==EShLangVertex ? EClassAttrib : EClassVarIn, name, ctor, pad) )
					{
						// In fragment shader, pass zero for POSITION inputs
						bool ignoredPositionInFragment = false;
						if (lang == EShLangFragment && attrSem == EAttrSemPosition)
						{
							call << ctor << "(0.0)";
							ignoredPositionInFragment = true;
						}
						// For "in" parameters, just call directly to the main
						else if ( sym->getQualifier() != EqtInOut )
						{
							call << ctor << "(" << name;
							for (int ii = 0; ii<pad; ii++)
								call << ", 0.0";
							call << ")";
						}
						// For "inout" parameters, declare a temp and initialize the temp
						else
						{
							preamble << "    ";
							writeType (preamble, sym->getType(), NULL, usePrecision?sym->getPrecision():EbpUndefined);
							preamble << " xlt_" << sym->getName() << " = ";
							preamble << ctor << "(" << name;
							for (int ii = 0; ii<pad; ii++)
								preamble << ", 0.0";
							preamble << ");\n";
						}

						if (lang == EShLangVertex) // vertex shader: deal with gl_ attributes
						{
							if ( strncmp( name.c_str(), "gl_", 3))
							{
								int typeOffset = 0;

								// If the type is integer or bool based, we must convert to a float based
								// type.  This is because GLSL does not allow int or bool based vertex attributes.
								if ( sym->getType() >= EgstInt && sym->getType() <= EgstInt4)
								{
									typeOffset += 4;
								}

								if ( sym->getType() >= EgstBool && sym->getType() <= EgstBool4)
								{
									typeOffset += 8;
								}

								// This is an undefined attribute
								attrib << "attribute " << getTypeString((EGlslSymbolType)(sym->getType() + typeOffset)) << " " << name << ";\n";
							}
						}

						if (lang == EShLangFragment) // deal with varyings
						{
							if (!ignoredPositionInFragment)
								AddToVaryings (varying, sym->getPrecision(), ctor, name);
						}
					}
					else
					{
						//should deal with fall through cases here
						assert(0);
						infoSink.info << "Unsupported type for shader entry parameter (";
						infoSink.info << getTypeString(sym->getType()) << ")\n";
					}
				}
				else
				{
					//structs must pass the struct, then process per element
					GlslStruct *Struct = sym->getStruct();
					assert(Struct);

					//first create the temp
					std::string tempVar = "xlt_" + sym->getName();
					preamble << "    " << Struct->getName() << " ";
					preamble << tempVar <<";\n";
					call << tempVar;

					const int elem = Struct->memberCount();
					for (int jj=0; jj<elem; jj++)
					{
						const GlslStruct::member &current = Struct->getMember(jj);
						EAttribSemantic memberSem = parseAttributeSemantic (current.semantic);
						std::string name, ctor;
						int pad;
						int numArrayElements = 1;
						bool bIsArray = false;

						// If it is an array, loop over each member
						if ( current.arraySize > 0 )
						{
							numArrayElements = current.arraySize;
							bIsArray = true;
						}

						for ( int arrayIndex = 0; arrayIndex <  numArrayElements; arrayIndex++ )
						{
							if ( getArgumentData2( current.name, current.semantic, current.type,
								lang==EShLangVertex ? EClassAttrib : EClassVarIn, name, ctor, pad, arrayIndex ) )
							{

								preamble << "    ";
								preamble << tempVar << "." << current.name;

								if ( bIsArray )
									preamble << "[" << arrayIndex << "]";

								// In fragment shader, pass zero for POSITION inputs
								bool ignoredPositionInFragment = false;
								if (lang == EShLangFragment && memberSem == EAttrSemPosition)
								{
									preamble << " = " << ctor << "(0.0);\n";
									ignoredPositionInFragment = true;
								}
								else
								{
									preamble << " = " << ctor << "( " << name;
									for (int ii = 0; ii<pad; ii++)
										preamble << ", 0.0";
									preamble << ");\n";
								}

								if (lang == EShLangVertex) // vertex shader: gl_ attributes
								{
									if ( strncmp( name.c_str(), "gl_", 3))
									{

										int typeOffset = 0;

										// If the type is integer or bool based, we must convert to a float based
										// type.  This is because GLSL does not allow int or bool based vertex attributes.
										if ( current.type >= EgstInt && current.type <= EgstInt4)
										{
											typeOffset += 4;
										}

										if ( current.type >= EgstBool && current.type <= EgstBool4)
										{
											typeOffset += 8;
										}

										// This is an undefined attribute
										attrib << "attribute " << getTypeString((EGlslSymbolType)(current.type + typeOffset)) << " " << name << ";\n";

									}
								}
								
								if (lang == EShLangFragment) // deal with varyings
								{
									if (!ignoredPositionInFragment)
										AddToVaryings (varying, current.precision, ctor, name);
								}
							}
							else
							{
								//should deal with fall through cases here
								assert(0);
								infoSink.info << "Unsupported type for struct element in shader entry parameter (";
								infoSink.info << getTypeString(current.type) << ")\n";
							}
						}
					}
				}

				//
				// NOTE: This check only breaks out of the case if we have an "in" parameter, for
				//       "inout" it will fallthrough to the next case
				//
				if ( sym->getQualifier() != EqtInOut )
				{
					break;
				}


			// -------- OUT parameters; also fall-through for INOUT (see the check above)
			case EqtOut:

				if ( sym->getType() != EgstStruct)
				{
					std::string name, ctor;
					int pad;

					if ( getArgumentData( sym, lang==EShLangVertex ? EClassVarOut : EClassRes, name, ctor, pad) )
					{
						// For "inout" parameters, the preamble was already written so no need to do it here.
						if ( sym->getQualifier() != EqtInOut )
						{
							preamble << "    ";
							writeType (preamble, sym->getType(), NULL, usePrecision?sym->getPrecision():EbpUndefined);
							preamble << " xlt_" << sym->getName() << ";\n";                     
						}
						
						if (lang == EShLangVertex) // deal with varyings
						{
							AddToVaryings (varying, sym->getPrecision(), ctor, name);
						}

						call << "xlt_" << sym->getName();

						postamble << "    ";
						postamble << name << " = " << ctor << "( xlt_" <<sym->getName();
						for (int ii = 0; ii<pad; ii++)
							postamble << ", 0.0";

						postamble << ");\n";
					}
					else
					{
						//should deal with fall through cases here
						assert(0);
						infoSink.info << "Unsupported type for shader entry parameter (";
						infoSink.info << getTypeString(sym->getType()) << ")\n";
					}
				}
				else
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
						const GlslStruct::member &current = Struct->getMember(ii);
						std::string name, ctor;
						int pad;

						if ( getArgumentData2( current.name, current.semantic, current.type, lang==EShLangVertex ? EClassVarOut : EClassRes, name, ctor, pad, 0) )
						{
							postamble << "    ";
							postamble << name << " = " << ctor;
							postamble << "( " << tempVar << "." << current.name;
							for (int ii = 0; ii<pad; ii++)
								postamble << ", 0.0";

							postamble << ");\n";

							if (lang == EShLangVertex) // deal with varyings
							{
								AddToVaryings (varying, current.precision, ctor, name);
							}
						}
						else
						{
							//should deal with fall through cases here
							assert(0);
							infoSink.info << "Unsupported type in struct element for shader entry parameter (";
							infoSink.info << getTypeString(current.type) << ")\n";
						}
					}
				}
				break;

			case EqtUniform:
				uniform << "uniform ";
				writeType (uniform, sym->getType(), NULL, usePrecision?sym->getPrecision():EbpUndefined);
				uniform << " xlu_" << sym->getName() << ";\n";
				call << "xlu_" << sym->getName();
				break;

			default:
				assert(0);
			};
			if (ii != pCount -1)
				call << ", ";
		}

		call << ");\n";


		// -------- return value of main entry point
		if (retType != EgstVoid)
		{

			if (retType != EgstStruct)
			{
				std::string name, ctor;
				int pad;

				if ( getArgumentData2( "", funcMain->getSemantic(), retType, lang==EShLangVertex ? EClassVarOut : EClassRes,
					name, ctor, pad, 0) )
				{

					postamble << "    ";
					postamble << name << " = " << ctor << "( xl_retval";
					for (int ii = 0; ii<pad; ii++)
						postamble << ", 0.0";

					postamble << ");\n";

					if (lang == EShLangVertex) // deal with varyings
					{
						AddToVaryings (varying, funcMain->getPrecision(), ctor, name);
					}
				}
				else
				{
					//should deal with fall through cases here
					assert(0);
					infoSink.info << (lang==EShLangVertex ? "Unsupported type for shader return value (" : "Unsupported return type for shader entry function (");
					infoSink.info << getTypeString(retType) << ")\n";
				}
			}
			else
			{
				const int elem = retStruct->memberCount();
				for (int ii=0; ii<elem; ii++)
				{
					const GlslStruct::member &current = retStruct->getMember(ii);
					std::string name, ctor;
					int pad;
					int numArrayElements = 1;
					bool bIsArray = false;

					if (lang == EShLangVertex) // vertex shader
					{
						// If it is an array, loop over each member
						if ( current.arraySize > 0 )
						{
							numArrayElements = current.arraySize;
							bIsArray = true;
						}
					}

					for ( int arrayIndex = 0; arrayIndex < numArrayElements; arrayIndex++ )
					{

						if ( getArgumentData2( current.name, current.semantic, current.type, lang==EShLangVertex ? EClassVarOut : EClassRes, name, ctor, pad, arrayIndex) )
						{
							postamble << "    ";
							postamble << name;                                                            
							postamble << " = " << ctor;
							postamble << "( xl_retval." << current.name;
							if ( bIsArray )
							{
								postamble << "[" << arrayIndex << "]";
							}
							for (int ii = 0; ii<pad; ii++)
								postamble << ", 0.0";

							postamble << ");\n";

							if (lang == EShLangVertex) // deal with varyings
							{
								AddToVaryings (varying, current.precision, ctor, name);
							}
						}
						else
						{
							//should deal with fall through cases here
							assert(0);
							infoSink.info << (lang==EShLangVertex ? "Unsupported element type in struct for shader return value (" : "Unsupported struct element type in return type for shader entry function (");
							infoSink.info << getTypeString(current.type) << ")\n";
						}
					}
				}
			}
		}
		else
		{
			if (lang == EShLangFragment) // fragment shader
			{
				// If no return type, close off the output
				postamble << ";\n";
			}
		}

		postamble << "}\n\n";

		EmitIfNotEmpty (shader, uniform);
		EmitIfNotEmpty (shader, attrib);
		EmitIfNotEmpty (shader, varying);

		shader << preamble.str() << "\n";
		shader << call.str() << "\n";
		shader << postamble.str() << "\n";

	return true;
}


static std::string CleanupShaderText (const std::string& str)
{
	std::string res;
	size_t n = str.size();
	res.reserve (n);
	for (size_t i = 0; i < n; ++i)
	{
		char c = str[i];
		if (c == '\n' && i!=0 && str[i-1] == '\n')
			continue;
		res.push_back (c);
	}
	return res;
}



const char* HlslLinker::getShaderText() const 
{
	bs = CleanupShaderText (shader.str());
	return bs.c_str();
}
