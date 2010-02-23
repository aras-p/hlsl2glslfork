//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
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
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
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

//
// Symbol table for parsing.  Most functionaliy and main ideas 
// are documented in the header file.
//

#include "SymbolTable.h"

TString* TParameter::NullSemantic = 0;
//
// TType helper function needs a place to live.
//

//
// Recursively generate mangled names.
//
void TType::buildMangledName(TString& mangledName)
{
   if (isMatrix())
      mangledName += 'm';
   else if (isVector())
      mangledName += 'v';

   switch (type)
   {
   case EbtFloat:              mangledName += 'f';      break;
   case EbtInt:                mangledName += 'i';      break;
   case EbtBool:               mangledName += 'b';      break;
   case EbtSamplerGeneric:     mangledName += "sg";     break;
   case EbtSampler1D:          mangledName += "s1";     break;
   case EbtSampler2D:          mangledName += "s2";     break;
   case EbtSampler3D:          mangledName += "s3";     break;
   case EbtSamplerCube:        mangledName += "sC";     break;
   case EbtSampler1DShadow:    mangledName += "sS1";    break;
   case EbtSampler2DShadow:    mangledName += "sS2";    break;
   case EbtSamplerRect:        mangledName += "sR2";    break;  // ARB_texture_rectangle
   case EbtSamplerRectShadow:  mangledName += "sSR2";   break;  // ARB_texture_rectangle
   case EbtStruct:
      mangledName += "struct-";
      if (typeName)
         mangledName += *typeName;
      {// support MSVC++6.0
         for (unsigned int i = 0; i < structure->size(); ++i)
         {
            mangledName += '-';
            (*structure)[i].type->buildMangledName(mangledName);
         }
      }
   default: 
      break;
   }

   mangledName += static_cast<char>('0' + getNominalSize());
   if (isArray())
   {
      char buf[10];
      sprintf(buf, "%d", arraySize);
      mangledName += '[';
      mangledName += buf;
      mangledName += ']';
   }
}

int TType::getStructSize() const
{
   if (!getStruct())
   {
      assert(false && "Not a struct");
      return 0;
   }

   if (structureSize == 0)
      for (TTypeList::iterator tl = getStruct()->begin(); tl != getStruct()->end(); tl++)
         structureSize += ((*tl).type)->getObjectSize();

   return structureSize;
}

//======================================================================================================
// Determine the parameter compatibility between this type and the parameter type
//======================================================================================================
TType::ECompatibility TType::determineCompatibility ( const TType *pType ) const
{
   // make sure the array info matches
   if ( isArray() && pType->isArray() &&
        getArraySize() != pType->getArraySize())
   {
      return NOT_COMPATIBLE;
   }

   if ( isArray() != pType->isArray())
   {
      return NOT_COMPATIBLE;
   }

   if ( IsNumeric( getBasicType() ) &&
        IsNumeric( pType->getBasicType() ) && !isArray() )
   {
      // both parameters are numeric, so we can possibly convert

      // only allow matrices to convert to matrices
      if ( isMatrix() != pType->isMatrix())
         return NOT_COMPATIBLE;

      // Check if this is a promotion from a smaller vector to
      // larger vector.  HLSL allows this on function calls and
      // pads the result to 0.0.  The work to make these promotions
      // happen is in TParseContext::promoteFunctionArguments
      if ( getNominalSize() < pType->getNominalSize() &&
           pType->getNominalSize() > 1 )
      {
         if ( isVector() && pType->isVector() )
            return UPWARD_VECTOR_PROMOTION_EXISTS;
         else
            return NOT_COMPATIBLE;
      }

      // changing the type also counts as a promotion
      if ( getBasicType() != pType->getBasicType() )
      {
         // If the sizes don't match, then this is an implicit cast
         // with a promotion
         if ( getNominalSize() != pType->getNominalSize() )
            return IMPLICIT_CAST_WITH_PROMOTION_EXISTS;
         // Otherwise this is just a change of type
         else
            return IMPLICIT_CAST_EXISTS;
      }

      // changing the size is a promotion
      if ( getNominalSize() != pType->getNominalSize() )
         return PROMOTION_EXISTS;
   }
   else if ( getBasicType() == pType->getBasicType() )
   {
      // base types match, make sure everything else is OK

      // structs are a special case, we need to look inside
      if ( getBasicType() == EbtStruct)
      {
         // make sure the struct is OK
         return NOT_COMPATIBLE;
      }
   }
   else
   {
      // This means that we have a conversion that crosses incompatible types
      // Presently, this means that we cannot handle sampler conversions
      return NOT_COMPATIBLE;
   }

   return MATCH_EXACTLY;
}

