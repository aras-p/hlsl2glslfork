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
// Implementation of GlslFunction
//=================================================================================================================================

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================

#include "glslFunction.h"


//=================================================================================================================================
//
//          Constructor(s) / Destructor(s) Block 
//
//=================================================================================================================================

//=========================================================================================================
/// Constructor
//=========================================================================================================
GlslFunction::GlslFunction( const std::string &n, const std::string &m, EGlslSymbolType type, const std::string &s ) :
      name(n),
      mangledName(m),
      returnType(type),
      semantic(s),
      structPtr(0),
      depth(0),
      inStatement(false)
{ 
   active <<std::showpoint;
}

//=========================================================================================================
/// Destructor
//=========================================================================================================
GlslFunction::~GlslFunction()
{
   for (std::vector<GlslSymbol*>::iterator it = symbols.begin(); it < symbols.end(); it++)
   {
      (*it)->releaseRef ();
      if ( (*it)->getRef() == 0 )
      {
         delete *it;
      }
   }
}

//=================================================================================================================================
//
//          Public methods block
//
//=================================================================================================================================

//=========================================================================================================
/// Returns whether the symbol id is found in the function's symbol ID map
/// \param id
///     Id of symbol to search for in ID map
/// \return
///     True if symbol is found in the ID map, false otherwise
//=========================================================================================================
bool GlslFunction::hasSymbol( int id )
{

   if (symbolIdMap.find(id) != symbolIdMap.end())
   {
      return true;
   }

   return false;
}


//=========================================================================================================
/// Returns a reference to the symbol referred to by id
/// \param id
///     Id of symbol to search for in ID map
/// \return
///     Reference to the symbol in the ID map
//=========================================================================================================
GlslSymbol& GlslFunction::getSymbol( int id )
{
   return *symbolIdMap[id];
}

//=========================================================================================================
/// Add a symbol to the function
/// \param sym
///     Symbol to add to the function
//=========================================================================================================
void GlslFunction::addSymbol( GlslSymbol *sym )
{
   // Add a reference count to the symbol
   sym->addRef();

   // mangle the name to avoid conflicts
   while ( symbolNameMap.find(sym->getName()) != symbolNameMap.end())
   {
      sym->mangleName();
   }

   symbols.push_back( sym);
   symbolNameMap[sym->getName()] = sym;
   symbolIdMap[sym->getId()] = sym;
}

//=========================================================================================================
/// Add a symbol to the function, mark it as a parameter
/// \param sym
///     Symbol to add to the function, to be marked as a function parameter
//=========================================================================================================
void GlslFunction::addParameter( GlslSymbol *sym )
{
   // Add a reference count to the symbol
   sym->addRef();

   sym->setIsParameter(true);

   //mangle the name to avoid conflicts
   while ( symbolNameMap.size() > 0 && symbolNameMap.find(sym->getName()) != symbolNameMap.end() )
   {
      sym->mangleName();
   }

   symbols.push_back( sym);
   symbolNameMap[sym->getName()] = sym;
   symbolIdMap[sym->getId()] = sym;
   parameters.push_back( sym);
}

//=========================================================================================================
/// Returns, as a string, the prototype for the function
/// \return
///     As a string, the prototype of the function
//=========================================================================================================
std::string GlslFunction::getPrototype()
{
   std::stringstream out;

   writeType( out, returnType, structPtr);
   out << " " << name << "( ";

   std::vector<GlslSymbol*>::iterator it = parameters.begin();

   if (it != parameters.end())
   {
      (*it)->writeDecl(out);
      it++;
   }

   while ( it != parameters.end())
   {
      out << ", ";
      (*it)->writeDecl(out);
      it++;
   }

   out << " )";

   return out.str();
}

