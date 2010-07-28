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


#ifndef _SHHANDLE_INCLUDED_
#define _SHHANDLE_INCLUDED_

#include "../../include/hlsl2glsl.h"

#include "InfoSink.h"

class TCompiler;
class TLinker;


class TShHandleBase
{
public:
   TShHandleBase() {}
   virtual ~TShHandleBase() {}
};

class TIntermNode;

class TCompiler : public TShHandleBase
{
public:
   TCompiler(EShLanguage l, TInfoSink& sink) : infoSink(sink) , language(l), haveValidObjectCode(false)
   { }
   virtual ~TCompiler() { }
   EShLanguage getLanguage() { return language; }
   virtual TInfoSink& getInfoSink() { return infoSink; }

   virtual bool compile(TIntermNode* root) = 0;

   virtual bool linkable() { return haveValidObjectCode; }

   TInfoSink& infoSink;
protected:
   EShLanguage language;
   bool haveValidObjectCode;
};


class TLinker : public TShHandleBase
{
public:
   TLinker(TInfoSink& iSink) : 
      infoSink(iSink),
      uniformBindings(0)
   {
   }
   virtual ~TLinker() { }
   virtual bool link(TCompiler*, const char*) { return false; }
   virtual bool setUserAttribName ( EAttribSemantic eSemantic, const char *pName ) = 0;
   virtual void setUseUserVaryings ( bool bUseUserVaryings ) = 0;
   virtual TInfoSink& getInfoSink()
   {
      return infoSink;
   }
   TInfoSink& infoSink;

   virtual const char* getShaderText() const = 0;
   virtual int getUniformCount() const = 0;
   virtual const ShUniformInfo* getUniformInfo() const = 0;

protected:
   ShBindingTable* uniformBindings;                // created by the linker
};



#endif // _SHHANDLE_INCLUDED_