//
// Dump functions.
//

void TVariable::dump(TInfoSink& infoSink) const 
{
   infoSink.debug << getName().c_str() << ": " << type.getQualifierString() << " " << type.getBasicString();
   if (type.isArray())
   {
      infoSink.debug << "[0]";
   }
   infoSink.debug << "\n";
}

void TFunction::dump(TInfoSink &infoSink) const
{
   infoSink.debug << getName().c_str() << ": " <<  returnType.getBasicString() << " " << getMangledName().c_str() << "\n";
}

void TSymbolTableLevel::dump(TInfoSink &infoSink) const 
{
   tLevel::const_iterator it;
   for (it = level.begin(); it != level.end(); ++it)
      (*it).second->dump(infoSink);
}

void TSymbolTable::dump(TInfoSink &infoSink) const
{
   for (int level = currentLevel(); level >= 0; --level)
   {
      infoSink.debug << "LEVEL " << level << "\n";
      table[level]->dump(infoSink);
   }
}

//
// Functions have buried pointers to delete.
//
TFunction::~TFunction()
{
   for (TParamList::iterator i = parameters.begin(); i != parameters.end(); ++i)
      delete (*i).type;
}

//
// Symbol table levels are a map of pointers to symbols that have to be deleted.
//
TSymbolTableLevel::~TSymbolTableLevel()
{
   for (tLevel::iterator it = level.begin(); it != level.end(); ++it)
      delete (*it).second;
}

//
// Change all function entries in the table with the non-mangled name
// to be related to the provided built-in operation.  This is a low
// performance operation, and only intended for symbol tables that
// live across a large number of compiles.
//
void TSymbolTableLevel::relateToOperator(const char* name, TOperator op) 
{
   tLevel::iterator it;
   for (it = level.begin(); it != level.end(); ++it)
   {
      if ((*it).second->isFunction())
      {
         TFunction* function = static_cast<TFunction*>((*it).second);
         if (function->getName() == name)
            function->relateToOperator(op);
      }
   }
}    


TSymbol::TSymbol(const TSymbol& copyOf)
{
   name = NewPoolTString(copyOf.name->c_str());
   uniqueId = copyOf.uniqueId;
}

TVariable::TVariable(const TVariable& copyOf, TStructureMap& remapper) : TSymbol(copyOf)
{
   type.copyType(copyOf.type, remapper);
   userType = copyOf.userType;
   // for builtIn symbol table level, unionArray and arrayInformation pointers should be NULL
   assert(copyOf.arrayInformationType == 0); 
   arrayInformationType = 0;

   if (copyOf.unionArray)
   {
      assert(!copyOf.type.getStruct()); 
      assert(copyOf.type.getObjectSize() == 1);
      unionArray = new constUnion[1];
      unionArray[0] = copyOf.unionArray[0];
   }
   else
      unionArray = 0;
}

TVariable* TVariable::clone(TStructureMap& remapper) 
{
   TVariable *variable = new TVariable(*this, remapper);

   return variable;
}

TFunction::TFunction(const TFunction& copyOf, TStructureMap& remapper) : TSymbol(copyOf)
{
   for (unsigned int i = 0; i < copyOf.parameters.size(); ++i)
   {
      TParameter param;
      parameters.push_back(param);
      parameters.back().copyParam(copyOf.parameters[i], remapper);
   }

   returnType.copyType(copyOf.returnType, remapper);
   mangledName = copyOf.mangledName;
   op = copyOf.op;
   defined = copyOf.defined;
}

TFunction* TFunction::clone(TStructureMap& remapper) 
{
   TFunction *function = new TFunction(*this, remapper);

   return function;
}


