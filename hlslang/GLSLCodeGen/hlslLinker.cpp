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

//=================================================================================================================================
//
// ATI Research, Inc.
//
// Implementation of HlslLinker
//=================================================================================================================================


//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================
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
	""
};

// String table that maps attribute semantics to built-in GLSL output varyings
static const char varOutString[EAttrSemCount][32] = {
	"",
	"gl_Position",
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
	""
};

// String table that maps attribute semantics to built-in GLSL input varyings
static const char varInString[EAttrSemCount][32] = {
	"",
	"gl_FragCoord",
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
	""
};

// String table that maps attribute semantics to built-in GLSL fragment shader outputs
static const char resultString[EAttrSemCount][32] = {
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
	"gl_FragDepth",
	""
};

//=================================================================================================================================
//
//          Constructor(s) / Destructor(s) Block 
//
//=================================================================================================================================

//=========================================================================================================
/// Constructor
//=========================================================================================================
HlslLinker::HlslLinker(int dOptions) : TLinker(infoSink), debugOptions(dOptions) 
{
	for ( int i = 0; i < EAttrSemCount; i++)
	{
		userAttribString[i][0] = 0;
	}

	// By default, use GL build-in varyings
	bUserVaryings = false;

	// By default, do not output a shader header
	bOutputShaderHeader = false;
}

//=========================================================================================================
/// Destructor
//=========================================================================================================
HlslLinker::~HlslLinker()
{
	for ( std::vector<ShUniformInfo>::iterator it = uniforms.begin(); it != uniforms.end(); it++)
	{
		delete [] it->name;
		delete [] it->semantic;
		delete [] it->init;
	}
}



//=================================================================================================================================
//
//          Protected Methods Block
//
//=================================================================================================================================

//=========================================================================================================
/// Get the GLSL name, constructor type and padding for a given argument 
/// \param name
///   The name of the symbol
/// \param semantic
///   The HLSL semantic for the symbol
/// \param type
///   The GLSL type of the symbol
/// \param c
///   The classifier for the symbol (e.g. attribute, varying, etc.)
/// \param outName
///   The final name to use for the symbol in the output GLSL code
/// \param ctor
///   The constructor to use for creating the GLSL symbol
/// \param pad
///   Any padding required in constructing the symbol
/// \param semanticOffset
///   When using an array variable bound to a semantic, this is the array offset
/// \return 
///   True if argument data was retrieved succesfully, false otherwise.
//=========================================================================================================
bool HlslLinker::getArgumentData( const std::string &name, const std::string &semantic, EGlslSymbolType type,
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
				outName = "xlat_varying_";
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
			pad = 4 - size;
			ctor = "vec4";
			break;

		case EClassVarIn:
			// If using user varyings, create a user varying name
			if ( bUserVaryings || varInString[sem][0] == 0 )
			{
				outName = "xlat_varying_";
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
				ctor = "float";
			break;

		case EClassUniform:
			assert(0); // this should have been stripped
			return false; 
		};


	}
	else
	{
		//these should always match exactly
		outName = "xlat_uniform_";
		outName += name;
	}

	return true;
}

//=========================================================================================================
/// Get the GLSL name, constructor type and padding for a given symbol
/// \param symbol
///   The symbol to get data for
/// \param c
///   The classifier for the symbol (e.g. attribute, varying, etc.)
/// \param outName
///   The final name to use for the symbol in the output GLSL code
/// \param ctor
///   The constructor to use for creating the GLSL symbol
/// \param pad
///   Any padding required in constructing the symbol
/// \return 
///   True if argument data was retrieved succesfully, false otherwise.
//=========================================================================================================
bool HlslLinker::getArgumentData( GlslSymbol* sym, EClassifier c, std::string &outName,
								 std::string &ctor, int &pad)
{
	const std::string &name = sym->getName();
	const std::string &semantic = sym->getSemantic();
	EGlslSymbolType type = sym->getType();

	return getArgumentData( name, semantic, type, c, outName, ctor, pad);
}

//=================================================================================================================================
//
//          Public Methods Methods Block
//
//=================================================================================================================================


//=========================================================================================================
/// If the user elects to use user attributes rather than built-ins, this function will set the user
/// attribute name the user wishes to use for the semantic passed in.
/// \param eSemantic
///   The semantic to set to a user attribute name
/// \param pName
///   The name to use for the user attribute 
/// \return
///   True if the semantic value is valid, false otherwise.
//=========================================================================================================
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

