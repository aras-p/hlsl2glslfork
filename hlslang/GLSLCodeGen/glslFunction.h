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


#ifndef GLSL_FUNCTION_H
#define GLSL_FUNCTION_H

#include <set>

#include "glslCommon.h"
#include "glslStruct.h"
#include "glslSymbol.h"

/// Represents all the data necessary to represent a
/// function for the linker to create a complete output program.
class GlslFunction 
{
public:
   GlslFunction( const std::string &n, const std::string &m, EGlslSymbolType type, const std::string &s );
   virtual ~GlslFunction();
   
   void addSymbol( GlslSymbol *sym );   
   void addParameter( GlslSymbol *sym );
   
   bool hasSymbol( int id );
   GlslSymbol& getSymbol( int id );
   
   std::string getPrototype();
   std::string getLocalDecls( int indentLevel );

   /// Returns, as a string, the mutable declarations in the function.  Takes a set of other functions to
   /// check whether the mutable has already been declared.
   std::string getMutableDecls( int indentLevel, std::vector<GlslFunction*>::iterator funcBegin, 
                                                 std::vector<GlslFunction*>::iterator funcEnd );

   /// Returns the active scope
   std::string getCode() { return active.str(); }

   int getParameterCount() { return (int)parameters.size();}   
   GlslSymbol* getParameter( int i ) { return parameters[i];}
   
   void addCalledFunction( const std::string& func ) { calledFunctions.insert(func); }
   const std::set<std::string>& getCalledFunctions() const  { return calledFunctions; }
   
   void addLibFunction( TOperator op ) { libFunctions.insert( op); }
   const std::set<TOperator>& getLibFunctions() const { return libFunctions; }
   
   const std::vector<GlslSymbol*>& getSymbols() { return symbols; }

   void increaseDepth() { depth++; }   
   void decreaseDepth() { depth = depth ? depth-1 : depth; }

   void indent( std::stringstream &s ) { for (int ii = 0; ii < depth; ii++) s << "    "; }
   void indent() { indent(active); }
   
   void beginBlock( bool brace = true) { if (brace) active << "{\n"; increaseDepth(); inStatement = false; }
   void endBlock() { endStatement(); decreaseDepth(); indent(); active << "}\n";  }
   
   void beginStatement() { if (!inStatement) { indent(); inStatement = true;}}
   void endStatement() { if (inStatement) { active << ";\n"; inStatement = false;}}

   const std::string &getName() { return name; }
   const std::string &getMangledName() { return mangledName; }

   EGlslSymbolType getReturnType() { return returnType; }   
   const std::string& getSemantic() const { return semantic; }    
   GlslStruct* getStruct() { return structPtr; }   
   void setStruct( GlslStruct *s ) { structPtr = s;}

   std::stringstream& getActiveOutput () { return active; }
   
protected:

   // Function info
   std::string name;
   std::string mangledName;
   EGlslSymbolType returnType;
   std::string semantic;

   // Structure return value
   GlslStruct *structPtr;  

   // Present indent depth
   int depth; 

   // These are the symbols referenced
   std::vector<GlslSymbol*> symbols;
   std::map<std::string,GlslSymbol*> symbolNameMap;
   std::map<int,GlslSymbol*> symbolIdMap;
   std::vector<GlslSymbol*> parameters;

   // Functions called by this function
   std::set<std::string> calledFunctions;

   // Built-in functions needing the support lib that were called
   std::set<TOperator> libFunctions;

   // Stores the active output of the function
   std::stringstream active;

   bool inStatement;    

};

#endif //GLSL_FUNCTION_H