TSymbolTableLevel* TSymbolTableLevel::clone(TStructureMap& remapper)
{
   TSymbolTableLevel *symTableLevel = new TSymbolTableLevel();
   tLevel::iterator iter;
   for (iter = level.begin(); iter != level.end(); ++iter)
   {
      symTableLevel->insert(*iter->second->clone(remapper));
   }

   return symTableLevel;
}
//======================================================================================================
// This function uses the matching rules as described in the Cg language doc (the closest
// thing we have to HLSL function matching description) to find a matching compatible function.  
//======================================================================================================
TSymbol* TSymbolTableLevel::findCompatible( const TFunction *call, bool &ambiguous) const
{
   ambiguous = false;

   const TString &name = call->getName();   
   std::list<TFunction*> funcList;

   // 1. Add all functions with matching names to the set to consider
   tLevel::const_iterator it = level.begin();
   while ( it != level.end() )
   {
      if ( it->second->getName() == name && it->second->isFunction() )
      {
         funcList.push_back ( (TFunction*) it->second );
      }
      it++;
   }

   // 2. Eliminate candidates with the wrong number of parameters
   std::list<TFunction*>::iterator funcIter = funcList.begin();
   while ( funcIter != funcList.end() )
   {
      const TFunction* curFunc = *(funcIter);

      if ( call->getParamCount() != curFunc->getParamCount() )
      {
         funcIter = funcList.erase ( funcIter );         
      }
      else
      {
         funcIter++;
      }
   }

   // For each actual parameter expression, in the sequence:   
   for ( int nParam = 0; nParam < call->getParamCount() ; nParam++ )
   {
      const TType* type0 = (*call)[nParam].type;      

      // From the Cg function matching rules, perform the following matching on each parameter
      //
      // 3. For each parameter in the expression, if the type matches exactly, remove
      // all functions which do not match exactly
      //
      // 4. If there is a defined promotion for the type of the actual parameter to the unqualified type
      // of the formal parameter of any function, remove all functions for which this is not true
      //
      // 5. If there is a implicit cast for the type of the actual parameter to the unqualified type
      // of the formal parameter of any function, remove all functions for which this is not true
      //
      // 6. *** Not part of Cg rules, but used to differentiate between an implicit cast and an
      //        implicit cast with promotion.  This additional rule favors implict casts over
      //        implicit casts that also require promotion ***
      //    If there is a implicit cast with a promotion for the type of the actual parameter to the unqualified type
      //    of the formal parameter of any function, remove all functions for which this is not true     
      //
      // 7. *** Not part of Cg rules, but used to allow HLSL upward promotion on function calls ***
      //    If there is an upward vector promotion for the type of the actual parameter to the unqualified type
      //    of the formal parameter of any function, remove all functions for which this is not true.
      const TType::ECompatibility eCompatType[] =
      {
         TType::MATCH_EXACTLY,
         TType::PROMOTION_EXISTS,
         TType::IMPLICIT_CAST_EXISTS,
         TType::IMPLICIT_CAST_WITH_PROMOTION_EXISTS,
         TType::UPWARD_VECTOR_PROMOTION_EXISTS
      };

      // Iterate over each matching type (declared above)
      for ( int nIter = 0; nIter < sizeof(eCompatType) / sizeof (TType::ECompatibility); nIter++ )
      {
         bool bMatchesCompatibility = false;

         // Grab the compatibility type for the test
         TType::ECompatibility eCompatibility = eCompatType[nIter];

         funcIter = funcList.begin();
         while ( funcIter != funcList.end() )
         {
            const TFunction* curFunc = *(funcIter);
            const TType* type1 = (*curFunc)[nParam].type;

            // Check to see if the compatibility matches the compatibility type for this test
            if ( type0->determineCompatibility ( type1 ) == eCompatibility )
            {
               bMatchesCompatibility = true;
               break;               
            }

            funcIter++;
         }

         if ( bMatchesCompatibility )
         {
            // Remove all that don't match this compatibility test
            funcIter = funcList.begin();
            while ( funcIter != funcList.end() )
            {
               TFunction* curFunc = *(funcIter);
               const TType* type1 = (*curFunc)[nParam].type;

               if ( type0->determineCompatibility ( type1 ) != eCompatibility )
               {
                  funcIter = funcList.erase ( funcIter );               
               }
               else
                  funcIter++;
            }
         }
      }
   }


   // If the function list has 1 element, then we were successful
   if ( funcList.size() == 1 )
      return funcList.front();
   // If there is more than one element, it is ambiguous
   else if ( funcList.size () > 1 )
   {
      ambiguous = true;
      return NULL;
   }

   // No function found
   return NULL;
}

void TSymbolTable::copyTable(const TSymbolTable& copyOf)
{
   TStructureMap remapper;
   uniqueId = copyOf.uniqueId;
   for (unsigned int i = 0; i < copyOf.table.size(); ++i)
   {
      table.push_back(copyOf.table[i]->clone(remapper));
   }
}