//=========================================================================================================
/// Returns, as a string, the local declarations in the function
/// \param indentLevel
///     How many levels to indent the output
/// \return
///     As a string, local declarations of the function
//=========================================================================================================
std::string GlslFunction::getLocalDecls( int indentLevel )
{
   std::stringstream out;
   int oldDepth;

   // Make sure that decimal points are printed to the output string.  Otherwise, the following:
   //  float fTemp = 1.0;
   // will come out:
   //  float fTemp = 1;
   // causing a GLSL parse error
   out.setf ( std::stringstream::showpoint );

   //save off the old depth, to restore later, this is probably unnecessary
   oldDepth = depth;
   depth = indentLevel;

   for (std::vector<GlslSymbol*>::iterator it = symbols.begin(); it < symbols.end(); it++)
   {
      if ( (!(*it)->getIsParameter() && !(*it)->getIsGlobal() && !(*it)->getIsMutable()) )
      {
         indent(out);
         if ((*it)->hasInitializer())
         {
            (*it)->writeDecl(out, true);         
            if ((*it)->isArray())
            {
               out <<";\n";
               for (int ii = 0; ii < (*it)->getArraySize(); ii++)
               {
                  indent(out);
                  out << (*it)->getName() << "[" << ii << "] = ";
                  (*it)->writeInitializer(out,ii);
                  if ( (ii+1) != (*it)->getArraySize())
                     out << ";\n";
               }
            }
            else
            {
               out << " = ";
               (*it)->writeInitializer(out);
            }
         }
         else
         {
            (*it)->writeDecl(out, true);         
         }
         out << ";\n";
      }
   }

   depth = oldDepth;

   return out.str();
}

//=========================================================================================================
/// Returns, as a string, the mutable declarations in the function.  Takes a set of other functions to
/// check whether the mutable has already been declared.
/// \param indentLevel
///     How many levels to indent the output
/// \param funcBegin
///     First function which for which mutables have already been processed
/// \param funcEnd
///     Current function being processed for mutables
/// \return
///     As a string, local declarations of the function
//=========================================================================================================
std::string GlslFunction::getMutableDecls( int indentLevel, std::vector<GlslFunction*>::iterator funcBegin, 
                                           std::vector<GlslFunction*>::iterator funcEnd  )
{
   std::stringstream out;
   int oldDepth;

   // Make sure that decimal points are printed to the output string.  Otherwise, the following:
   //  float fTemp = 1.0;
   // will come out:
   //  float fTemp = 1;
   // causing a GLSL parse error
   out.setf ( std::stringstream::showpoint );

   //save off the old depth, to restore later, this is probably unnecessary
   oldDepth = depth;
   depth = indentLevel;

   for (std::vector<GlslSymbol*>::iterator it = symbols.begin(); it < symbols.end(); it++)
   {
      if ( (*it)->getIsMutable() )
      {
         bool bMutableAlreadyProcessed = false;

         // Check the set of already processed functions to see if the mutable was already
         // written out
         std::vector<GlslFunction*>::iterator fit = funcBegin;
         while ( fit != funcEnd && bMutableAlreadyProcessed == false)
         {
            std::vector<GlslSymbol*> funcSymbols = (*fit)->getSymbols();

            for (size_t i = 0; i < funcSymbols.size(); i++ )
            {
               if ( (*it)->getName() == funcSymbols[i]->getName() )
               {
                  // Symbol found, this mutable has already been processed.  Don't do it again.
                  bMutableAlreadyProcessed = true;
                  break;                  
               }
            }
            fit++;
         }

         if ( bMutableAlreadyProcessed == false )
         {
            indent(out);
            if ((*it)->isArray())
            {
               for (int ii = 0; ii < (*it)->getArraySize(); ii++)
               {
                  out << (*it)->getName() << "[" << ii << "] = ";
                  out << (*it)->getName(false) << "[" << ii << "]";
                  if ( (ii+1) != (*it)->getArraySize())
                  {
                     out << ";\n";
                     indent(out);
                  }
               }
            }
            else
            {
               out << (*it)->getName() << " = " << (*it)->getName(false);
            }
         
            out << ";\n";
         }
      }
   }

   depth = oldDepth;

   return out.str();
}