//=========================================================================================================
/// Enable the the output of a shader header to the GLSL output stream.  
/// \param bOutputShaderHeader 
///      If true, the string specified by "precisionString" will be output at the top of the GLSL output
/// \param shaderHeaderString
///      The string to output at the top of the GLSL output
//=========================================================================================================   
bool HlslLinker::setShaderHeader ( bool outputShaderHeader, const char *shaderHeaderString )
{
	bOutputShaderHeader = outputShaderHeader;
	outputShaderHeaderString = shaderHeaderString;

	return true;
}


//=========================================================================================================
/// Strip the semantic string of any modifiers (e.g. _centroid)
/// \param semantic
///   String of HLSL semantic for variable
/// \param bWarn
///   Boolean value of whether to add a warning to the output log about the existence of the semantic
///   modifier.
/// \return The semantic name with the modifier stripped
//=========================================================================================================
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

//=========================================================================================================
/// Determine the GLSL attribute semantic for a given HLSL semantic
/// \param semantic
///   String of HLSL semantic for variable
/// \return The GLSL attribute semantic associated with the HLSL smenatic, or none if not matched
//=========================================================================================================
EAttribSemantic HlslLinker::parseAttributeSemantic( const std::string &semantic )
{

	std::string curSemantic = stripSemanticModifier (semantic, true);

	if ( !_stricmp(curSemantic.c_str(), "position"))
		return EAttrSemPosition;

	if ( !_stricmp(curSemantic.c_str(), "position0"))
		return EAttrSemPosition;
	
	if ( !_stricmp(curSemantic.c_str(), "vpos"))
		return EAttrSemPosition;

	if ( !_stricmp(curSemantic.c_str(), "normal"))
		return EAttrSemNormal;

	if ( !_stricmp(curSemantic.c_str(), "normal0"))
		return EAttrSemNormal;

	if ( !_stricmp(curSemantic.c_str(), "tangent"))
		return EAttrSemTangent;

	if ( !_stricmp(curSemantic.c_str(), "tangent0"))
		return EAttrSemTangent;

	if ( !_stricmp(curSemantic.c_str(), "binormal"))
		return EAttrSemBinormal;

	if ( !_stricmp(curSemantic.c_str(), "binormal0"))
		return EAttrSemBinormal;

	if ( !_stricmp(curSemantic.c_str(), "blendweight"))
		return EAttrSemBlendWeight;

	if ( !_stricmp(curSemantic.c_str(), "blendweight0"))
		return EAttrSemBlendWeight;

	if ( !_stricmp(curSemantic.c_str(), "blendindices"))
		return EAttrSemBlendIndices;

	if ( !_stricmp(curSemantic.c_str(), "blendindices0"))
		return EAttrSemBlendIndices;

	if ( !_stricmp(curSemantic.c_str(), "color"))
		return EAttrSemColor0;

	if ( !_stricmp(curSemantic.c_str(), "color0"))
		return EAttrSemColor0;

	if ( !_stricmp(curSemantic.c_str(), "color1"))
		return EAttrSemColor1;

	if ( !_stricmp(curSemantic.c_str(), "color2"))
		return EAttrSemColor2;

	if ( !_stricmp(curSemantic.c_str(), "color3"))
		return EAttrSemColor3;

	if ( !_stricmp(curSemantic.c_str(), "texcoord"))
		return EAttrSemTex0;

	if ( !_stricmp(curSemantic.c_str(), "texcoord0"))
		return EAttrSemTex0;

	if ( !_stricmp(curSemantic.c_str(), "texcoord1"))
		return EAttrSemTex1;

	if ( !_stricmp(curSemantic.c_str(), "texcoord2"))
		return EAttrSemTex2;

	if ( !_stricmp(curSemantic.c_str(), "texcoord3"))
		return EAttrSemTex3;

	if ( !_stricmp(curSemantic.c_str(), "texcoord4"))
		return EAttrSemTex4;

	if ( !_stricmp(curSemantic.c_str(), "texcoord5"))
		return EAttrSemTex5;

	if ( !_stricmp(curSemantic.c_str(), "texcoord6"))
		return EAttrSemTex6;

	if ( !_stricmp(curSemantic.c_str(), "texcoord7"))
		return EAttrSemTex7;

	if ( !_stricmp(curSemantic.c_str(), "texcoord8"))
		return EAttrSemTex8;

	if ( !_stricmp(curSemantic.c_str(), "texcoord9"))
		return EAttrSemTex9;

	if ( !_stricmp(curSemantic.c_str(), "depth"))
		return EAttrSemDepth;
	
	return EAttrSemUnknown;
}



