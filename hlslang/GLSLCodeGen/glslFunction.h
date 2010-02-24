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
// Definition of glslFunction
//=================================================================================================================================

#ifndef GLSL_FUNCTION_H
#define GLSL_FUNCTION_H

#include <set>

#include "glslCommon.h"
#include "glslStruct.h"
#include "glslSymbol.h"

//=================================================================================================================================
/// GlslFunction
/// 
/// \brief This class represents all the data necessary to represent a
///        function for the linker to create a complete output program.
//=================================================================================================================================
class GlslFunction 
{

public:

   //=========================================================================================================
   /// Constructor
   //=========================================================================================================
   GlslFunction( const std::string &n, const std::string &m, EGlslSymbolType type, const std::string &s );

   //=========================================================================================================
   /// Destructor
   //=========================================================================================================
   virtual ~GlslFunction();
   
   //=========================================================================================================
   /// Add a symbol to the function
   //=========================================================================================================   
   void addSymbol( GlslSymbol *sym );
   
   //=========================================================================================================
   /// Add a symbol to the function, mark it as a parameter
   //=========================================================================================================         
   void addParameter( GlslSymbol *sym );
   
   //=========================================================================================================
   /// Returns whether the symbol id is found in the function's symbol ID map
   //=========================================================================================================  
   bool hasSymbol( int id );

   //=========================================================================================================
   /// Returns a reference to the symbol referred to by id
   //=========================================================================================================
   GlslSymbol& getSymbol( int id );
   
   //=========================================================================================================
   /// Returns, as a string, the prototype for the function
   //=========================================================================================================
   std::string getPrototype();

   //=========================================================================================================
   /// Returns, as a string, the local declarations in the function
   //=========================================================================================================   
   std::string getLocalDecls( int indentLevel );

   //=========================================================================================================
   /// Returns, as a string, the mutable declarations in the function.  Takes a set of other functions to
   /// check whether the mutable has already been declared.
   //=========================================================================================================   
   std::string getMutableDecls( int indentLevel, std::set<GlslFunction*>::iterator funcBegin, 
                                                 std::set<GlslFunction*>::iterator funcEnd );

   //=========================================================================================================
   /// Returns the active scope
   //========================================================================================================= 
   std::string getCode() { return active.str(); }

   //=========================================================================================================
   /// Returns the number of parameters to the function
   //=========================================================================================================              
   int getParameterCount() { return (int)parameters.size();}
   
   //=========================================================================================================
   /// Returns the parameter i as a GLSL symbol
   //=========================================================================================================                 
   GlslSymbol* getParameter( int i ) { return parameters[i];}
   
   //=========================================================================================================
   /// Add a function called by this function
   //=========================================================================================================                    
   void addCalledFunction( const std::string& func ) { calledFunctions.insert(func); }
   
   //=========================================================================================================
   /// Get the set of functions called by this function
   //=========================================================================================================                       
   const std::set<std::string>& getCalledFunctions() const  { return calledFunctions; }
   
   //=========================================================================================================
   /// Add an HLSL support library function by opcode
   //=========================================================================================================                          
   void addLibFunction( TOperator op ) { libFunctions.insert( op); }

   //=========================================================================================================
   /// Get the HLSL support library functions used by this function
   //=========================================================================================================                          
   const std::set<TOperator>& getLibFunctions() const { return libFunctions; }
   
   //=========================================================================================================
   /// Get a vector of symbols used in this function
   //=========================================================================================================                          
   const std::vector<GlslSymbol*>& getSymbols() { return symbols; }

   //=========================================================================================================
   /// Increase function depth
   //=========================================================================================================                             
   void increaseDepth() { depth++; }
   
   //=========================================================================================================
   /// Decrease function depth
   //=========================================================================================================                                
   void decreaseDepth() { depth = depth ? depth-1 : depth; }

   //=========================================================================================================
   /// Indent the function based on the current depth
   //=========================================================================================================                                   
   void indent( std::stringstream &s ) { for (int ii = 0; ii < depth; ii++) s << "    "; }
   
   //=========================================================================================================
   /// Indent the active function stream
   //=========================================================================================================                                      
   void indent() { indent(active); }
   
   //=========================================================================================================
   /// Begin a new block on the active function stream
   //=========================================================================================================                                         
   void beginBlock( bool brace = true) { if (brace) active << "{\n"; increaseDepth(); inStatement = false; }
   
   //=========================================================================================================
   /// End the current block in the active function stream
   //=========================================================================================================                                            
   void endBlock() { endStatement(); decreaseDepth(); indent(); active << "}\n";  }
   
   //=========================================================================================================
   /// Begin a new statement in the active function stream
   //=========================================================================================================                                               
   void beginStatement() { if (!inStatement) { indent(); inStatement = true;}}

   //=========================================================================================================
   /// End the current statement in the active function stream
   //=========================================================================================================                                               
   void endStatement() { if (inStatement) { active << ";\n"; inStatement = false;}}

   //=========================================================================================================
   /// Get the name of the function
   //=========================================================================================================                                                  
   const std::string &getName() { return name; }
   
   //=========================================================================================================
   /// Get the mangled name of the function
   //=========================================================================================================                                                     
   const std::string &getMangledName() { return mangledName; }

   //=========================================================================================================
   /// Get the return type of the function
   //=========================================================================================================                                                     
   EGlslSymbolType getReturnType() { return returnType; }
   
   //=========================================================================================================
   /// Get the semantic of the function return
   //=========================================================================================================                                                        
   const std::string& getSemantic() const { return semantic; } 
   
   //=========================================================================================================
   /// Get the struct of the function return
   //=========================================================================================================                                                           
   GlslStruct* getStruct() { return structPtr; }
   
   //=========================================================================================================
   /// Set the the struct of the function return
   //=========================================================================================================                                                           
   void setStruct( GlslStruct *s ) { structPtr = s;}

   //=========================================================================================================
   /// Get the active output stream for the function
   //=========================================================================================================
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

