// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "glslFunction.h"


GlslFunction::GlslFunction( const std::string &n, const std::string &m, EGlslSymbolType type, TPrecision prec, const std::string &s, const TSourceLoc& l) :
      name(n),
      mangledName(m),
      returnType(type),
	  precision(prec)
, semantic(s)
, line(l)
, structPtr(0),
      depth(0),
      inStatement(false)
{ 
	active = new std::stringstream();
	active->setf ( std::stringstream::showpoint );
	active->unsetf(std::ios::fixed);
	active->unsetf(std::ios::scientific);
	active->precision (6);
	pushDepth(0);
}


GlslFunction::~GlslFunction()
{
   popDepth();
   delete active;
   for (std::vector<GlslSymbol*>::iterator it = symbols.begin(); it < symbols.end(); it++)
   {
      (*it)->releaseRef ();
      if ( (*it)->getRef() == 0 )
      {
         delete *it;
      }
   }
}

void GlslFunction::pushDepth(int depth) { this->depth.push_back(depth); }
void GlslFunction::popDepth() { depth.pop_back(); }

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

   writeType (out, returnType, structPtr, precision);
   out << " " << name << "( ";

   std::vector<GlslSymbol*>::iterator it = parameters.begin();

   if (it != parameters.end())
   {
      (*it)->writeDecl(out,0);
      it++;
   }

   while ( it != parameters.end())
   {
      out << ", ";
      (*it)->writeDecl(out,0);
      it++;
   }

   out << " )";

   return out.str();
}