//=========================================================================================================
/// Add the functions called by a function to the function set
/// \param func
///   The function for which all called functions will be added
/// \param funcSet
///   The set of currently called functions
/// \param funcList
///   The list of all functions
/// \return
///   True if all functions are found in the funcList, false otherwise.
//=========================================================================================================
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

static void EmitCalledFunctions (std::stringstream& shader, const FunctionSet& functions, bool comments)
{
	if (functions.empty())
		return;

	if (comments)
		shader << "\n//\n// Function declarations\n//\n\n";

	for (FunctionSet::const_reverse_iterator fit = functions.rbegin(); fit != functions.rend(); fit++) // emit backwards, will put least used ones in front
	{
		shader << (*fit)->getPrototype() << ";\n";
	}

	if (comments)
		shader << "\n//\n// Function definitions\n//\n\n";

	for (FunctionSet::const_reverse_iterator fit = functions.rbegin(); fit != functions.rend(); fit++) // emit backwards, will put least used ones in front
	{
		shader << (*fit)->getPrototype() << " {\n";
		shader << (*fit)->getLocalDecls(1) << "\n";
		shader << (*fit)->getCode() << "\n"; //has embedded }
		shader << "\n";
	}
}

//=========================================================================================================
/// This function is the main function that initiates code generation for the shader.  
/// \param hList
///   The list of functions to link
/// \param vertEntryFunc
///   The name of the vertex shader entrypoint
/// \param fragEntryFunc
///   The name of the fragment shader entrypoint
/// \return
///   True if linking is succesful, false otherwise
//=========================================================================================================
bool HlslLinker::link(THandleList& hList, const char* vertEntryFunc, const char* fragEntryFunc, bool comments)
{
	std::vector<GlslFunction*> globalList;
	std::vector<GlslFunction*> functionList;
	GlslFunction *vertMain = 0;
	GlslFunction *fragMain = 0;
	FunctionSet calledFunctions[2];
	std::set<TOperator> libFunctions[2];
	std::map<std::string,GlslSymbol*> globalSymMap[2];
	std::map<std::string,GlslStruct*> structMap[2];
	std::string vertEntry, fragEntry;

	if (!vertEntryFunc && !fragEntryFunc)
	{
		infoSink.info << "No shader entry functions provided\n";
		return false;
	}

	if (vertEntryFunc)
	{
		vertEntry = vertEntryFunc;
		if (vertEntry == "main")
			vertEntry = "xlat_main";
	}

	if (fragEntryFunc)
	{
		fragEntry = fragEntryFunc;
		if (fragEntry == "main")
			fragEntry = "xlat_main";
	}

	//build the list of functions
	for (THandleList::iterator it = hList.begin(); it < hList.end(); it++ )
	{
		HlslCrossCompiler *comp = static_cast<HlslCrossCompiler*>( *it);

		std::vector<GlslFunction*> &fl = comp->functionList;

		for ( std::vector<GlslFunction*>::iterator fit = fl.begin(); fit < fl.end(); fit++)
		{
			if ( (*fit)->getName() == "__global__")
				globalList.push_back( *fit);
			else
				functionList.push_back( *fit);

			if ( (*fit)->getName() == vertEntry)
			{
				if (vertMain)
				{
					infoSink.info << "Vertex entry function cannot be overloaded\n";
					return false;
				}
				vertMain = *fit;
			}

			if ( (*fit)->getName() == fragEntry)
			{
				if (fragMain)
				{
					infoSink.info << "Fragment entry function cannot be overloaded\n";
					return false;
				}
				fragMain = *fit;
			}
		}
	}

	//check to ensure that we found the entry functions
	if ( fragEntryFunc && !fragMain)
	{
		infoSink.info << "Failed to find fragment entry function: '" << fragEntry <<"'\n";
		return false;
	}

	if ( vertEntryFunc && !vertMain)
	{
		infoSink.info << "Failed to find vertex entry function: '" << vertEntry <<"'\n";
		return false;
	}

	//add all the vertex functions to the list
	if (vertEntryFunc)
	{
		calledFunctions[0].push_back( vertMain);

		if ( !addCalledFunctions( vertMain, calledFunctions[0], functionList))
		{
			infoSink.info << "Failed to resolve all called functions in the vertex shader\n";
		}
	}

	//add all the fragment functions to the list
	if (fragEntryFunc)
	{
		calledFunctions[1].push_back( fragMain);

		if ( !addCalledFunctions( fragMain, calledFunctions[1], functionList))
		{
			infoSink.info << "Failed to resolve all called functions in the fragment shader\n";
		}
	}

	//iterate over the functions, building a global list of structure declaractions and symbols
	// assume a single compilation unit for expediency (eliminates name clashes, as type checking
	// withing a single compilation unit has been performed)
	for (int ii=0; ii<2; ii++)
	{
		for (FunctionSet::iterator it=calledFunctions[ii].begin(); it != calledFunctions[ii].end(); it++)
		{
			//get each symbol and each structure, and add them to the map
			// checking that any previous entries are equivalent
			const std::vector<GlslSymbol*> &symList = (*it)->getSymbols();

			for (std::vector<GlslSymbol*>::const_iterator cit = symList.begin(); cit < symList.end(); cit++)
			{
				if ( (*cit)->getIsGlobal())
				{
					//should check for already added ones here
					globalSymMap[ii][(*cit)->getName()] = *cit;
				}
			}

			//take each referenced library function, and add it to the set
			const std::set<TOperator> &libSet = (*it)->getLibFunctions();

			libFunctions[ii].insert( libSet.begin(), libSet.end());
		}
	}


	// The following code is what is used to generate the actual shader and "main"
	// function. The process is to take all the components collected above, and
	// write them to the appropriate code stream. Finally, a main function is
	// generated that calls the specified entrypoint. That main function uses
	// semantics on the arguments and return values to connect items appropriately.

	//
	// Write shader header if one was provided
	//
	if ( bOutputShaderHeader )
	{
		vertShader << outputShaderHeaderString;
		fragShader << outputShaderHeaderString;
	}

	//
	// Write Library Functions
	//
	if (libFunctions[0].size() > 0)
	{
		if (comments)
			vertShader << "//\n// Translator library functions\n//\n\n";
		for (std::set<TOperator>::iterator it = libFunctions[0].begin(); it != libFunctions[0].end(); it++)
		{
			const std::string &func = getHLSLSupportCode( *it);
			if (func.size())
				vertShader << func << "\n";
		}
	}

	if (libFunctions[1].size() > 0)
	{
		if (comments)
			fragShader << "//\n// Translator library functions\n//\n\n";
		for (std::set<TOperator>::iterator it = libFunctions[1].begin(); it != libFunctions[1].end(); it++)
		{
			const std::string &func = getHLSLSupportCode( *it);
			if (func.size())
				fragShader << func << "\n";

		}
	}

	//
	//Structure addition hack
	// Presently, structures are not tracked per function, just dump them all
	// This could be improved by building a complete list of structures for the
	// vertex and fragment shaders based on the variables in each function
	//
	{
		HlslCrossCompiler *comp = static_cast<HlslCrossCompiler*>(*hList.begin());
		std::vector<GlslStruct*> &sList = comp->structList;

		if (sList.size() > 0)
		{
			if (comments)
			{
				fragShader << "//\n// Structure definitions\n//\n\n";
				vertShader << "//\n// Structure definitions\n//\n\n";
			}

			for (std::vector<GlslStruct*>::iterator it = sList.begin(); it < sList.end(); it++)
			{
				fragShader << (*it)->getDecl() << "\n";
				vertShader << (*it)->getDecl() << "\n";
			}
		}
	}

	//
	// Write global variables
	//
	if (globalSymMap[0].size() > 0 )
	{
		if (comments)
			vertShader << "\n//\n// Global variable definitions\n//\n\n";

		for (std::map<std::string,GlslSymbol*>::iterator sit = globalSymMap[0].begin(); sit != globalSymMap[0].end(); sit++)
		{
			sit->second->writeDecl(vertShader);
			vertShader << ";\n";

			if ( sit->second->getIsMutable() )
			{
				sit->second->writeDecl(vertShader, true);
				vertShader << ";\n";
			}         
		}
	}

	if (globalSymMap[1].size() > 0 )
	{
		if (comments)
			fragShader << "\n//\n// Global variable definitions\n//\n\n";

		for (std::map<std::string,GlslSymbol*>::iterator sit = globalSymMap[1].begin(); sit != globalSymMap[1].end(); sit++)
		{
			sit->second->writeDecl(fragShader);
			fragShader << ";\n";

			if ( sit->second->getIsMutable() )
			{
				sit->second->writeDecl(fragShader, true);
				fragShader << ";\n";
			}         
		}
	}

	//
	// Write function declarations and definitions
	//
	EmitCalledFunctions (vertShader, calledFunctions[0], comments);
	EmitCalledFunctions (fragShader, calledFunctions[1], comments);

	// 
	// Gather the uniforms into the uniform list
	//
	for (std::map<std::string, GlslSymbol*>::iterator it = globalSymMap[0].begin(); it != globalSymMap[0].end(); it++)
	{
		ShUniformInfo infoStruct;
		if (it->second->getQualifier()  == EqtUniform)
		{
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
	}

	for (std::map<std::string, GlslSymbol*>::iterator it = globalSymMap[1].begin(); it != globalSymMap[1].end(); it++)
	{
		ShUniformInfo infoStruct;
		if (it->second->getQualifier()  == EqtUniform)
		{
			if ( globalSymMap[0].find(it->first) != globalSymMap[0].end())
				continue; //already added
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

			if ( it->second->hasInitializer())
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
	}

	//
	// Generate the main functions
	//

	if (vertMain)
	{
		std::stringstream attrib;
		std::stringstream uniform;
		std::stringstream preamble;
		std::stringstream postamble;
		std::stringstream varying;
		std::stringstream call;
		const int pCount = vertMain->getParameterCount();

		if (comments)
			preamble << "//\n// Translator's entry point\n//\n";
		preamble << "void main() {\n";
		const EGlslSymbolType retType = vertMain->getReturnType();
		GlslStruct *retStruct = vertMain->getStruct();
		if (  retType == EgstStruct)
		{
			assert(retStruct);
			preamble << "    " << retStruct->getName() << " xlat_retVal;\n";
		}
		else
		{
			if ( retType != EgstVoid)
			{
				preamble << "    " << getTypeString(retType) << " xlat_retVal;\n";
			}
		}

		// Write all mutable initializations
		if ( calledFunctions[0].size() > 0 )
		{
			for (FunctionSet::iterator fit = calledFunctions[0].begin(); fit != calledFunctions[0].end(); fit++)
			{
				std::string mutableDecls = (*fit)->getMutableDecls(1, calledFunctions[0].begin(), fit);

				if ( mutableDecls.size() > 0 )
				{
					preamble << mutableDecls;
				}
			}
		}

		call << "    ";
		if (retType != EgstVoid)
			call << "xlat_retVal = " << vertMain->getName() << "( ";
		else
			call << vertMain->getName() << "( ";

		for (int ii=0; ii<pCount; ii++)
		{
			GlslSymbol *sym = vertMain->getParameter(ii);
			EAttribSemantic attrSem = parseAttributeSemantic( sym->getSemantic());

			switch (sym->getQualifier())
			{

			case EqtIn:
			case EqtInOut:
				if ( sym->getType() != EgstStruct)
				{
					std::string name, ctor;
					int pad;

					if ( getArgumentData( sym, EClassAttrib, name, ctor, pad) )
					{
						// For "in" parameters, just call directly to the main
						if ( sym->getQualifier() != EqtInOut )
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
							preamble << getTypeString(sym->getType()) << " xlat_temp_" << sym->getName() << " = ";
							preamble << ctor << "(" << name;
							for (int ii = 0; ii<pad; ii++)
								preamble << ", 0.0";
							preamble << ");\n";
						}

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
					std::string tempVar = "xlat_temp_" + sym->getName();
					preamble << "    " << Struct->getName() << " ";
					preamble << tempVar <<";\n";
					call << tempVar;

					const int elem = Struct->memberCount();
					for (int jj=0; jj<elem; jj++)
					{
						const GlslStruct::member &current = Struct->getMember(jj);
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
							if ( getArgumentData( current.name, current.semantic, current.type,
								EClassAttrib, name, ctor, pad, arrayIndex ) )
							{

								preamble << "    ";
								preamble << tempVar << "." << current.name;

								if ( bIsArray )
									preamble << "[" << arrayIndex << "]";

								preamble << " = " << ctor << "( " << name;
								for (int ii = 0; ii<pad; ii++)
									preamble << ", 0.0";

								preamble << ");\n";

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

				// Also a fallthrough for "inout" (see if check above)
			case EqtOut:

				if ( sym->getType() != EgstStruct)
				{
					std::string name, ctor;
					int pad;

					if ( getArgumentData( sym, EClassVarOut, name, ctor, pad) )
					{
						// For "inout" parameters, the preamble was already written so no need to do it here.
						if ( sym->getQualifier() != EqtInOut )
						{
							preamble << "    ";
							preamble << getTypeString(sym->getType()) << " xlat_temp_" << sym->getName() << ";\n";                     
						}

						// If using user varying, add it to the varying list
						if ( strstr ( name.c_str(), "xlat_varying" ) )
						{
							varying << "varying vec4 " << name <<";\n" ;
						}                     

						call << "xlat_temp_" << sym->getName();

						postamble << "    ";
						postamble << name << " = " << ctor << "( xlat_temp_" <<sym->getName();
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
					std::string tempVar = "xlat_temp_" + sym->getName();

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

						if ( getArgumentData( current.name, current.semantic, current.type, EClassVarOut, name, ctor, pad) )
						{
							postamble << "    ";
							postamble << name << " = " << ctor;
							postamble << "( " << tempVar << "." << current.name;
							for (int ii = 0; ii<pad; ii++)
								postamble << ", 0.0";

							postamble << ");\n";

							// If using user varying, add it to the varying list
							if ( strstr ( name.c_str(), "xlat_varying" ) )
							{
								varying << "varying vec4 " << name <<";\n" ;
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
				uniform << "uniform " << getTypeString(sym->getType()) << " ";
				uniform << "xlat_uniform_" << sym->getName() << ";\n";
				call << "xlat_uniform_" << sym->getName();
				break;

			default:
				assert(0);
			};
			if (ii != pCount -1)
				call << ", ";
		}

		if (retType != EgstVoid)
		{

			if (retType != EgstStruct)
			{
				std::string name, ctor;
				int pad;

				if ( getArgumentData( "", vertMain->getSemantic(), retType, EClassVarOut,
					name, ctor, pad) )
				{

					postamble << "    ";
					postamble << name << " = " << ctor << "( xlat_retVal";
					for (int ii = 0; ii<pad; ii++)
						postamble << ", 0.0";

					postamble << ");\n";

					// If using user varying, add it to the varying list
					if ( strstr ( name.c_str(), "xlat_varying" ) )
					{
						varying << "varying vec4 " << name <<";\n" ;
					}
				}
				else
				{
					//should deal with fall through cases here
					assert(0);
					infoSink.info << "Unsupported type for shader return value (";
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

					// If it is an array, loop over each member
					if ( current.arraySize > 0 )
					{
						numArrayElements = current.arraySize;
						bIsArray = true;
					}

					for ( int arrayIndex = 0; arrayIndex < numArrayElements; arrayIndex++ )
					{

						if ( getArgumentData( current.name, current.semantic, current.type, EClassVarOut, name, ctor, pad, arrayIndex) )
						{
							postamble << "    ";
							postamble << name;                                                            
							postamble << " = " << ctor;
							postamble << "( xlat_retVal." << current.name;
							if ( bIsArray )
							{
								postamble << "[" << arrayIndex << "]";
							}
							for (int ii = 0; ii<pad; ii++)
								postamble << ", 0.0";

							postamble << ");\n";

							// If using user varying, add it to the varying list
							if ( strstr ( name.c_str(), "xlat_varying" ) )
							{
								varying << "varying vec4 " << name <<";\n" ;
							}
						}
						else
						{
							//should deal with fall through cases here
							assert(0);
							infoSink.info << "Unsupported element type in struct for shader return value (";
							infoSink.info << getTypeString(current.type) << ")\n";
						}
					}
				}
			}
		}

		call << ");\n";
		postamble << "}\n\n";

		if (uniform.str().size() )
		{
			if (comments)
				vertShader << "//\n// Uniform Arguments\n//\n";
			vertShader << uniform.str() << "\n";
		}

		if (attrib.str().size() )
		{
			if (comments)
				vertShader << "//\n// Attributes\n//\n";
			vertShader << attrib.str() << "\n";
		}

		if (varying.str().size() )
		{
			if (comments)
				vertShader << "//\n// User varying\n//\n";
			vertShader << varying.str() << "\n";
		}

		vertShader << preamble.str() << "\n";
		vertShader << call.str() << "\n";
		vertShader << postamble.str() << "\n";

	}

	if (fragMain)
	{
		std::stringstream uniform;
		std::stringstream preamble;
		std::stringstream postamble;
		std::stringstream varying;
		std::stringstream call;
		const int pCount = fragMain->getParameterCount();

		if (comments)
			preamble << "//\n// Translator's entry point\n//\n";
		preamble << "void main() {\n";
		const EGlslSymbolType retType = fragMain->getReturnType();
		GlslStruct *retStruct = fragMain->getStruct();
		if (  retType == EgstStruct)
		{
			assert(retStruct);
			preamble << "    " << retStruct->getName() << " xlat_retVal;\n";
		}
		else
		{
			if ( retType != EgstVoid)
			{
				preamble << "    " << getTypeString(retType) << " xlat_retVal;\n";
			}
		}

		// Write all mutable initializations
		if ( calledFunctions[1].size() > 0 )
		{
			for (FunctionSet::iterator fit = calledFunctions[1].begin(); fit != calledFunctions[1].end(); fit++)
			{
				std::string mutableDecls = (*fit)->getMutableDecls(1, calledFunctions[1].begin(), fit);

				if ( mutableDecls.size() > 0 )
				{
					preamble << mutableDecls;
				}
			}
		}

		call << "    ";
		if (retType != EgstVoid)
			call << "xlat_retVal = " << fragMain->getName() << "( ";
		else
			call << fragMain->getName() << "( ";

		for (int ii=0; ii<pCount; ii++)
		{
			GlslSymbol *sym = fragMain->getParameter(ii);
			EAttribSemantic attrSem = parseAttributeSemantic( sym->getSemantic());
			switch (sym->getQualifier())
			{
			case EqtIn:
			case EqtInOut:
				if ( sym->getType() != EgstStruct)
				{
					std::string name, ctor;
					int pad;

					if ( getArgumentData( sym, EClassVarIn, name, ctor, pad) )
					{
						// For "in" parameters, just call directly to the main                  
						if ( sym->getQualifier() != EqtInOut )
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
							preamble << getTypeString(sym->getType()) << " xlat_temp_" << sym->getName() << " = ";
							preamble << ctor << "(" << name;
							for (int ii = 0; ii<pad; ii++)
								preamble << ", 0.0";
							preamble << ");\n";
						}

						// If using user varying, add it to the varying list
						if ( strstr ( name.c_str(), "xlat_varying" ) )
						{
							varying << "varying vec4 " << name <<";\n" ;
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
					std::string tempVar = "xlat_temp_" + sym->getName();
					preamble << "    " << Struct->getName() << " ";
					preamble << tempVar <<";\n";
					call << tempVar;

					const int elem = Struct->memberCount();
					for (int jj=0; jj<elem; jj++)
					{
						const GlslStruct::member &current = Struct->getMember(jj);
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

						for ( int arrayIndex = 0; arrayIndex < numArrayElements; arrayIndex++ )
						{

							if ( getArgumentData( current.name, current.semantic, current.type,
								EClassVarIn, name, ctor, pad, arrayIndex ) )
							{

								preamble << "    ";
								preamble << tempVar << "." << current.name;

								if ( bIsArray )
									preamble << "[" << arrayIndex << "]";

								preamble << " = " << ctor << "( " << name;

								for (int ii = 0; ii<pad; ii++)
									preamble << ", 0.0";

								preamble << ");\n";

								// If using user varying, add it to the varying list
								if ( strstr ( name.c_str(), "xlat_varying" ) )
								{
									varying << "varying vec4 " << name <<";\n" ;
								}
							}
							else
							{
								//should deal with fall through cases here
								assert(0);
								infoSink.info << "Unsupported struct element type for shader entry parameter (";
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

				// Also a fallthrough for "inout" (see if check above)
			case EqtOut:
				if (sym->getType() != EgstStruct)
				{
					std::string name, ctor;
					int pad;

					if ( getArgumentData( sym, EClassRes, name, ctor, pad) )
					{
						if ( sym->getQualifier() != EqtInOut )
						{
							preamble << "    ";
							preamble << getTypeString(sym->getType()) << " xlat_temp_" << sym->getName() << ";\n";
						}

						call << "xlat_temp_" << sym->getName();

						postamble << "    ";
						postamble << name << " = " << ctor << "( xlat_temp_" <<sym->getName();
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
					std::string tempVar = "xlat_temp_" + sym->getName();

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

						if ( getArgumentData( current.name, current.semantic, current.type, EClassRes, name, ctor, pad) )
						{
							postamble << "    ";
							postamble << name << " = " << ctor;
							postamble << "( " << tempVar << "." << current.name;
							for (int ii = 0; ii<pad; ii++)
								postamble << ", 0.0";

							postamble << ");\n";
						}
						else
						{
							//should deal with fall through cases here
							assert(0);
							infoSink.info << "Unsupported struct element type for shader entry parameter (";
							infoSink.info << getTypeString(current.type) << ")\n";
						}
					}
				}
				break;
			case EqtUniform:
				uniform << "uniform " << getTypeString(sym->getType()) << " ";
				uniform << "xlat_uniform_" << sym->getName() << ";\n";
				call << "xlat_uniform_" << sym->getName();
				break;
			default:
				assert(0);
			};
			if (ii != pCount -1)
				call << ", ";
		}

		call << ");\n";

		if (retType != EgstVoid)
		{

			if (retType != EgstStruct)
			{
				std::string name, ctor;
				int pad;

				if ( getArgumentData( "", fragMain->getSemantic(), retType, EClassRes,
					name, ctor, pad) )
				{

					postamble << "    ";
					postamble << name << " = " << ctor << "( xlat_retVal";
					for (int ii = 0; ii<pad; ii++)
						postamble << ", 0.0";

					postamble << ");\n";
				}
				else
				{
					//should deal with fall through cases here
					assert(0);
					infoSink.info << "Unsupported return type for shader entry function (";
					infoSink.info << getTypeString(retType) << ")\n";
				}
			}
			else
			{
				//structs must pass the struct, then process per element
				GlslStruct *Struct = retStruct;
				assert(Struct);

				const int elem = Struct->memberCount();
				for (int ii=0; ii<elem; ii++)
				{
					const GlslStruct::member &current = Struct->getMember(ii);
					std::string name, ctor;
					int pad;

					if ( getArgumentData( current.name, current.semantic, current.type, EClassRes, name, ctor, pad) )
					{
						postamble << "    ";
						postamble << name << " = " << ctor;
						postamble << "( xlat_retVal."  << current.name;
						for (int ii = 0; ii<pad; ii++)
							postamble << ", 0.0";

						postamble << ");\n";
					}
					else
					{
						//should deal with fall through cases here
						assert(0);
						infoSink.info << "Unsupported struct element type in return type for shader entry function (";
						infoSink.info << getTypeString(current.type) << ")\n";
					}
				}
			}
		}
		else
		{
			// If no return type, close off the output
			postamble << ";\n";
		}
		postamble << "}\n\n";

		if (uniform.str().size())
		{
			if (comments)
				fragShader << "//\n// Uniform Arguments\n//\n";
			fragShader << uniform.str() << "\n";
		}

		if (varying.str().size())
		{
			if (comments)
				fragShader << "//\n// User varying\n//\n";
			fragShader << varying.str() << "\n";
		}
		fragShader << preamble.str() << "\n";
		fragShader << call.str() << "\n";
		fragShader << postamble.str() << "\n";
	}

	return true;
}


//=========================================================================================================
/// Interface to retreive the output GLSL shader text
/// \param lan
///   The language to get the shader text for
/// \return As a C string, the shader text for the language specified
//=========================================================================================================   
const char* HlslLinker::getShaderText( EShLanguage lang ) const 
{
	if ( lang == EShLangVertex) 
	{
		bs = vertShader.str();
		return bs.c_str();
	}
	else if ( lang == EShLangFragment) 
	{
		bs = fragShader.str();
		return bs.c_str();
	}
	else
		return 0;     
}