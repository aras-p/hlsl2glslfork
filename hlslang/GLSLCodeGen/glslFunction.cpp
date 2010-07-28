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

#include "glslFunction.h"


GlslFunction::GlslFunction( const std::string &n, const std::string &m, EGlslSymbolType type, const std::string &s ) :
      name(n),
      mangledName(m),
      returnType(type),
      semantic(s),
      structPtr(0),
      depth(0),
      inStatement(false)
{ 
	active.setf ( std::stringstream::showpoint );
	active.unsetf(std::ios::fixed);
	active.unsetf(std::ios::scientific);
	active.precision (6);
}


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


bool GlslFunction::hasSymbol( int id )
{

   if (symbolIdMap.find(id) != symbolIdMap.end())
   {
      return true;
   }

   return false;
}


GlslSymbol& GlslFunction::getSymbol( int id )
{
   return *symbolIdMap[id];
}

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
	out.unsetf(std::ios::fixed);
	out.unsetf(std::ios::scientific);
	out.precision (6);

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
	out.precision (6);

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

